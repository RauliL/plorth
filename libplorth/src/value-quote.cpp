/*
 * Copyright (c) 2017-2018, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <plorth/context.hpp>

#include "./utils.hpp"

namespace plorth
{
  namespace
  {
    /**
     * Compiled quote consists from sequence of words parsed from source code.
     * When called, values are iterated and each value is being executed as part
     * of a script.
     */
    class compiled_quote : public quote
    {
    public:
      explicit compiled_quote(const std::vector<std::shared_ptr<value>>& values)
        : m_values(values) {}

      inline enum quote_type quote_type() const
      {
        return quote_type::compiled;
      }

      bool call(const std::shared_ptr<context>& ctx) const
      {
        for (const auto& value : m_values)
        {
          if (!value::exec(ctx, value))
          {
            return false;
          }
        }

        return true;
      }

      std::u32string to_string() const
      {
        std::u32string result;
        bool first = true;

        for (const auto& value : m_values)
        {
          if (first)
          {
            first = false;
          } else {
            result += ' ';
          }
          if (value)
          {
            result += value->to_source();
          } else {
            result += U"null";
          }
        }

        return result;
      }

      bool equals(const std::shared_ptr<value>& that) const
      {
        std::shared_ptr<compiled_quote> q;

        if (!value::is(that, type::quote) ||
            !std::static_pointer_cast<quote>(that)->is(quote_type::compiled))
        {
          return false;
        }
        q = std::static_pointer_cast<compiled_quote>(that);
        if (m_values.size() != q->m_values.size())
        {
          return false;
        }
        for (std::size_t i = 0; i < m_values.size(); ++i)
        {
          if (m_values[i] != q->m_values[i])
          {
            return false;
          }
        }

        return true;
      }

    private:
      const std::vector<std::shared_ptr<value>> m_values;
    };

    /**
     * Native quotes are wrappers around native C++ functions, allowing the
     * interpreter binary to interoperate with Plorth source code.
     */
    class native_quote : public quote
    {
    public:
      explicit native_quote(callback cb)
        : m_callback(cb) {}

      inline enum quote_type quote_type() const
      {
        return quote_type::native;
      }

      bool call(const std::shared_ptr<context>& ctx) const
      {
        m_callback(ctx);

        return !ctx->error();
      }

      std::u32string to_string() const
      {
        return U"\"native quote\"";
      }

      bool equals(const std::shared_ptr<value>& that) const
      {
        // Currently there is no way to compare two std::function instances
        // against each other, even when they are the same type.
        return this == that.get();
      }

    private:
      const callback m_callback;
    };
  }

  std::shared_ptr<quote> runtime::compiled_quote(const std::vector<std::shared_ptr<class value>>& values)
  {
    return std::shared_ptr<quote>(
      new (*m_memory_manager) class compiled_quote(values)
    );
  }

  std::shared_ptr<quote> runtime::native_quote(quote::callback callback)
  {
    return std::shared_ptr<quote>(
      new (*m_memory_manager) class native_quote(callback)
    );
  }

  std::u32string quote::to_source() const
  {
    return U"(" + to_string() + U")";
  }

  /**
   * Word: call
   * Prototype: quote
   *
   * Takes:
   * - quote
   *
   * Executes the quote taken from the top of the stack.
   */
  static void w_call(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<quote> q;

    if (ctx->pop_quote(q))
    {
      q->call(ctx);
    }
  }

  /**
   * Word: compose
   * Prototype: quote
   *
   * Takes:
   * - quote
   * - quote
   *
   * Gives:
   * - quote
   *
   * Constructs a new quote which will call the two given quotes in sequence.
   */
  static void w_compose(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<quote> left;
    std::shared_ptr<quote> right;

    if (ctx->pop_quote(right) && ctx->pop_quote(left))
    {
      ctx->push_quote({
        left,
        runtime->symbol(U"call"),
        right,
        runtime->symbol(U"call")
      });
    }
  }

  /**
   * Word: curry
   * Prototype: quote
   *
   * Takes:
   * - any
   * - quote
   *
   * Gives:
   * - quote
   *
   * Constructs a curried quote where given value will be pushed onto the stack
   * before calling the original quote.
   */
  static void w_curry(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<value> argument;
    std::shared_ptr<quote> quo;

    if (ctx->pop_quote(quo) && ctx->pop(argument))
    {
      ctx->push_quote({ argument, quo, runtime->symbol(U"call") });
    }
  }

  /**
   * Word: negate
   * Prototype: quote
   *
   * Takes:
   * - quote
   *
   * Gives:
   * - quote
   *
   * Constructs a negated version of given quote which negates the boolean
   * result returned by the original quote.
   */
  static void w_negate(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<quote> quo;

    if (ctx->pop_quote(quo))
    {
      ctx->push_quote({
        quo,
        runtime->symbol(U"call"),
        runtime->symbol(U"not")
      });
    }
  }

  /**
   * Word: dip
   * Prototype: quote
   *
   * Takes:
   * - any
   * - quote
   *
   * Gives:
   * - any
   *
   * Temporarily hides given value from the stack and calls given quote. Once
   * the quote has returned from it's execution, hidden value will be placed
   * back on the stack.
   */
  static void w_dip(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val;
    std::shared_ptr<quote> quo;

    if (!ctx->pop_quote(quo) || !ctx->pop(val))
    {
      return;
    }

    quo->call(ctx);
    ctx->push(val);
  }

  /**
   * Word: 2dip
   * Prototype: quote
   *
   * Takes:
   * - any
   * - any
   * - quote
   *
   * Gives:
   * - any
   * - any
   *
   *
   * Temporarily hides two given values from the stack and calls given quote.
   * Once the quote has returned from it's execution, hidden values will be
   * placed back on the stack.
   */
  static void w_2dip(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val1;
    std::shared_ptr<value> val2;
    std::shared_ptr<quote> quo;

    if (!ctx->pop_quote(quo) || !ctx->pop(val2) || !ctx->pop(val1))
    {
      return;
    }

    quo->call(ctx);
    ctx->push(val1);
    ctx->push(val2);
  }

  /**
   * Word: >word
   * Prototype: quote
   *
   * Takes:
   * - symbol
   * - quote
   *
   * Gives:
   * - word
   *
   * Constructs word from given pair of symbol and quote.
   */
  static void w_to_word(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<symbol> sym;
    std::shared_ptr<quote> quo;

    if (ctx->pop_quote(quo) && ctx->pop_symbol(sym))
    {
      ctx->push_word(sym, quo);
    }
  }

  namespace api
  {
    runtime::prototype_definition quote_prototype()
    {
      return
      {
        { U"call", w_call },
        { U"compose", w_compose },
        { U"curry", w_curry },
        { U"negate", w_negate },
        { U"dip", w_dip },
        { U"2dip", w_2dip },

        // Type conversions.
        { U">word", w_to_word }
      };
    }
  }
}

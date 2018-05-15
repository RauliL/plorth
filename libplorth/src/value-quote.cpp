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
      using size_type = std::size_t;

      explicit compiled_quote(const std::vector<ref<value>>& values)
        : m_size(values.size())
        , m_values(m_size > 0 ? new value*[m_size] : nullptr)
      {
        for (size_type i = 0; i < m_size; ++i)
        {
          m_values[i] = values[i].get();
        }
      }

      ~compiled_quote()
      {
        if (m_size > 0)
        {
          delete[] m_values;
        }
      }

      inline enum quote_type quote_type() const
      {
        return quote_type_compiled;
      }

      bool call(const ref<context>& ctx) const
      {
        for (size_type i = 0; i < m_size; ++i)
        {
          const auto& value = m_values[i];

          if (!value::exec(ctx, ref<class value>(value)))
          {
            return false;
          }
        }

        return true;
      }

      unistring to_string() const
      {
        unistring result;

        for (size_type i = 0; i < m_size; ++i)
        {
          const auto& value = m_values[i];

          if (i > 0)
          {
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

      bool equals(const ref<value>& that) const
      {
        ref<compiled_quote> q;

        if (!that->is(type_quote)
            || !that.cast<quote>()->is(quote_type_compiled))
        {
          return false;
        }
        q = that.cast<compiled_quote>();
        if (m_size != q->m_size)
        {
          return false;
        }
        for (size_type i = 0; i < m_size; ++i)
        {
          const auto ref1 = ref<value>(m_values[i]);
          const auto ref2 = ref<value>(q->m_values[i]);

          if (ref1 != ref2)
          {
            return false;
          }
        }

        return true;
      }

      void mark()
      {
        quote::mark();
        for (size_type i = 0; i < m_size; ++i)
        {
          auto& value = m_values[i];

          if (value && !value->marked())
          {
            value->mark();
          }
        }
      }

    private:
      const size_type m_size;
      value** m_values;
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
        return quote_type_native;
      }

      bool call(const ref<context>& ctx) const
      {
        m_callback(ctx);

        return !ctx->error();
      }

      unistring to_string() const
      {
        return U"\"native quote\"";
      }

      bool equals(const ref<value>& that) const
      {
        // Currently there is no way to compare two std::function instances
        // against each other, even when they are the same type.
        return this == that.get();
      }

    private:
      const callback m_callback;
    };
  }

  ref<quote> runtime::compiled_quote(const std::vector<ref<class value>>& values)
  {
    return ref<quote>(
      new (*m_memory_manager) class compiled_quote(values)
    );
  }

  ref<quote> runtime::native_quote(quote::callback callback)
  {
    return ref<quote>(
      new (*m_memory_manager) class native_quote(callback)
    );
  }

  unistring quote::to_source() const
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
  static void w_call(const ref<context>& ctx)
  {
    ref<quote> q;

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
  static void w_compose(const ref<context>& ctx)
  {
    const auto runtime = ctx->runtime();
    ref<quote> left;
    ref<quote> right;

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
  static void w_curry(const ref<context>& ctx)
  {
    const auto runtime = ctx->runtime();
    ref<value> argument;
    ref<quote> quo;

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
  static void w_negate(const ref<context>& ctx)
  {
    const auto runtime = ctx->runtime();
    ref<quote> quo;

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
  static void w_dip(const ref<context>& ctx)
  {
    ref<value> val;
    ref<quote> quo;

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
  static void w_2dip(const ref<context>& ctx)
  {
    ref<value> val1;
    ref<value> val2;
    ref<quote> quo;

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
  static void w_to_word(const ref<context>& ctx)
  {
    ref<symbol> sym;
    ref<quote> quo;

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

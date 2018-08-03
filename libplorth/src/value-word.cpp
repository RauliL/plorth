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
#include <plorth/value-word.hpp>

namespace plorth
{
  word::word(const std::shared_ptr<class symbol>& symbol,
             const std::shared_ptr<class quote>& quote)
    : m_symbol(symbol)
    , m_quote(quote) {}

  bool word::equals(const std::shared_ptr<value>& that) const
  {
    std::shared_ptr<word> w;

    if (!is(that, type::word))
    {
      return false;
    }
    w = std::static_pointer_cast<word>(that);

    return m_symbol->equals(w->m_symbol) && m_quote->equals(w->m_quote);
  }

  std::u32string word::to_string() const
  {
    return to_source();
  }

  std::u32string word::to_source() const
  {
    return U": " + m_symbol->id() + U" " + m_quote->to_string() + U" ;";
  }

  std::shared_ptr<word> runtime::word(
    const std::u32string& id,
    const std::shared_ptr<class quote>& quote
  )
  {
    return word(symbol(id), quote);
  }

  std::shared_ptr<word> runtime::word(
    const std::shared_ptr<class symbol>& symbol,
    const std::shared_ptr<class quote>& quote
  )
  {
    return value<class word>(symbol, quote);
  }

  /**
   * Word: symbol
   * Prototype: word
   *
   * Takes:
   * - word
   *
   * Gives:
   * - word
   * - symbol
   *
   * Extracts symbol from the word and places it onto top of the stack.
   */
  static void w_symbol(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<word> wrd;

    if (ctx->pop_word(wrd))
    {
      ctx->push(wrd);
      ctx->push(wrd->symbol());
    }
  }

  /**
   * Word: quote
   * Prototype: word
   *
   * Takes:
   * - word
   *
   * Gives:
   * - word
   * - quote
   *
   * Extracts quote which acts as the body of the word and places it onto top
   * of the stack.
   */
  static void w_quote(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<word> wrd;

    if (ctx->pop_word(wrd))
    {
      ctx->push(wrd);
      ctx->push(wrd->quote());
    }
  }

  /**
   * Word: call
   * Prototype: word
   *
   * Takes:
   * - word
   *
   * Executes body of the given word.
   */
  static void w_call(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<word> wrd;

    if (ctx->pop_word(wrd))
    {
      wrd->quote()->call(ctx);
    }
  }

  /**
   * Word: define
   * Prototype: word
   *
   * Takes:
   * - word
   *
   * Inserts given word into current local dictionary.
   */
  void w_define(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<word> wrd;

    if (ctx->pop_word(wrd))
    {
      value::exec(ctx, wrd);
    }
  }

  namespace api
  {
    runtime::prototype_definition word_prototype()
    {
      return
      {
        { U"symbol", w_symbol },
        { U"quote", w_quote },

        { U"call", w_call },
        { U"define", w_define }
      };
    }
  }
}

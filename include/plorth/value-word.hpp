/*
 * Copyright (c) 2017, Rauli Laine
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
#ifndef PLORTH_VALUE_WORD_HPP_GUARD
#define PLORTH_VALUE_WORD_HPP_GUARD

#include <plorth/value-quote.hpp>
#include <plorth/value-symbol.hpp>

namespace plorth
{
  /**
   * Word is a pair of symbol and quote, which can be placed into dictionary.
   */
  class word : public value
  {
  public:
    /**
     * Constructs new word.
     *
     * \param symbol Identifier of the word.
     * \param quote  Executable portion of the word.
     */
    explicit word(const ref<class symbol>& symbol,
                  const ref<class quote>& quote);

    /**
     * Returns identifier of the word.
     */
    inline const ref<class symbol>& symbol() const
    {
      return m_symbol;
    }

    /**
     * Returns executable portion of the word.
     */
    inline const ref<class quote>& quote() const
    {
      return m_quote;
    }

    inline enum type type() const
    {
      return type_word;
    }

    bool equals(const ref<value>& that) const;
    bool exec(const ref<context>& ctx);
    bool eval(const ref<context>& ctx, ref<value>& slot);
    unistring to_string() const;
    unistring to_source() const;

  private:
    /** Identifier of the word. */
    const ref<class symbol> m_symbol;
    /** Executable portion of the word. */
    const ref<class quote> m_quote;
  };
}

#endif /* !PLORTH_VALUE_WORD_HPP_GUARD */

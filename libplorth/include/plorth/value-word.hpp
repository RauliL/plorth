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
    explicit word(const std::shared_ptr<class symbol>& symbol,
                  const std::shared_ptr<class quote>& quote);

    /**
     * Returns identifier of the word.
     */
    inline const std::shared_ptr<class symbol>& symbol() const
    {
      return m_symbol;
    }

    /**
     * Returns executable portion of the word.
     */
    inline const std::shared_ptr<class quote>& quote() const
    {
      return m_quote;
    }

    inline enum type type() const
    {
      return type::word;
    }

    bool equals(const std::shared_ptr<value>& that) const;
    std::u32string to_string() const;
    std::u32string to_source() const;

  private:
    /** Identifier of the word. */
    const std::shared_ptr<class symbol> m_symbol;
    /** Executable portion of the word. */
    const std::shared_ptr<class quote> m_quote;
  };
}

#endif /* !PLORTH_VALUE_WORD_HPP_GUARD */

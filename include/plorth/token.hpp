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
#ifndef PLORTH_TOKEN_HPP_GUARD
#define PLORTH_TOKEN_HPP_GUARD

#include <plorth/unicode.hpp>

namespace plorth
{
  /**
   * Token is a portion of compiled Plorth source code, such as separator
   * operator, string literal or word.
   */
  class token
  {
  public:
    /**
     * Enumeration of diffent token types.
     */
    enum type
    {
      /** Left parenthesis operator. */
      type_lparen = '(',
      /** Right parenthesis operator. */
      type_rparen = ')',
      /** Left bracket operator. */
      type_lbrack = '[',
      /** Right bracket operator. */
      type_rbrack = ']',
      /** Left brace operator. */
      type_lbrace = '{',
      /** Right brace operator. */
      type_rbrace = '}',
      /** Colon operator. */
      type_colon = ':',
      /** Semicolon operator. */
      type_semicolon = ';',
      /** Comma operator. */
      type_comma = ',',
      /** Sequence of non-whitespace non-operator characters. */
      type_word = 'w',
      /** String literal. */
      type_string = '"'
    };

    /**
     * Constructs new token.
     *
     * \param type Type of the token.
     * \param text Text data associated with the token.
     */
    token(enum type type = type_word, const unistring& text = unistring());

    /**
     * Constructs copy of existing token.
     *
     * \param that Another token to construct copy of.
     */
    token(const token& that);

    /**
     * Moves contents of another token into new one.
     *
     * \param that Existing token to move contents from.
     */
    token(token&& that);

    /**
     * Copies contents of another token into this one.
     *
     * \param that Another token to copy contents from.
     */
    token& operator=(const token& that);

    /**
     * Moves contents of another token into this one.
     *
     * \param that Another token to move contents from.
     */
    token& operator=(token&& that);

    /**
     * Returns type of the token.
     */
    inline enum type type() const
    {
      return m_type;
    }

    /**
     * Tests whether token is of given type.
     */
    inline bool is(enum type t) const
    {
      return m_type == t;
    }

    /**
     * Returns text included within the token.
     */
    inline const unistring& text() const
    {
      return m_text;
    }

    /**
     * Tests whether this token is equal with another token.
     *
     * \param that Other token to test equality against.
     */
    bool equals(const token& that) const;

    /**
     * Equality testing operator.
     */
    inline bool operator==(const token& that) const
    {
      return equals(that);
    }

    /**
     * Non-equality testing operator.
     */
    inline bool operator!=(const token& that) const
    {
      return !equals(that);
    }

    /**
     * Constructs string which is source code representation of the token.
     */
    unistring to_source() const;

  private:
    /** Type of the token. */
    enum type m_type;
    /** Text data associated with the token. */
    unistring m_text;
  };

  std::ostream& operator<<(std::ostream&, enum token::type);
  std::ostream& operator<<(std::ostream&, const token&);
  uniostream& operator<<(uniostream&, enum token::type);
  uniostream& operator<<(uniostream&, const token&);
}

#endif /* !PLORTH_TOKEN_HPP_GUARD */

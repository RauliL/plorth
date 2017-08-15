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
#include <plorth/token.hpp>
#include "./utils.hpp"

namespace plorth
{
  token::token(enum type type, const unistring& data)
    : m_type(type)
    , m_text(data) {}

  token::token(const token& that)
    : m_type(that.m_type)
    , m_text(that.m_text) {}

  token::token(token&& that)
    : m_type(that.m_type)
    , m_text(that.m_text)
  {
    that.m_type = type_word;
    that.m_text.clear();
  }

  token& token::operator=(const token& that)
  {
    m_type = that.m_type;
    m_text = that.m_text;

    return *this;
  }

  token& token::operator=(token&& that)
  {
    if (this != &that)
    {
      m_type = that.m_type;
      m_text = that.m_text;
      that.m_type = type_word;
      that.m_text.clear();
    }

    return *this;
  }

  bool token::equals(const token& that) const
  {
    return m_type == that.m_type && !m_text.compare(that.m_text);
  }

  unistring token::to_source() const
  {
    const char* str;

    switch (m_type)
    {
      case type_lparen:
        str = "(";
        break;

      case type_rparen:
        str = ")";
        break;

      case type_lbrack:
        str = "[";
        break;

      case type_rbrack:
        str = "]";
        break;

      case type_lbrace:
        str = "{";
        break;

      case type_rbrace:
        str = "}";
        break;

      case type_colon:
        str = ":";
        break;

      case type_semicolon:
        str = ";";
        break;

      case type_comma:
        str = ",";
        break;

      case type_word:
        return m_text;

      case type_string:
        return json_stringify(m_text);
    }

    return utf8_decode(str);
  }

  std::ostream& operator<<(std::ostream& out, enum token::type type)
  {
    switch (type)
    {
      case token::type_lparen:
        out << "`('";
        break;

      case token::type_rparen:
        out << "`)'";
        break;

      case token::type_lbrack:
        out << "`['";
        break;

      case token::type_rbrack:
        out << "`]'";
        break;

      case token::type_lbrace:
        out << "`{'";
        break;

      case token::type_rbrace:
        out << "`}'";
        break;

      case token::type_colon:
        out << "`:'";
        break;

      case token::type_semicolon:
        out << "`;'";
        break;

      case token::type_comma:
        out << "`,'";
        break;

      case token::type_word:
        out << "word";
        break;

      case token::type_string:
        out << "string literal";
        break;
    }

    return out;
  }

  std::ostream& operator<<(std::ostream& out, const class token& token)
  {
    if (token.is(token::type_word))
    {
      out << "`" << token.text() << "'";
    }
    else if (token.is(token::type_string))
    {
      const unistring& text = token.text();

      if (text.length() > 15)
      {
        out << json_stringify(text.substr(0, 15) + "...");
      } else {
        out << json_stringify(text);
      }
    } else {
      out << token.type();
    }

    return out;
  }
}

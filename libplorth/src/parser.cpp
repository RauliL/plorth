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
#include <plorth/parser.hpp>

namespace plorth
{
  parser::parser(const std::u32string& source,
                 const std::u32string& filename,
                 int line,
                 int column)
    : m_pos(std::begin(source))
    , m_end(std::end(source))
  {
    m_position.filename = filename;
    m_position.line = line;
    m_position.column = column;
  }

  bool parser::parse(std::vector<std::shared_ptr<token>>& container)
  {
    while (!eof())
    {
      if (skip_whitespace())
      {
        break;
      }
      else if (const auto token = parse_value())
      {
        container.push_back(token);
      } else {
        return false;
      }
    }

    return true;
  }

  std::shared_ptr<token> parser::parse_value()
  {
    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing value.";

      return std::shared_ptr<token>();
    }
    switch (peek())
    {
      case '"':
      case '\'':
        return parse_string();

      case '(':
        return parse_quote();

      case '[':
        return parse_array();

      case '{':
        return parse_object();

      case ':':
        return parse_word();

      default:
        return parse_symbol();
    }
  }

  std::shared_ptr<token::array> parser::parse_array()
  {
    struct position position;
    token::array::container_type elements;

    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing array.";

      return std::shared_ptr<token::array>();
    }

    position = m_position;

    if (!peek_read('['))
    {
      m_error = U"Unexpected input; Missing array.";

      return std::shared_ptr<token::array>();
    }

    for (;;)
    {
      if (skip_whitespace())
      {
        m_error = U"Unterminated array; Missing `]'.";

        return std::shared_ptr<token::array>();
      }
      else if (peek_read(']'))
      {
        break;
      }
      else if (const auto value = parse_value())
      {
        elements.push_back(value);
        if (skip_whitespace() || (!peek(',') && !peek(']')))
        {
          m_error = U"Unterminated array; Missing `]'.";

          return std::shared_ptr<token::array>();
        }
        peek_read(',');
      } else {
        return std::shared_ptr<token::array>();
      }
    }

    return std::make_shared<token::array>(position, elements);
  }

  std::shared_ptr<token::object> parser::parse_object()
  {
    struct position position;
    token::object::container_type properties;

    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing object.";

      return std::shared_ptr<token::object>();
    }

    position = m_position;

    if (!peek_read('{'))
    {
      m_error = U"Unexpected input; Missing object.";

      return std::shared_ptr<token::object>();
    }

    for (;;)
    {
      if (skip_whitespace())
      {
        m_error = U"Unterminated object; Missing `}'.";

        return std::shared_ptr<token::object>();
      }
      else if (peek_read('}'))
      {
        break;
      }
      else if (const auto key = parse_string())
      {
        token::object::mapped_type value;

        if (skip_whitespace())
        {
          m_error = U"Unterminated object; Missing `}'.";

          return std::shared_ptr<token::object>();
        }

        if (!peek_read(':'))
        {
          m_error = U"Missing `:' after property key.";

          return std::shared_ptr<token::object>();
        }

        if (!(value = parse_value()))
        {
          return std::shared_ptr<token::object>();
        }

        properties.push_back({ key->value(), value });

        if (skip_whitespace() || (!peek(',') && !peek('}')))
        {
          m_error = U"Unterminated object; Missing `}'.";

          return std::shared_ptr<token::object>();
        }
        peek_read(',');
      } else {
        return std::shared_ptr<token::object>();
      }
    }

    return std::make_shared<token::object>(position, properties);
  }

  std::shared_ptr<token::quote> parser::parse_quote()
  {
    struct position position;
    token::quote::container_type children;

    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing quote.";

      return std::shared_ptr<token::quote>();
    }

    position = m_position;

    if (!peek_read('('))
    {
      m_error = U"Unexpected input; Missing quote.";

      return std::shared_ptr<token::quote>();
    }

    for (;;)
    {
      if (skip_whitespace())
      {
        m_error = U"Unterminated quote; Missing `)'.";

        return std::shared_ptr<token::quote>();
      }
      else if (peek_read(')'))
      {
        break;
      }
      else if (const auto child = parse_value())
      {
        children.push_back(child);
      } else {
        return std::shared_ptr<token::quote>();
      }
    }

    return std::make_shared<token::quote>(position, children);
  }

  std::shared_ptr<token::string> parser::parse_string()
  {
    struct position position;
    char32_t separator;
    std::u32string buffer;

    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing string.";

      return std::shared_ptr<token::string>();
    }

    position = m_position;

    if (peek_read('"'))
    {
      separator = '"';
    }
    else if (peek_read('\''))
    {
      separator = '\'';
    } else {
      m_error = U"Unexpected input; Missing string.";

      return std::shared_ptr<token::string>();
    }

    for (;;)
    {
      if (eof())
      {
        m_error = std::u32string(U"Unterminated string; Missing `")
          + separator
          + U"'.";

        return std::shared_ptr<token::string>();
      }
      else if (peek_read(separator))
      {
        break;
      }
      else if (peek_read('\\'))
      {
        if (!parse_escape_sequence(buffer))
        {
          return std::shared_ptr<token::string>();
        }
      } else {
        buffer.append(1, read());
      }
    }

    return std::make_shared<token::string>(position, buffer);
  }

  std::shared_ptr<token::symbol> parser::parse_symbol()
  {
    struct position position;
    std::u32string buffer;

    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing symbol.";

      return std::shared_ptr<token::symbol>();
    }

    position = m_position;

    if (!unicode_isword(peek()))
    {
      m_error = U"Unexpected input; Missing symbol.";

      return std::shared_ptr<token::symbol>();
    }

    do
    {
      buffer.append(1, read());
    }
    while (!eof() && unicode_isword(peek()));

    return std::make_shared<token::symbol>(position, buffer);
  }

  std::shared_ptr<token::word> parser::parse_word()
  {
    struct position position;
    std::shared_ptr<token::symbol> symbol;
    token::quote::container_type children;

    if (skip_whitespace())
    {
      m_error = U"Unexpected end of input; Missing word.";

      return std::shared_ptr<token::word>();
    }

    position = m_position;

    if (!peek_read(':'))
    {
      m_error = U"Unexpected input; Missing word.";

      return std::shared_ptr<token::word>();
    }

    if (!(symbol = parse_symbol()))
    {
      return std::shared_ptr<token::word>();
    }

    for (;;)
    {
      if (skip_whitespace())
      {
        m_error = U"Unterminated word; Missing `;'.";

        return std::shared_ptr<token::word>();
      }
      else if (peek_read(';'))
      {
        break;
      }
      else if (const auto child = parse_value())
      {
        children.push_back(child);
      } else {
        return std::shared_ptr<token::word>();
      }
    }

    return std::make_shared<token::word>(
      position,
      symbol,
      std::make_shared<token::quote>(position, children)
    );
  }

  char32_t parser::read()
  {
    const auto result = *m_pos++;

    if (result == '\n')
    {
      ++m_position.line;
      m_position.column = 1;
    } else {
      ++m_position.column;
    }

    return result;
  }

  bool parser::peek_read(char32_t expected)
  {
    if (peek(expected))
    {
      advance();

      return true;
    }

    return false;
  }

  bool parser::skip_whitespace()
  {
    while (!eof())
    {
      // Skip line comments.
      if (peek_read('#'))
      {
        while (!eof())
        {
          if (peek_read('\n') || peek_read('\r'))
          {
            break;
          } else {
            advance();
          }
        }
      }
      else if (!std::isspace(peek()))
      {
        return false;
      } else {
        advance();
      }
    }

    return true;
  }

  bool parser::parse_escape_sequence(std::u32string& buffer)
  {
    if (eof())
    {
      m_error = U"Unexpected end of input; Missing escape sequence.";

      return false;
    }
    switch (read())
    {
      case 'b':
        buffer.append(1, 010);
        break;

      case 't':
        buffer.append(1, 011);
        break;

      case 'n':
        buffer.append(1, 012);
        break;

      case 'f':
        buffer.append(1, 014);
        break;

      case 'r':
        buffer.append(1, 015);
        break;

      case '"':
      case '\'':
      case '\\':
      case '/':
        buffer.append(1, *(m_pos - 1));
        break;

      case 'u':
        {
          char32_t result = 0;

          for (int i = 0; i < 4; ++i)
          {
            if (eof())
            {
              m_error = U"Unterminated escape sequence.";

              return false;
            }
            else if (!std::isxdigit(peek()))
            {
              m_error = U"Illegal Unicode hex escape sequence.";

              return false;
            }

            if (peek() >= 'A' && peek() <= 'F')
            {
              result = result * 16 + (read() - 'A' + 10);
            }
            else if (peek() >= 'a' && peek() <= 'f')
            {
              result = result * 16 + (read() - 'a' + 10);
            } else {
              result = result * 16 + (read() - '0');
            }
          }

          if (!unicode_validate(result))
          {
            m_error = U"Illegal Unicode hex escape sequence.";

            return false;
          }

          buffer.append(1, result);
        }
        break;

      default:
        m_error = U"Illegal escape sequence in string literal.";

        return false;
    }

    return true;
  }
}

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
#include <plorth/context.hpp>
#include <plorth/token.hpp>

namespace plorth
{
  static bool compile_advance(context*,
                              std::string::const_iterator&,
                              const std::string::const_iterator&,
                              unichar&);
  static bool compile_string_literal(context*,
                                     const unichar,
                                     std::string::const_iterator&,
                                     const std::string::const_iterator&,
                                     std::vector<token>&,
                                     unistring&);

  ref<quote> context::compile(const std::string& source)
  {
    std::vector<token> tokens;
    auto it = std::begin(source);
    const auto end = std::end(source);
    unistring buffer;

    while (it != end)
    {
      unichar c;

      if (!compile_advance(this, it, end, c))
      {
        return ref<quote>();
      }

retry_switch:
      switch (c)
      {
        // Skip line comments.
        case '#':
          while (it != end)
          {
            if (!compile_advance(this, it, end, c))
            {
              return ref<quote>();
            }
            else if (c == '\n' || c == '\r')
            {
              break;
            }
          }
          continue;

        // Skip whitespace.
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          continue;

        // Separator characters.
        case '(': case ')':
        case '[': case ']':
        case '{': case '}':
        case ':': case ',':
        case ';':
          tokens.push_back(token(
            c == '(' ? token::type_lparen :
            c == ')' ? token::type_rparen :
            c == '[' ? token::type_lbrack :
            c == ']' ? token::type_rbrack :
            c == '{' ? token::type_lbrace :
            c == '}' ? token::type_rbrace :
            c == ':' ? token::type_colon :
            c == ';' ? token::type_semicolon :
            token::type_comma
          ));
          continue;

        case '\'':
        case '"':
          if (!compile_string_literal(this, c, it, end, tokens, buffer))
          {
            return ref<quote>();
          }
          continue;
      }

      if (!unichar_isword(c))
      {
        error(error::code_syntax, "Unexpected input.");

        return ref<quote>();
      }

      buffer.assign(1, c);

      while (it != end)
      {
        if (!compile_advance(this, it, end, c))
        {
          return ref<quote>();
        }
        else if (!unichar_isword(c))
        {
          tokens.push_back(token(token::type_word, buffer));
          goto retry_switch;
        }
        buffer.append(1, c);
      }

      tokens.push_back(token(token::type_word, buffer));
    }

    return m_runtime->compiled_quote(tokens);
  }

  static bool compile_advance(context* ctx,
                              std::string::const_iterator& it,
                              const std::string::const_iterator& end,
                              unichar& output)
  {
    if (utf8_advance(it, end, output))
    {
      return true;
    }
    ctx->error(error::code_syntax, "Unable to decode source code as UTF-8");

    return false;
  }

  static bool compile_string_literal(context* ctx,
                                     const unichar separator,
                                     std::string::const_iterator& it,
                                     const std::string::const_iterator& end,
                                     std::vector<token>& tokens,
                                     unistring& buffer)
  {
    buffer.clear();
    for (;;)
    {
      unichar c;

      if (it >= end)
      {
        ctx->error(error::code_syntax, "Unterminated string literal.");

        return false;
      }

      if (!compile_advance(ctx, it, end, c))
      {
        return false;
      }

      if (c == separator)
      {
        tokens.push_back(token(token::type_string, buffer));

        return true;
      }
      else if (c != '\\')
      {
        buffer.append(1, c);
        continue;
      }

      if (it >= end)
      {
        ctx->error(error::code_syntax, "Unterminated string literal.");

        return false;
      }
      else if (!compile_advance(ctx, it, end, c))
      {
        return false;
      }

      switch (c)
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
          buffer.append(1, c);
          break;

        case 'u':
          {
            unichar result = 0;

            for (int i = 0; i < 4; ++i)
            {
              if (it >= end)
              {
                ctx->error(error::code_syntax, "Unterminated string literal.");

                return false;
              }
              else if (!compile_advance(ctx, it, end, c))
              {
                return false;
              }
              else if (!std::isxdigit(c))
              {
                ctx->error(error::code_syntax, "Illegal Unicode hex escape sequence.");

                return false;
              }

              if (c >= 'A' && c <= 'F')
              {
                result = result * 16 + (c - 'A' + 10);
              }
              else if (c >= 'a' && c <= 'f')
              {
                result = result * 16 + (c - 'a' + 10);
              } else {
                result = result * 16 + (c - '0');
              }
            }

            if (!unichar_validate(result))
            {
              ctx->error(error::code_syntax, "Illegal Unicode hex escape sequence.");

              return false;
            }

            buffer.append(1, result);
          }
          break;

        default:
          ctx->error(error::code_syntax, "Illegal escape sequence in string literal.");

          return false;
      }
    }
  }
}

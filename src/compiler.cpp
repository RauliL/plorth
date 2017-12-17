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
#include <plorth/value-word.hpp>

namespace plorth
{
  namespace
  {
    class compiler
    {
    public:
      explicit compiler(const unistring& source)
        : m_pos(std::begin(source))
        , m_end(std::end(source)) {}

      /**
       * Returns true if there are no more characters to be read from the
       * source code.
       */
      inline bool eof() const
      {
        return m_pos >= m_end;
      }

      /**
       * Advances to the next character in source code, discarding the current
       * one.
       */
      inline void advance()
      {
        ++m_pos;
      }

      /**
       * Advances to the next character in source code and returns the current
       * one.
       */
      inline unichar read()
      {
        return *m_pos++;
      }

      /**
       * Returns next character to be read from the source code without
       * advancing any further.
       */
      inline unichar peek() const
      {
        return *m_pos;
      }

      /**
       * Returns true if next character to be read from the source code equals
       * with one given as argument.
       */
      inline bool peek(unichar expected) const
      {
        return !eof() && peek() == expected;
      }

      /**
       * Returns true if next character to be read from the source code matches
       * with given callback function.
       */
      inline bool peek(bool (*callback)(unichar)) const
      {
        return !eof() && callback(peek());
      }

      /**
       * Returns true and advances to next character if next character to be
       * read from the source code equals with one given as argument.
       */
      inline bool peek_read(unichar expected)
      {
        if (peek(expected))
        {
          advance();

          return true;
        }

        return false;
      }

      /**
       * Returns true and advances to next character if next character to be
       * read from the source code equals with one given as argument. Current
       * character will be stored into given slot.
       */
      inline bool peek_read(unichar expected, unichar& slot)
      {
        if (peek(expected))
        {
          slot = read();

          return true;
        }

        return false;
      }

      /**
       * Compiles top-level script into quote.
       */
      ref<quote> compile(context* ctx)
      {
        std::vector<ref<class value>> values;
        ref<class value> value;

        while (!eof())
        {
          if (skip_whitespace())
          {
            break;
          }
          else if (!(value = compile_value(ctx)))
          {
            return ref<quote>();
          }
          values.push_back(value);
        }

        return ctx->runtime()->compiled_quote(values);
      }

      ref<value> compile_value(context* ctx)
      {
        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing value."
          );

          return ref<value>();
        }

        switch (peek())
        {
        case '"':
        case '\'':
          return compile_string(ctx);

        case '(':
          return compile_quote(ctx);

        case '[':
          return compile_array(ctx);

        case '{':
          return compile_object(ctx);

        case ':':
          return compile_word(ctx);

        default:
          return compile_symbol(ctx);
        }
      }

      ref<symbol> compile_symbol(context* ctx)
      {
        unistring buffer;

        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing symbol."
          );

          return ref<symbol>();
        }

        if (!unichar_isword(peek()))
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected input; Missing symbol."
          );

          return ref<symbol>();
        }

        buffer.append(1, read());
        while (peek(unichar_isword))
        {
          buffer.append(1, read());
        }

        return ctx->runtime()->value<symbol>(buffer);
      }

      ref<word> compile_word(context* ctx)
      {
        const auto& runtime = ctx->runtime();
        ref<class symbol> symbol;
        std::vector<ref<value>> values;

        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing word."
            );

          return ref<word>();
        }

        if (!peek_read(':'))
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected input; Missing word."
            );

          return ref<word>();
        }

        if (!(symbol = compile_symbol(ctx)))
        {
          return ref<word>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(error::code_syntax, U"Unterminated word; Missing `;'.");

            return ref<word>();
          }
          else if (peek_read(';'))
          {
            break;
          } else {
            const auto value = compile_value(ctx);

            if (!value)
            {
              return ref<word>();
            }
            values.push_back(value);
          }
        }

        return runtime->value<word>(symbol, runtime->compiled_quote(values));
      }

      ref<quote> compile_quote(context* ctx)
      {
        std::vector<ref<value>> values;

        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing quote."
          );

          return ref<quote>();
        }

        if (!peek_read('('))
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected input; Missing quote."
          );

          return ref<quote>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code_syntax,
              U"Unterminated quote; Missing `)'."
            );

            return ref<quote>();
          }
          else if (peek_read(')'))
          {
            break;
          } else {
            const auto value = compile_value(ctx);

            if (!value)
            {
              return ref<quote>();
            }
            values.push_back(value);
          }
        }

        return ctx->runtime()->compiled_quote(values);
      }

      ref<string> compile_string(context* ctx)
      {
        unichar separator;
        unistring buffer;

        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing string."
          );

          return ref<string>();
        }

        if (!peek_read('"', separator) && !peek_read('\'', separator))
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected input; Missing string."
          );

          return ref<string>();
        }

        for (;;)
        {
          if (eof())
          {
            ctx->error(
              error::code_syntax,
              unistring(U"Unterminated string; Missing `") + separator + U"'"
            );

            return ref<string>();
          }
          else if (peek_read(separator))
          {
            break;
          }
          else if (peek_read('\\'))
          {
            if (!compile_escape_sequence(ctx, buffer))
            {
              return ref<string>();
            }
          } else {
            buffer.append(1, read());
          }
        }

        return ctx->runtime()->string(buffer);
      }

      ref<array> compile_array(context* ctx)
      {
        std::vector<ref<value>> elements;

        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing array."
          );

          return ref<array>();
        }

        if (!peek_read('['))
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected input; Missing array."
          );

          return ref<array>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code_syntax,
              U"Unterminated array; Missing `]'."
            );

            return ref<array>();
          }
          else if (peek_read(']'))
          {
            break;
          } else {
            const auto value = compile_value(ctx);

            if (!value)
            {
              return ref<array>();
            }
            elements.push_back(value);
            if (skip_whitespace() || (!peek(',') && !peek(']')))
            {
              ctx->error(
                error::code_syntax,
                U"Unterminated array; Missing `]'."
              );

              return ref<array>();
            } else {
              peek_read(',');
            }
          }
        }

        return ctx->runtime()->array(elements.data(), elements.size());
      }

      ref<object> compile_object(context* ctx)
      {
        object::container_type properties;

        if (skip_whitespace())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing object."
          );

          return ref<object>();
        }

        if (!peek_read('{'))
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected input; Missing object."
          );

          return ref<object>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code_syntax,
              U"Unterminated object; Missing `}'."
            );

            return ref<object>();
          }
          else if (peek_read('}'))
          {
            break;
          } else {
            ref<string> key;
            ref<class value> value;

            if (!(key = compile_string(ctx)))
            {
              return ref<object>();
            }

            if (skip_whitespace())
            {
              ctx->error(
                error::code_syntax,
                U"Unterminated object; Missing `}'."
              );

              return ref<object>();
            }

            if (!peek_read(':'))
            {
              ctx->error(
                error::code_syntax,
                U"Missing `:' after property key."
              );

              return ref<object>();
            }

            if (!(value = compile_value(ctx)))
            {
              return ref<object>();
            }

            properties[key->to_string()] = value;

            if (skip_whitespace() || (!peek(',') && !peek('}')))
            {
              ctx->error(
                error::code_syntax,
                U"Unterminated object; Missing `}'."
              );

              return ref<object>();
            } else {
              peek_read(',');
            }
          }
        }

        return ctx->runtime()->value<object>(properties);
      }

      bool compile_escape_sequence(context* ctx, unistring& buffer)
      {
        if (eof())
        {
          ctx->error(
            error::code_syntax,
            U"Unexpected end of input; Missing escape sequence."
          );

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
            unichar result = 0;

            for (int i = 0; i < 4; ++i)
            {
              if (eof())
              {
                ctx->error(
                  error::code_syntax,
                  U"Unterminated escape sequence."
                );

                return false;
              }
              else if (!std::isxdigit(peek()))
              {
                ctx->error(
                  error::code_syntax,
                  U"Illegal Unicode hex escape sequence."
                );

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

            if (!unichar_validate(result))
            {
              ctx->error(
                error::code_syntax,
                U"Illegal Unicode hex escape sequence."
              );

              return false;
            }

            buffer.append(1, result);
          }
          break;

        default:
          ctx->error(
            error::code_syntax,
            U"Illegal escape sequence in string literal."
          );

          return false;
        }

        return true;
      }

      /**
       * Skips whitespace and comments from source code.
       *
       * \return True if end of input has been reached, false otherwise.
       */
      bool skip_whitespace()
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

      compiler(const compiler&) = delete;
      void operator=(const compiler&) = delete;

    private:
      /** Current position in source code. */
      unistring::const_iterator m_pos;
      /** Iterator which marks end of the source code. */
      const unistring::const_iterator m_end;
    };
  }

  ref<quote> context::compile(const unistring& source)
  {
    return compiler(source).compile(this);
  }
}

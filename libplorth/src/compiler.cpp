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

namespace plorth
{
  namespace
  {
    class compiler
    {
    public:
      explicit compiler(const std::u32string& source,
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
        read();
      }

      /**
       * Advances to the next character in source code and returns the current
       * one.
       */
      inline char32_t read()
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

      /**
       * Returns next character to be read from the source code without
       * advancing any further.
       */
      inline char32_t peek() const
      {
        return *m_pos;
      }

      /**
       * Returns true if next character to be read from the source code equals
       * with one given as argument.
       */
      inline bool peek(char32_t expected) const
      {
        return !eof() && peek() == expected;
      }

      /**
       * Returns true if next character to be read from the source code matches
       * with given callback function.
       */
      inline bool peek(bool (*callback)(char32_t)) const
      {
        return !eof() && callback(peek());
      }

      /**
       * Returns true and advances to next character if next character to be
       * read from the source code equals with one given as argument.
       */
      inline bool peek_read(char32_t expected)
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
      inline bool peek_read(char32_t expected, char32_t& slot)
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
      std::shared_ptr<quote> compile(context* ctx)
      {
        std::vector<std::shared_ptr<class value>> values;
        std::shared_ptr<class value> value;

        while (!eof())
        {
          if (skip_whitespace())
          {
            break;
          }
          else if (!(value = compile_value(ctx)))
          {
            return std::shared_ptr<quote>();
          }
          values.push_back(value);
        }

        return ctx->runtime()->compiled_quote(values);
      }

      std::shared_ptr<value> compile_value(context* ctx)
      {
        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing value.",
            &m_position
          );

          return std::shared_ptr<value>();
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

      std::shared_ptr<symbol> compile_symbol(context* ctx)
      {
        struct position position;
        std::u32string buffer;

        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing symbol.",
            &m_position
          );

          return std::shared_ptr<symbol>();
        }

        position = m_position;

        if (!unicode_isword(peek()))
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected input; Missing symbol.",
            &m_position
          );

          return std::shared_ptr<symbol>();
        }

        buffer.append(1, read());
        while (peek(unicode_isword))
        {
          buffer.append(1, read());
        }

        return ctx->runtime()->symbol(buffer, &position);
      }

      std::shared_ptr<word> compile_word(context* ctx)
      {
        struct position position;
        const auto& runtime = ctx->runtime();
        std::shared_ptr<class symbol> symbol;
        std::vector<std::shared_ptr<value>> values;

        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing word.",
            &m_position
          );

          return std::shared_ptr<word>();
        }

        position = m_position;

        if (!peek_read(':'))
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected input; Missing word.",
            &position
          );

          return std::shared_ptr<word>();
        }

        if (!(symbol = compile_symbol(ctx)))
        {
          return std::shared_ptr<word>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code::syntax,
              U"Unterminated word; Missing `;'.",
              &position
            );

            return std::shared_ptr<word>();
          }
          else if (peek_read(';'))
          {
            break;
          } else {
            const auto value = compile_value(ctx);

            if (!value)
            {
              return std::shared_ptr<word>();
            }
            values.push_back(value);
          }
        }

        return runtime->word(symbol, runtime->compiled_quote(values));
      }

      std::shared_ptr<quote> compile_quote(context* ctx)
      {
        struct position position;
        std::vector<std::shared_ptr<value>> values;

        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing quote.",
            &m_position
          );

          return std::shared_ptr<quote>();
        }

        position = m_position;

        if (!peek_read('('))
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected input; Missing quote.",
            &position
          );

          return std::shared_ptr<quote>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code::syntax,
              U"Unterminated quote; Missing `)'.",
              &position
            );

            return std::shared_ptr<quote>();
          }
          else if (peek_read(')'))
          {
            break;
          } else {
            const auto value = compile_value(ctx);

            if (!value)
            {
              return std::shared_ptr<quote>();
            }
            values.push_back(value);
          }
        }

        return ctx->runtime()->compiled_quote(values);
      }

      std::shared_ptr<string> compile_string(context* ctx)
      {
        struct position position;
        char32_t separator;
        std::u32string buffer;

        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing string.",
            &m_position
          );

          return std::shared_ptr<string>();
        }

        position = m_position;

        if (!peek_read('"', separator) && !peek_read('\'', separator))
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected input; Missing string.",
            &position
          );

          return std::shared_ptr<string>();
        }

        for (;;)
        {
          if (eof())
          {
            ctx->error(
              error::code::syntax,
              std::u32string(U"Unterminated string; Missing `") + separator + U"'",
              &position
            );

            return std::shared_ptr<string>();
          }
          else if (peek_read(separator))
          {
            break;
          }
          else if (peek_read('\\'))
          {
            if (!compile_escape_sequence(ctx, buffer))
            {
              return std::shared_ptr<string>();
            }
          } else {
            buffer.append(1, read());
          }
        }

        return ctx->runtime()->string(buffer);
      }

      std::shared_ptr<array> compile_array(context* ctx)
      {
        struct position position;
        std::vector<std::shared_ptr<value>> elements;

        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing array.",
            &m_position
          );

          return std::shared_ptr<array>();
        }

        position = m_position;

        if (!peek_read('['))
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected input; Missing array.",
            &position
          );

          return std::shared_ptr<array>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code::syntax,
              U"Unterminated array; Missing `]'.",
              &position
            );

            return std::shared_ptr<array>();
          }
          else if (peek_read(']'))
          {
            break;
          } else {
            const auto value = compile_value(ctx);

            if (!value)
            {
              return std::shared_ptr<array>();
            }
            elements.push_back(value);
            if (skip_whitespace() || (!peek(',') && !peek(']')))
            {
              ctx->error(
                error::code::syntax,
                U"Unterminated array; Missing `]'.",
                &position
              );

              return std::shared_ptr<array>();
            } else {
              peek_read(',');
            }
          }
        }

        return ctx->runtime()->array(elements.data(), elements.size());
      }

      std::shared_ptr<object> compile_object(context* ctx)
      {
        struct position position;
        std::vector<object::value_type> properties;

        if (skip_whitespace())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing object.",
            &m_position
          );

          return std::shared_ptr<object>();
        }

        position = m_position;

        if (!peek_read('{'))
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected input; Missing object.",
            &position
          );

          return std::shared_ptr<object>();
        }

        for (;;)
        {
          if (skip_whitespace())
          {
            ctx->error(
              error::code::syntax,
              U"Unterminated object; Missing `}'.",
              &position
            );

            return std::shared_ptr<object>();
          }
          else if (peek_read('}'))
          {
            break;
          } else {
            std::shared_ptr<string> key;
            std::shared_ptr<class value> value;

            if (!(key = compile_string(ctx)))
            {
              return std::shared_ptr<object>();
            }

            if (skip_whitespace())
            {
              ctx->error(
                error::code::syntax,
                U"Unterminated object; Missing `}'.",
                &position
              );

              return std::shared_ptr<object>();
            }

            if (!peek_read(':'))
            {
              ctx->error(
                error::code::syntax,
                U"Missing `:' after property key.",
                &m_position
              );

              return std::shared_ptr<object>();
            }

            if (!(value = compile_value(ctx)))
            {
              return std::shared_ptr<object>();
            }

            properties.push_back({ key->to_string(), value });

            if (skip_whitespace() || (!peek(',') && !peek('}')))
            {
              ctx->error(
                error::code::syntax,
                U"Unterminated object; Missing `}'.",
                &position
              );

              return std::shared_ptr<object>();
            } else {
              peek_read(',');
            }
          }
        }

        return ctx->runtime()->object(properties);
      }

      bool compile_escape_sequence(context* ctx, std::u32string& buffer)
      {
        struct position position;

        if (eof())
        {
          ctx->error(
            error::code::syntax,
            U"Unexpected end of input; Missing escape sequence.",
            &m_position
          );

          return false;
        }

        position = m_position;

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
                ctx->error(
                  error::code::syntax,
                  U"Unterminated escape sequence.",
                  &position
                );

                return false;
              }
              else if (!std::isxdigit(peek()))
              {
                ctx->error(
                  error::code::syntax,
                  U"Illegal Unicode hex escape sequence.",
                  &position
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

            if (!unicode_validate(result))
            {
              ctx->error(
                error::code::syntax,
                U"Illegal Unicode hex escape sequence.",
                &position
              );

              return false;
            }

            buffer.append(1, result);
          }
          break;

        default:
          ctx->error(
            error::code::syntax,
            U"Illegal escape sequence in string literal.",
            &position
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

      compiler(const compiler&) = delete;
      void operator=(const compiler&) = delete;

    private:
      /** Current position in source code. */
      std::u32string::const_iterator m_pos;
      /** Iterator which marks end of the source code. */
      const std::u32string::const_iterator m_end;
      /** Current source code location information. */
      position m_position;
    };
  }

  std::shared_ptr<quote> context::compile(const std::u32string& source,
                                          const std::u32string& filename,
                                          int line,
                                          int column)
  {
    return compiler(source, filename, line, column).compile(this);
  }
}

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
  using source_iterator = unistring::const_iterator;

  static bool skip_whitespace(source_iterator&, const source_iterator&);
  static ref<value> compile_value(context*,
                                  source_iterator&,
                                  const source_iterator&);

  ref<quote> context::compile(const unistring& source)
  {
    std::vector<ref<class value>> values;
    ref<class value> value;
    auto it = std::begin(source);
    const auto end = std::end(source);

    while (it != end)
    {
      if (skip_whitespace(it, end))
      {
        break;
      }
      else if (!(value = compile_value(this, it, end)))
      {
        return ref<quote>();
      }
      values.push_back(value);
    }

    return m_runtime->compiled_quote(values);
  }

  /**
   * Skips whitespace and comments from source code.
   *
   * \return True if end of input has been reached, false otherwise.
   */
  static bool skip_whitespace(source_iterator& it, const source_iterator& end)
  {
    while (it != end)
    {
      // Skip line comments.
      if (*it == '#')
      {
        while (++it != end)
        {
          if (*it == '\n' || *it == '\r')
          {
            break;
          }
        }
      }
      else if (!std::isspace(*it))
      {
        return false;
      } else {
        ++it;
      }
    }

    return true;
  }

  static bool compile_escape_sequence(context* ctx,
                                      source_iterator& it,
                                      const source_iterator& end,
                                      unistring& buffer)
  {
    if (it >= end)
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing escape sequence."
      );

      return false;
    }

    switch (*it++)
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
      buffer.append(1, *(it - 1));
      break;

    case 'u':
      {
        unichar result = 0;

        for (int i = 0; i < 4; ++i)
        {
          if (it >= end)
          {
            ctx->error(
              error::code_syntax,
              U"Unterminated escape sequence."
            );

            return false;
          }
          else if (!std::isxdigit(*it))
          {
            ctx->error(
              error::code_syntax,
              U"Illegal Unicode hex escape sequence."
            );

            return false;
          }

          if (*it >= 'A' && *it <= 'F')
          {
            result = result * 16 + (*it++ - 'A' + 10);
          }
          else if (*it >= 'a' && *it <= 'f')
          {
            result = result * 16 + (*it++ - 'a' + 10);
          } else {
            result = result * 16 + (*it++ - '0');
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

  static ref<string> compile_string(context* ctx,
                                    source_iterator& it,
                                    const source_iterator& end)
  {
    unichar separator;
    unistring buffer;

    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing string."
      );

      return ref<string>();
    }

    if (*it != '"' && *it != '\'')
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected input; Missing string."
      );

      return ref<string>();
    }

    separator = *it++;

    for (;;)
    {
      if (it >= end)
      {
        ctx->error(
          error::code_syntax,
          unistring(U"Unterminated string; Missing `") + separator + U"'"
        );

        return ref<string>();
      }
      else if (*it == separator)
      {
        ++it;
        break;
      }
      else if (*it != '\\')
      {
        buffer.append(1, *it++);
      }
      else if (!compile_escape_sequence(ctx, ++it, end, buffer))
      {
        return ref<string>();
      }
    }

    return ctx->runtime()->string(buffer);
  }

  static ref<quote> compile_quote(context* ctx,
                                  source_iterator& it,
                                  const source_iterator& end)
  {
    std::vector<ref<value>> values;

    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing quote."
      );

      return ref<quote>();
    }

    if (*it != '(')
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected input; Missing quote."
      );

      return ref<quote>();
    }

    ++it;

    for (;;)
    {
      if (skip_whitespace(it, end))
      {
        ctx->error(error::code_syntax, U"Unterminated quote; Missing `)'.");

        return ref<quote>();
      }
      else if (*it == ')')
      {
        ++it;
        break;
      } else {
        const auto value = compile_value(ctx, it, end);

        if (!value)
        {
          return ref<quote>();
        }
        values.push_back(value);
      }
    }

    return ctx->runtime()->compiled_quote(values);
  }

  static ref<array> compile_array(context* ctx,
                                  source_iterator& it,
                                  const source_iterator& end)
  {
    std::vector<ref<value>> elements;

    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing array."
      );

      return ref<array>();
    }

    if (*it != '[')
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected input; Missing array."
      );

      return ref<array>();
    }

    ++it;

    for (;;)
    {
      if (skip_whitespace(it, end))
      {
        ctx->error(
          error::code_syntax,
          U"Unterminated array; Missing `]'."
        );

        return ref<array>();
      }
      else if (*it == ']')
      {
        ++it;
        break;
      } else {
        const auto value = compile_value(ctx, it, end);

        if (!value)
        {
          return ref<array>();
        }
        elements.push_back(value);
        if (skip_whitespace(it, end) || (*it != ',' && *it != ']'))
        {
          ctx->error(
            error::code_syntax,
            U"Unterminated array; Missing `]'."
          );

          return ref<array>();
        }
        else if (*it == ',')
        {
          ++it;
        }
      }
    }

    return ctx->runtime()->array(elements.data(), elements.size());
  }

  static ref<object> compile_object(context* ctx,
                                    source_iterator& it,
                                    const source_iterator& end)
  {
    object::container_type properties;

    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing object."
      );

      return ref<object>();
    }

    if (*it != '{')
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected input; Missing object."
      );

      return ref<object>();
    }

    ++it;

    for (;;)
    {
      if (skip_whitespace(it, end))
      {
        ctx->error(
          error::code_syntax,
          U"Unterminated object; Missing `}'."
        );

        return ref<object>();
      }
      else if (*it == '}')
      {
        ++it;
        break;
      } else {
        ref<string> key;
        ref<class value> value;

        if (!(key = compile_string(ctx, it, end)))
        {
          return ref<object>();
        }

        if (skip_whitespace(it, end))
        {
          ctx->error(
            error::code_syntax,
            U"Unterminated object; Missing `}'."
          );

          return ref<object>();
        }

        if (*it != ':')
        {
          ctx->error(
            error::code_syntax,
            U"Missing `:' after property key."
          );

          return ref<object>();
        }

        if (!(value = compile_value(ctx, ++it, end)))
        {
          return ref<object>();
        }

        properties[key->to_string()] = value;

        if (skip_whitespace(it, end) || (*it != ',' && *it != '}'))
        {
          ctx->error(
            error::code_syntax,
            U"Unterminated object; Missing `}'."
          );

          return ref<object>();
        }
        else if (*it == ',')
        {
          ++it;
        }
      }
    }

    return ctx->runtime()->value<object>(properties);
  }

  static ref<symbol> compile_symbol(context* ctx,
                                    source_iterator& it,
                                    const source_iterator& end)
  {
    unistring buffer;

    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing symbol."
      );

      return ref<symbol>();
    }

    if (!unichar_isword(*it))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected input; Missing symbol."
      );

      return ref<symbol>();
    }

    buffer.append(1, *it++);
    while (it != end && unichar_isword(*it))
    {
      buffer.append(1, *it++);
    }

    return ctx->runtime()->value<symbol>(buffer);
  }

  static ref<word> compile_word(context* ctx,
                                source_iterator& it,
                                const source_iterator& end)
  {
    const auto& runtime = ctx->runtime();
    ref<class symbol> symbol;
    std::vector<ref<value>> values;

    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing word."
        );

      return ref<word>();
    }

    if (*it != ':')
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected input; Missing word."
        );

      return ref<word>();
    }

    ++it;

    if (!(symbol = compile_symbol(ctx, it, end)))
    {
      return ref<word>();
    }

    for (;;)
    {
      if (skip_whitespace(it, end))
      {
        ctx->error(error::code_syntax, U"Unterminated word; Missing `;'.");

        return ref<word>();
      }
      else if (*it == ';')
      {
        ++it;
        break;
      } else {
        const auto value = compile_value(ctx, it, end);

        if (!value)
        {
          return ref<word>();
        }
        values.push_back(value);
      }
    }

    return runtime->value<word>(symbol, runtime->compiled_quote(values));
  }

  static ref<value> compile_value(context* ctx,
                                  source_iterator& it,
                                  const source_iterator& end)
  {
    if (skip_whitespace(it, end))
    {
      ctx->error(
        error::code_syntax,
        U"Unexpected end of input; Missing value."
      );

      return ref<value>();
    }
    switch (*it)
    {
    case '"':
    case '\'':
      return compile_string(ctx, it, end);

    case '(':
      return compile_quote(ctx, it, end);

    case '[':
      return compile_array(ctx, it, end);

    case '{':
      return compile_object(ctx, it, end);

    case ':':
      return compile_word(ctx, it, end);

    default:
      return compile_symbol(ctx, it, end);
    }
  }
}

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
#include <plorth/unicode.hpp>
#include <plorth/value-number.hpp>
#include <plorth/value-string.hpp>

#include "./utils.hpp"

#include <algorithm>

namespace plorth
{
  string::string(const unistring& value)
    : m_value(value) {}

  bool string::equals(const ref<class value>& that) const
  {
    if (!that || !that->is(type_string))
    {
      return false;
    }

    return !m_value.compare(that.cast<string>()->m_value);
  }

  unistring string::to_string() const
  {
    return m_value;
  }

  unistring string::to_source() const
  {
    return json_stringify(m_value);
  }

  /**
   * length ( string -- string number )
   *
   * Returns length of the string.
   */
  static void w_length(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      ctx->push_number(str->value().length());
    }
  }

  /**
   * space? ( string -- string boolean )
   *
   * Tests whether the string contains only whitespace characters. Empty strings
   * return false.
   */
  static void w_is_space(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& input = str->value();
      const auto length = input.length();

      ctx->push(str);
      if (!length)
      {
        ctx->push_boolean(false);
      }
      for (std::size_t i = 0; i < length; ++i)
      {
        if (!unichar_isspace(input[i]))
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * lower-case? ( string -- string boolean )
   *
   * Tests whether the string contains only lower case characters. Empty strings
   * return false.
   */
  static void w_is_lower_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& input = str->value();
      const auto length = input.length();

      ctx->push(str);
      if (!length)
      {
        ctx->push_boolean(false);
      }
      for (std::size_t i = 0; i < length; ++i)
      {
        if (!unichar_islower(input[i]))
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * upper-case? ( string -- string boolean )
   *
   * Tests whether the string contains only upper case characters. Empty strings
   * return false.
   */
  static void w_is_upper_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& input = str->value();
      const auto length = input.length();

      ctx->push(str);
      if (!length)
      {
        ctx->push_boolean(false);
      }
      for (std::size_t i = 0; i < length; ++i)
      {
        if (!unichar_isupper(input[i]))
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * chars ( string -- string array )
   *
   * Extracts characters from the string and returns them in an array of
   * substrings.
   */
  static void w_chars(const ref<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& input = str->value();
      const auto length = input.length();
      array::container_type output;

      output.reserve(length);
      for (std::size_t i = 0; i < length; ++i)
      {
        output.push_back(runtime->value<string>(input.substr(i, 1)));
      }
      ctx->push(str);
      ctx->push_array(output);
    }
  }

  /**
   * runes ( string -- string array )
   *
   * Extracts Unicode code points from the string and returns them in an array
   * of numbers.
   */
  static void w_runes(const ref<context>& ctx)
  {
    const ref<class runtime>& runtime = ctx->runtime();
    ref<string> str;
    std::vector<ref<value>> result;

    if (!ctx->pop_string(str))
    {
      return;
    }

    for (const auto& c : str->value())
    {
      result.push_back(runtime->value<number>(c));
    }

    ctx->push(str);
    ctx->push_array(result);
  }

  /**
   * words ( string -- string array )
   *
   * Extracts white space separated words from the string and returns them in
   * an array.
   */
  static void w_words(const ref<context>& ctx)
  {
    const ref<class runtime>& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& s = str->value();
      const std::size_t length = s.length();
      std::size_t begin = 0;
      std::size_t end = 0;
      std::vector<ref<value>> result;

      for (std::size_t i = 0; i < length; ++i)
      {
        if (unichar_isspace(s[i]))
        {
          if (end - begin > 0)
          {
            result.push_back(runtime->value<string>(s.substr(begin, end - begin)));
          }
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(runtime->value<string>(s.substr(begin, end - begin)));
      }

      ctx->push(str);
      ctx->push_array(result);
    }
  }

  /**
   * lines ( string -- string array )
   *
   * Extracts lines from the string and returns them in an array.
   */
  static void w_lines(const ref<context>& ctx)
  {
    const ref<class runtime>& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& s = str->value();
      const std::size_t length = s.length();
      std::size_t begin = 0;
      std::size_t end = 0;
      std::vector<ref<value>> result;

      for (std::size_t i = 0; i < length; ++i)
      {
        if (i + 1 < length && s[i] == '\r' && s[i + 1] == '\n')
        {
          result.push_back(runtime->value<string>(s.substr(begin, end - begin)));
          begin = end = ++i + 1;
        }
        else if (s[i] == '\n' || s[i] == '\r')
        {
          result.push_back(runtime->value<string>(s.substr(begin, end - begin)));
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(runtime->value<string>(s.substr(begin, end - begin)));
      }

      ctx->push(str);
      ctx->push_array(result);
    }
  }

  /**
   * reverse ( string -- string )
   *
   * Returns reversed copy of the string.
   */
  static void w_reverse(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& s = str->value();

      ctx->push_string(unistring(s.rbegin(), s.rend()));
    }
  }

  /*
   * upper-case ( string -- string )
   *
   * Converts string into upper case.
   */
  static void w_upper_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      unistring result = str->value();

      std::transform(result.begin(), result.end(), result.begin(), unichar_toupper);
      ctx->push_string(result);
    }
  }

  /**
   * lower-case ( string -- string )
   *
   * Converts string into lower case.
   */
  static void w_lower_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      unistring result = str->value();

      std::transform(result.begin(), result.end(), result.begin(), unichar_tolower);
      ctx->push_string(result);
    }
  }

  /**
   * swap-case ( string -- string )
   *
   * Constructs copy of string where lower case characters have been turned into
   * upper case and vice versa.
   */
  static void w_swap_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& input = str->value();
      const auto length = input.length();
      unistring output;

      output.reserve(length);
      for (std::size_t i = 0; i < length; ++i)
      {
        unichar c = input[i];

        if (unichar_islower(c))
        {
          c = unichar_toupper(c);
        } else {
          c = unichar_tolower(c);
        }
        output.append(1, c);
      }
      ctx->push_string(output);
    }
  }

  /**
   * capitalize ( string -- string )
   *
   * Converts first character of the string into upper case and remaining to
   * lower case.
   */
  static void w_capitalize(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const unistring& input = str->value();
      const auto length = input.length();
      unistring output;

      output.reserve(length);
      for (std::size_t i = 0; i < length; ++i)
      {
        unichar c = input[i];

        if (i == 0)
        {
          c = unichar_toupper(c);
        } else {
          c = unichar_tolower(c);
        }
        output.append(1, c);
      }
      ctx->push_string(output);
    }
  }

  /**
   * + ( string string -- string )
   *
   * Concatenates contents of two strings and returns the result.
   */
  static void w_concat(const ref<context>& ctx)
  {
    ref<string> a;
    ref<string> b;

    if (ctx->pop_string(a) && ctx->pop_string(b))
    {
      ctx->push_string(b->value() + a->value());
    }
  }

  /**
   * * ( number string -- string )
   *
   * Repeats string given number of times.
   */
  static void w_repeat(const ref<context>& ctx)
  {
    ref<string> str;
    double count;

    if (ctx->pop_string(str) && ctx->pop_number(count))
    {
      const unistring& s = str->value();
      unistring result;

      if (count < 0.0)
      {
        count = -count;
      }

      result.reserve(s.length() * count);

      while (count >= 1.0)
      {
        count -= 1.0;
        result.append(s);
      }

      ctx->push_string(result);
    }
  }

  /**
   * @ ( number string -- string string )
   *
   * Retrieves character from given index. Negative indexes count backwards. If
   * given index is out of bounds, range error will be thrown.
   */
  static void w_get(const ref<context>& ctx)
  {
    ref<string> str;
    double index;

    if (ctx->pop_string(str) && ctx->pop_number(index))
    {
      const unistring& s = str->value();

      if (index < 0.0)
      {
        index += s.length();
      }

      ctx->push(str);

      if (index < 0.0 || index > s.length())
      {
        ctx->error(error::code_range, "String index out of bounds.");
        return;
      }

      ctx->push_string(unistring(1, s[index]));
    }
  }

  namespace api
  {
    runtime::prototype_definition string_prototype()
    {
      return
      {
        { "length", w_length },
        { "chars", w_chars },
        { "runes", w_runes },
        { "words", w_words },
        { "lines", w_lines },

        // Tests.
        { "space?", w_is_space },
        { "lower-case?", w_is_lower_case },
        { "upper-case?", w_is_upper_case },

        // Conversions.
        { "reverse", w_reverse },
        { "upper-case", w_upper_case },
        { "lower-case", w_lower_case },
        { "swap-case", w_swap_case },
        { "capitalize", w_capitalize },

        { "+", w_concat },
        { "*", w_repeat },
        { "@", w_get }
      };
    }
  }
}

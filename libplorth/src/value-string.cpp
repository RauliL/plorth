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
#include <plorth/unicode.hpp>

#include "./utils.hpp"

#include <algorithm>
#include <cstring>

namespace plorth
{
  namespace
  {
    class simple_string : public string
    {
    public:
      explicit simple_string(const char32_t* chars, size_type length)
        : m_length(length)
        , m_chars(m_length > 0 ? new char32_t[m_length] : nullptr)
      {
        if (m_length > 0)
        {
          std::memcpy(m_chars, chars, sizeof(char32_t) * m_length);
        }
      }

      ~simple_string()
      {
        if (m_length > 0)
        {
          delete[] m_chars;
        }
      }

      inline size_type length() const
      {
        return m_length;
      }

      value_type at(size_type offset) const
      {
        return m_chars[offset];
      }

    private:
      const size_type m_length;
      char32_t* m_chars;
    };

    class concat_string : public string
    {
    public:
      explicit concat_string(const std::shared_ptr<string>& left,
                             const std::shared_ptr<string>& right)
        : m_length(left->length() + right->length())
        , m_left(left)
        , m_right(right) {}

      inline size_type length() const
      {
        return m_length;
      }

      value_type at(size_type offset) const
      {
        const size_type left_length = m_left->length();

        if (offset < left_length)
        {
          return m_left->at(offset);
        } else {
          return m_right->at(offset - left_length);
        }
      }

    private:
      const size_type m_length;
      const std::shared_ptr<string> m_left;
      const std::shared_ptr<string> m_right;
    };

    class substring : public string
    {
    public:
      explicit substring(const std::shared_ptr<string>& original,
                         size_type offset,
                         size_type length)
        : m_original(original)
        , m_offset(offset)
        , m_length(length) {}

      inline size_type length() const
      {
        return m_length;
      }

      value_type at(size_type offset) const
      {
        return m_original->at(m_offset + offset);
      }

    private:
      const std::shared_ptr<string> m_original;
      const size_type m_offset;
      const size_type m_length;
    };

    /**
     * Implementation of string which reverses already existing string.
     */
    class reversed_string : public string
    {
    public:
      explicit reversed_string(const std::shared_ptr<string>& original)
        : m_original(original) {}

      inline size_type length() const
      {
        return m_original->length();
      }

      value_type at(size_type offset) const
      {
        return m_original->at(length() - offset - 1);
      }

    private:
      const std::shared_ptr<string> m_original;
    };
  }

  bool string::equals(const std::shared_ptr<class value>& that) const
  {
    const size_type len = length();
    std::shared_ptr<string> str;

    if (!is(that, type::string))
    {
      return false;
    }
    str = std::static_pointer_cast<string>(that);
    if (len != str->length())
    {
      return false;
    }
    for (size_type i = 0; i < len; ++i)
    {
      if (at(i) != str->at(i))
      {
        return false;
      }
    }

    return true;
  }

  std::u32string string::to_string() const
  {
    const size_type len = length();
    std::u32string result;

    result.reserve(len);
    for (size_type i = 0; i < len; ++i)
    {
      result.append(1, at(i));
    }

    return result;
  }

  std::u32string string::to_source() const
  {
    return json_stringify(to_string());
  }

  string::iterator::iterator(const std::shared_ptr<string>& str,
                             string::size_type index)
    : m_string(str)
    , m_index(index) {}

  string::iterator::iterator(const iterator& that)
    : m_string(that.m_string)
    , m_index(that.m_index) {}

  string::iterator& string::iterator::operator=(const iterator& that)
  {
    m_string = that.m_string;
    m_index = that.m_index;

    return *this;
  }

  string::iterator& string::iterator::operator++()
  {
    ++m_index;

    return *this;
  }

  string::iterator string::iterator::operator++(int)
  {
    const iterator copy(*this);

    ++m_index;

    return copy;
  }

  string::iterator::value_type string::iterator::operator*()
  {
    return m_string->at(m_index);
  }

  bool string::iterator::operator==(const iterator& that) const
  {
    return m_index == that.m_index;
  }

  bool string::iterator::operator!=(const iterator& that) const
  {
    return m_index != that.m_index;
  }

  std::shared_ptr<string> runtime::string(const std::u32string& input)
  {
    return string(input.c_str(), input.length());
  }

  std::shared_ptr<string> runtime::string(string::const_pointer chars,
                                          string::size_type length)
  {
    return std::shared_ptr<class string>(
      new (*m_memory_manager) simple_string(chars, length)
    );
  }

  /**
   * Word: length
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - number
   *
   * Returns the length of the string.
   */
  static void w_length(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      ctx->push(str);
      ctx->push_int(str->length());
    }
  }

  static void str_test(const std::shared_ptr<context>& ctx,
                       bool (*callback)(char32_t))
  {
    std::shared_ptr<string> str;

    if (!ctx->pop_string(str))
    {
      return;
    }
    ctx->push(str);
    if (str->empty())
    {
      ctx->push_boolean(false);
      return;
    }
    for (const auto c : str)
    {
      if (!callback(c))
      {
        ctx->push_boolean(false);
        return;
      }
    }
    ctx->push_boolean(true);
  }

  /**
   * Word: includes?
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the topmost string contains contents of the second string,
   * returning true or false as appropriate.
   */
  static void w_includes(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<string> substr;

    if (!ctx->pop_string(str) || !ctx->pop_string(substr))
    {
      return;
    }

    const auto str_length = str->length();
    const auto substr_length = substr->length();

    if (substr_length > str_length)
    {
      ctx->push(str);
      ctx->push_boolean(false);
      return;
    }
    else if (!substr_length)
    {
      ctx->push(str);
      ctx->push_boolean(true);
      return;
    }

    for (string::size_type i = 0; i < str_length; ++i)
    {
      bool found = true;

      if (i + substr_length > str_length)
      {
        break;
      }
      for (string::size_type j = 0; j < substr_length; ++j)
      {
        if (str->at(i + j) != substr->at(j))
        {
          found = false;
          break;
        }
      }
      if (found)
      {
        ctx->push(str);
        ctx->push_boolean(true);
        return;
      }
    }

    ctx->push(str);
    ctx->push_boolean(false);
  }

  /**
   * Word: index-of
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   * - number|null
   *
   * Searches for the first occurrence of a substring given as second topmost
   * value of the stack from string given as topmost value of the stack. If the
   * substring does not exist in the string, null will be returned. Otherwise,
   * first numerical index of the occurrence is returned.
   */
  static void w_index_of(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<string> substr;

    if (!ctx->pop_string(str) || !ctx->pop_string(substr))
    {
      return;
    }

    const auto str_length = str->length();
    const auto substr_length = substr->length();

    ctx->push(str);

    if (substr_length > str_length)
    {
      ctx->push_null();
      return;
    }
    else if (!substr_length)
    {
      ctx->push_int(0);
      return;
    }

    for (string::size_type i = 0; i < str_length; ++i)
    {
      bool found = true;

      if (i + substr_length > str_length)
      {
        break;
      }
      for (string::size_type j = 0; j < substr_length; ++j)
      {
        if (str->at(i + j) != substr->at(j))
        {
          found = false;
          break;
        }
      }
      if (found)
      {
        ctx->push_int(i);
        return;
      }
    }

    ctx->push_null();
  }

  /**
   * Word: last-index-of
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   * - number|null
   *
   * Searches for the last occurrence of a substring given as second topmost
   * value of the stack from string given as topmost value of the stack. If the
   * substring does not exist in the string, null will be returned. Otherwise,
   * last numerical index of the occurrence is returned.
   */
  static void w_last_index_of(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<string> substr;

    if (!ctx->pop_string(str) || !ctx->pop_string(substr))
    {
      return;
    }

    const auto str_length = str->length();
    const auto substr_length = substr->length();

    ctx->push(str);

    if (substr_length > str_length)
    {
      ctx->push_null();
      return;
    }
    else if (!substr_length)
    {
      ctx->push_int(str_length);
      return;
    }

    for (auto i = str_length; i > 0; --i)
    {
      bool found = true;

      if (str_length - i + 1 < substr_length)
      {
        continue;
      }
      for (string::size_type j = 0; j < substr_length; ++j)
      {
        if (str->at(i + j - 1) != substr->at(j))
        {
          found = false;
          break;
        }
      }
      if (found)
      {
        ctx->push_int(i - 1);
        return;
      }
    }

    ctx->push_null();
  }

  /**
   * Word: starts-with?
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether beginning of the string is identical with the given
   * substring.
   */
  static void w_starts_with(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<string> substr;

    if (!ctx->pop_string(str) || !ctx->pop_string(substr))
    {
      return;
    }

    const auto str_length = str->length();
    const auto substr_length = substr->length();

    if (substr_length > str_length)
    {
      ctx->push(str);
      ctx->push_boolean(false);
      return;
    }
    else if (!substr_length)
    {
      ctx->push(str);
      ctx->push_boolean(true);
      return;
    }

    for (string::size_type i = 0; i < substr_length; ++i)
    {
      if (str->at(i) != substr->at(i))
      {
        ctx->push(str);
        ctx->push_boolean(false);
        return;
      }
    }

    ctx->push(str);
    ctx->push_boolean(true);
  }

  /**
   * Word: ends-with?
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether end of the string is identical with the given substring.
   */
  static void w_ends_with(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<string> substr;

    if (!ctx->pop_string(str) || !ctx->pop_string(substr))
    {
      return;
    }

    const auto str_length = str->length();
    const auto substr_length = substr->length();

    if (substr_length > str_length)
    {
      ctx->push(str);
      ctx->push_boolean(false);
      return;
    }
    else if (!substr_length)
    {
      ctx->push(str);
      ctx->push_boolean(true);
      return;
    }

    for (string::size_type i = 0; i < substr_length; ++i)
    {
      if (str->at(str_length - substr_length + i) != substr->at(i))
      {
        ctx->push(str);
        ctx->push_boolean(false);
        return;
      }
    }

    ctx->push(str);
    ctx->push_boolean(true);
  }

  /**
   * Word: space?
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the string contains only whitespace characters. Empty
   * strings return false.
   */
  static void w_is_space(const std::shared_ptr<context>& ctx)
  {
    str_test(ctx, unicode_isspace);
  }

  /**
   * Word: lower-case?
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the string contains only lower case characters. Empty
   * strings return false.
   */
  static void w_is_lower_case(const std::shared_ptr<context>& ctx)
  {
    str_test(ctx, unicode_islower);
  }

  /**
   * Word: upper-case?
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the string contains only upper case characters. Empty strings
   * return false.
   */
  static void w_is_upper_case(const std::shared_ptr<context>& ctx)
  {
    str_test(ctx, unicode_isupper);
  }

  /**
   * Word: chars
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts characters from the string and returns them in an array of
   * substrings.
   */
  static void w_chars(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      std::vector<std::shared_ptr<value>> output;

      output.reserve(length);
      for (const auto c : str)
      {
        output.push_back(runtime->string(&c, 1));
      }
      ctx->push(str);
      ctx->push_array(output.data(), length);
    }
  }

  /**
   * Word: runes
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts Unicode code points from the string and returns them in an array
   * of numbers.
   */
  static void w_runes(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      std::vector<std::shared_ptr<value>> output;

      output.reserve(length);
      for (const auto c : str)
      {
        output.push_back(runtime->number(static_cast<number::int_type>(c)));
      }
      ctx->push(str);
      ctx->push_array(output.data(), length);
    }
  }

  /**
   * Word: words
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts white space separated words from the string and returns them in
   * an array.
   */
  static void w_words(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      string::size_type begin = 0;
      string::size_type end = 0;
      std::vector<std::shared_ptr<value>> result;

      for (string::size_type i = 0; i < length; ++i)
      {
        if (unicode_isspace(str->at(i)))
        {
          if (end - begin > 0)
          {
            result.push_back(runtime->value<substring>(str, begin, end - begin));
          }
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(runtime->value<substring>(str, begin, end - begin));
      }

      ctx->push(str);
      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: lines
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts lines from the string and returns them in an array.
   */
  static void w_lines(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      string::size_type begin = 0;
      string::size_type end = 0;
      std::vector<std::shared_ptr<value>> result;

      for (string::size_type i = 0; i < length; ++i)
      {
        const auto c = str->at(i);

        if (i + 1 < length && c == '\r' && str->at(i + 1) == '\n')
        {
          result.push_back(runtime->value<substring>(str, begin, end - begin));
          begin = end = ++i + 1;
        }
        else if (c == '\n' || c == '\r')
        {
          result.push_back(runtime->value<substring>(str, begin, end - begin));
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(runtime->value<substring>(str, begin, end - begin));
      }

      ctx->push(str);
      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: reverse
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Reverses the string.
   */
  static void w_reverse(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      ctx->push(ctx->runtime()->value<reversed_string>(str));
    }
  }

  static void str_convert(const std::shared_ptr<context>& ctx,
                          char32_t (*callback)(char32_t))
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      char32_t result[length];

      for (string::size_type i = 0; i < length; ++i)
      {
        result[i] = callback(str->at(i));
      }
      ctx->push_string(result, length);
    }
  }

  /*
   * Word: upper-case
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Converts the string into upper case.
   */
  static void w_upper_case(const std::shared_ptr<context>& ctx)
  {
    str_convert(ctx, unicode_toupper);
  }

  /**
   * Word: lower-case
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Converts the string into lower case.
   */
  static void w_lower_case(const std::shared_ptr<context>& ctx)
  {
    str_convert(ctx, unicode_tolower);
  }

  static inline char32_t unicode_swapcase(char32_t c)
  {
    if (unicode_islower(c))
    {
      return unicode_toupper(c);
    } else {
      return unicode_tolower(c);
    }
  }

  /**
   * Word: swap-case
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Turns lower case characters in the string into upper case and vice versa.
   */
  static void w_swap_case(const std::shared_ptr<context>& ctx)
  {
    str_convert(ctx, unicode_swapcase);
  }

  /**
   * Word: capitalize
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Converts the first character of the string into upper case and the
   * remaining characters into lower case.
   */
  static void w_capitalize(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      char32_t output[length];

      for (string::size_type i = 0; i < length; ++i)
      {
        auto c = str->at(i);

        if (i == 0)
        {
          c = unicode_toupper(c);
        } else {
          c = unicode_tolower(c);
        }
        output[i] = c;
      }
      ctx->push_string(output, length);
    }
  }

  /**
   * Word: trim
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Strips whitespace from the begining and the end of the string.
   */
  static void w_trim(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const string::size_type length = str->length();
      string::size_type i, j;

      for (i = 0; i < length; ++i)
      {
        if (!unicode_isspace(str->at(i)))
        {
          break;
        }
      }
      for (j = length; j != 0; --j)
      {
        if (!unicode_isspace(str->at(j - 1)))
        {
          break;
        }
      }
      if (i != 0 || j != length)
      {
        ctx->push(ctx->runtime()->value<substring>(str, i, j - i));
      } else {
        ctx->push(str);
      }
    }
  }

  /**
   * Word: trim-left
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Strips whitespace from the begining of the string.
   */
  static void w_trim_left(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const string::size_type length = str->length();
      string::size_type i;

      for (i = 0; i < length; ++i)
      {
        if (!unicode_isspace(str->at(i)))
        {
          break;
        }
      }
      if (i != 0)
      {
        ctx->push(ctx->runtime()->value<substring>(str, i, length - i));
      } else {
        ctx->push(str);
      }
    }
  }

  /**
   * Word: trim-right
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Strips whitespace from the end of the string.
   */
  static void w_trim_right(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const string::size_type length = str->length();
      string::size_type i;

      for (i = length; i != 0; --i)
      {
        if (!unicode_isspace(str->at(i - 1)))
        {
          break;
        }
      }
      if (i != length)
      {
        ctx->push(ctx->runtime()->value<substring>(str, 0, i));
      } else {
        ctx->push(str);
      }
    }
  }

  /**
   * Word: >number
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - number
   *
   * Converts string into a floating point decimal number, or throws a value
   * error if the number cannot be converted into one.
   */
  static void w_to_number(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> a;

    if (ctx->pop_string(a))
    {
      const auto str = a->to_string();

      if (is_number(str))
      {
        ctx->push_number(str);
      } else {
        ctx->error(error::code::value, U"Could not convert string to number.");
      }
    }
  }

  /**
   * Word: +
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   *
   * Concatenates the contents of the two strings and returns the result.
   */
  static void w_concat(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> a;
    std::shared_ptr<string> b;

    if (ctx->pop_string(a) && ctx->pop_string(b))
    {
      if (a->empty())
      {
        ctx->push(b);
      }
      else if (b->empty())
      {
        ctx->push(a);
      } else {
        ctx->push(ctx->runtime()->value<concat_string>(b, a));
      }
    }
  }

  /**
   * Word: *
   * Prototype: string
   *
   * Takes:
   * - number
   * - string
   *
   * Gives:
   * - string
   *
   * Repeats the string given number of times.
   */
  static void w_repeat(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<number> num;

    if (ctx->pop_string(str) && ctx->pop_number(num))
    {
      number::int_type count = num->as_int();

      if (count > 0)
      {
        const auto& runtime = ctx->runtime();
        std::shared_ptr<string> result = str;

        for (number::int_type i = 1; i < count; ++i)
        {
          result = runtime->value<concat_string>(result, str);
        }
        ctx->push(result);
      }
      else if (count == 0)
      {
        ctx->push_string(nullptr, 0);
      } else {
        ctx->error(error::code::range, U"Invalid repeat count.");
      }
    }
  }

  /**
   * Word: @
   * Prototype: string
   *
   * Takes:
   * - number
   * - string
   *
   * Gives:
   * - string
   * - string
   *
   * Retrieves a character at given index. Negative indices count backwards
   * from the end of the string. If given index is out of bounds, a range error
   * will be thrown.
   */
  static void w_get(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;
    std::shared_ptr<number> num;

    if (ctx->pop_string(str) && ctx->pop_number(num))
    {
      const auto length = str->length();
      number::int_type index = num->as_int();
      char32_t c;

      if (index < 0)
      {
        index += length;
      }

      ctx->push(str);

      if (!length || index < 0 || index >= static_cast<number::int_type>(length))
      {
        ctx->error(error::code::range, U"String index out of bounds.");
        return;
      }

      c = str->at(index);
      ctx->push(ctx->runtime()->string(&c, 1));
    }
  }

  /**
   * Word: >symbol
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - symbol
   *
   * Converts given string into symbol. Value error will be thrown if the string
   * is empty or contains whitespace or non-symbolic characters such as separators.
   */
  static void w_to_symbol(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();

      if (!length)
      {
        ctx->error(error::code::value, U"Cannot construct empty symbol.");
        return;
      }
      for (string::size_type i = 0; i < length; ++i)
      {
        if (!unicode_isword(str->at(i)))
        {
          ctx->error(
            error::code::value,
            U"Cannot convert " + str->to_source() + U" into symbol."
          );
          return;
        }
      }
      ctx->push_symbol(str->to_string());
    }
  }

  namespace api
  {
    runtime::prototype_definition string_prototype()
    {
      return
      {
        { U"length", w_length },
        { U"chars", w_chars },
        { U"runes", w_runes },
        { U"words", w_words },
        { U"lines", w_lines },

        // Tests.
        { U"includes?", w_includes },
        { U"index-of", w_index_of },
        { U"last-index-of", w_last_index_of },
        { U"starts-with?", w_starts_with },
        { U"ends-with?", w_ends_with },
        { U"space?", w_is_space },
        { U"lower-case?", w_is_lower_case },
        { U"upper-case?", w_is_upper_case },

        // Conversions.
        { U"reverse", w_reverse },
        { U"upper-case", w_upper_case },
        { U"lower-case", w_lower_case },
        { U"swap-case", w_swap_case },
        { U"capitalize", w_capitalize },
        { U"trim", w_trim },
        { U"trim-left", w_trim_left },
        { U"trim-right", w_trim_right },
        // TODO: pad-left
        // TODO: pad-right
        // TODO: substring
        // TODO: split
        // TODO: replace
        // TODO: normalize
        { U">number", w_to_number },

        { U"+", w_concat },
        { U"*", w_repeat },
        { U"@", w_get },

        // Type conversions.
        { U">symbol", w_to_symbol }
      };
    }
  }
}

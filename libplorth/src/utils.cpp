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
#include <plorth/unicode.hpp>

#include "./utils.hpp"

#include <cfloat>
#include <climits>
#include <cmath>

namespace plorth
{
#if PLORTH_ENABLE_32BIT_INT
  using uint_type = std::uint32_t;
#else
  using uint_type = std::uint64_t;
#endif

  static const char digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  static const std::u32string string_nan = {'n', 'a', 'n'};
  static const std::u32string string_inf = {'i', 'n', 'f'};
  static const std::u32string string_inf_neg = {'-', 'i', 'n', 'f'};

  std::u32string json_stringify(const std::u32string& input)
  {
    std::u32string result;

    result.reserve(input.length() + 2);
    result.append(1, '"');

    for (const auto& c : input)
    {
      switch (c)
      {
        case 010:
          result.append(1, '\\');
          result.append(1, 'b');
          break;

        case 011:
          result.append(1, '\\');
          result.append(1, 't');
          break;

        case 012:
          result.append(1, '\\');
          result.append(1, 'n');
          break;

        case 014:
          result.append(1, '\\');
          result.append(1, 'f');
          break;

        case 015:
          result.append(1, '\\');
          result.append(1, 'r');
          break;

        case '"':
        case '\\':
        case '/':
          result.append(1, '\\');
          result.append(1, c);
          break;

        default:
          if (unicode_iscntrl(c))
          {
            char buffer[7];

            std::snprintf(buffer, 7, "\\u%04x", c);
            for (const char* p = buffer; *p; ++p)
            {
              result.append(1, static_cast<char32_t>(*p));
            }
          } else {
            result.append(1, c);
          }
      }
    }

    result.append(1, '"');

    return result;
  }

  bool is_number(const std::u32string& input)
  {
    const auto length = input.length();
    std::u32string::size_type start;
    bool dot_seen = false;
    bool exponent_seen = false;

    if (!length)
    {
      return false;
    }

    if (input[0] == '+' || input[0] == '-')
    {
      start = 1;
      if (length < 2)
      {
        return false;
      }
    } else {
      start = 0;
    }

    for (auto i = start; i < length; ++i)
    {
      const auto c = input[i];

      if (c == '.')
      {
        if (dot_seen || exponent_seen || i == start || i + 1 > length)
        {
          return false;
        }
        dot_seen = true;
      }
      else if (c == 'e' || c == 'E')
      {
        if (exponent_seen || i == start || i + 2 > length)
        {
          return false;
        }
        if (input[i + 1] == '+' || input[i + 1] == '-')
        {
          if (i + 3 > length) {
            return false;
          }
          ++i;
        }
        exponent_seen = true;
      }
      else if (!std::isdigit(c))
      {
        return false;
      }
    }

    return true;
  }

  std::u32string to_unistring(number::int_type number)
  {
    const bool negative = number < 0;
    uint_type mag = static_cast<uint_type>(negative ? -number : number);
    std::u32string result;

    if (mag != 0)
    {
      result.reserve(negative ? 21 : 20);
      do
      {
        result.insert(result.begin(), digitmap[mag % 10]);
        mag /= 10;
      }
      while (mag);
    } else {
      result.insert(result.begin(), '0');
    }
    if (negative)
    {
      result.insert(result.begin(), '-');
    }

    return result;
  }

  std::u32string to_unistring(number::real_type number)
  {
    char buffer[20];

    if (std::isnan(number))
    {
      return string_nan;
    }
    else if (std::isinf(number))
    {
      return number < 0.0 ? string_inf_neg : string_inf;
    }
    std::snprintf(buffer, sizeof(buffer), "%g", number);

    return utf8_decode(buffer);
  }

  number::int_type to_integer(const std::u32string& input)
  {
    static const number::int_type div = number::int_max / 10;
    static const number::int_type rem = number::int_max % 10;
    number::int_type number = 0;
    const std::size_t length = input.length();
    std::size_t offset;
    bool sign;

    // Extract the sign.
    if (length > 0 && (input[0] == '+' || input[0] == '-'))
    {
      offset = 1;
      sign = input[0] == '+';
    } else {
      offset = 0;
      sign = true;
    }

    for (; offset < length; ++offset)
    {
      const auto c = input[offset];
      int digit;

      if (!std::isdigit(c))
      {
        continue;
      }
      digit = c - '0';
      if (number > div || (number == div && digit > rem))
      {
        return false; // Integer underflow / overflow.
      }
      number = (number * 10) + digit;
    }

    return sign ? number : -number;
  }

  number::real_type to_real(const std::u32string& input)
  {
    const auto length = input.length();
    number::real_type number;
    std::u32string::size_type offset;
    bool seen_digits = false;
    bool seen_dot = false;
    bool sign;
    number::int_type exponent = 0;

    if (!length)
    {
      return false;
    }

    if (!input.compare(string_nan))
    {
      return NAN;
    }
    else if (!input.compare(string_inf))
    {
      return INFINITY;
    }
    else if (!input.compare(string_inf_neg))
    {
      return -INFINITY;
    }

    if (input[0] == '+' || input[0] == '-')
    {
      sign = input[0] == '+';
      offset = 1;
    } else {
      sign = true;
      offset = 0;
    }

    number = 0.0;

    for (; offset < length; ++offset)
    {
      const auto c = input[offset];

      if (std::isdigit(c))
      {
        seen_digits = true;
        if (number > number::real_max * 0.1)
        {
          ++exponent;
        } else {
          number = (number * 10.0) + (c - '0');
        }
        if (seen_dot)
        {
          --exponent;
        }
      }
      else if (!seen_dot && c == '.')
      {
        seen_dot = true;
      } else {
        break;
      }
    }

    if (!seen_digits)
    {
      return 0.0;
    }

    // Parse exponent.
    if (offset < length && (input[offset] == 'e' || input[offset] == 'E'))
    {
      exponent += to_integer(input.substr(offset + 1));
    }

    if (number == 0.0)
    {
      return 0.0;
    }

    number *= std::pow(10.0, static_cast<number::real_type>(exponent));
    if (!sign)
    {
      number = -number;
    }

    return number;
  }
}

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
#include <plorth/unicode.hpp>

#include <cfloat>
#include <climits>
#include <cmath>

namespace plorth
{
  static const char digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  static const unistring unistring_nan = {'n', 'a', 'n'};
  static const unistring unistring_inf = {'i', 'n', 'f'};
  static const unistring unistring_inf_neg = {'-', 'i', 'n', 'f'};

  unistring json_stringify(const unistring& input)
  {
    unistring result;

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
          if (unichar_iscntrl(c))
          {
            char buffer[7];

            std::snprintf(buffer, 7, "\\u%04x", c);
            for (const char* p = buffer; *p; ++p)
            {
              result.append(1, static_cast<unichar>(*p));
            }
          } else {
            result.append(1, c);
          }
      }
    }

    result.append(1, '"');

    return result;
  }

  bool is_number(const unistring& input)
  {
    const auto length = input.length();
    unistring::size_type start;
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

  unistring to_unistring(std::int64_t number)
  {
    const bool negative = number < 0;
    std::uint64_t mag = static_cast<std::uint64_t>(negative ? -number : number);
    unistring result;

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

  unistring to_unistring(double number)
  {
    char buffer[20];

    if (std::isnan(number))
    {
      return unistring_nan;
    }
    else if (std::isinf(number))
    {
      return number < 0.0 ? unistring_inf_neg : unistring_inf;
    }
    std::snprintf(buffer, sizeof(buffer), "%g", number);

    return utf8_decode(buffer);
  }

  std::int64_t to_integer(const unistring& input)
  {
    static const std::int64_t div = INT64_MAX / 10;
    static const std::int64_t rem = INT64_MAX % 10;
    std::int64_t number = 0;
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
      const unichar c = input[offset];
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

  double to_real(const unistring& input)
  {
    const auto length = input.length();
    double number;
    unistring::size_type offset;
    bool seen_digits = false;
    bool seen_dot = false;
    bool sign;
    std::int64_t exponent = 0;

    if (!length)
    {
      return false;
    }

    if (!input.compare(unistring_nan))
    {
      return NAN;
    }
    else if (!input.compare(unistring_inf))
    {
      return INFINITY;
    }
    else if (!input.compare(unistring_inf_neg))
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
        if (number > DBL_MAX * 0.1)
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

    number *= std::pow(10.0, static_cast<double>(exponent));
    if (!sign)
    {
      number = -number;
    }

    return number;
  }
}

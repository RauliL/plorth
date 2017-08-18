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
        if (dot_seen || i == start || i + 1 > length)
        {
          return false;
        }
        dot_seen = true;
      }
      else if (!std::isdigit(c))
      {
        return false;
      }
    }

    return true;
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

  static bool parse_int(const unistring& input, int& number)
  {
    static const int div = INT_MAX / 10;
    static const int rem = INT_MAX % 10;
    const std::size_t length = input.length();
    std::size_t offset = 0;
    bool sign;

    // Extract the sign.
    sign = !(length > 0 && input[0] == '-');
    if (!sign || (length > 0 && input[0] == '+'))
    {
      ++offset;
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
    number = sign ? number : -number;

    return true;
  }

  bool to_number(const unistring& input, double& number)
  {
    const std::size_t length = input.length();
    std::size_t offset = 0;
    bool seen_digits = false;
    bool seen_dot = false;
    bool sign;
    int exponent = 0;

    if (!input.compare(unistring_nan))
    {
      number = NAN;

      return true;
    }
    else if (!input.compare(unistring_inf))
    {
      number = INFINITY;

      return true;
    }
    else if (!input.compare(unistring_inf_neg))
    {
      number = -INFINITY;

      return true;
    }

    // Extract the sign.
    sign = !(length > 0 && input[0] == '-');
    if (!sign || (length > 0 && input[0] == '+'))
    {
      ++offset;
    }

    number = 0.0;

    for (; offset < length; ++offset)
    {
      const unichar c = input[offset];

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
      number = 0.0;

      return true;
    }

    // Parse exponent.
    if (offset < length && (input[offset] == 'e' || input[offset] == 'E'))
    {
      if (!parse_int(input.substr(offset + 1), exponent))
      {
        return false;
      }
    }

    if (number == 0.0)
    {
      number = 0.0;

      return true;
    }

    if (exponent < 0)
    {
      if (number < DBL_MIN * std::pow(10.0, static_cast<double>(-exponent)))
      {
        return false; // Float underflow.
      }
    }
    else if (exponent > 0)
    {
      if (number > DBL_MAX * std::pow(10.0, static_cast<double>(exponent)))
      {
        return false; // Float overflow.
      }
    }

    number *= std::pow(10.0, static_cast<double>(exponent));
    number = number * sign;

    return true;
  }
}

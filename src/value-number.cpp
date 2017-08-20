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

#include "./utils.hpp"

#include <cmath>

namespace plorth
{
  namespace
  {
    class int_number : public number
    {
    public:
      explicit int_number(std::int64_t val)
        : m_value(val) {}

      enum number_type number_type() const
      {
        return number_type_int;
      }

      std::int64_t as_int() const
      {
        return m_value;
      }

      double as_real() const
      {
        return static_cast<double>(m_value);
      }

    private:
      const std::int64_t m_value;
    };

    class real_number : public number
    {
    public:
      explicit real_number(double val)
        : m_value(val) {}

      enum number_type number_type() const
      {
        return number_type_real;
      }

      std::int64_t as_int() const
      {
        double value = m_value;

        if (value > 0.0)
        {
          value = std::floor(value);
        }
        if (value < 0.0)
        {
          value = std::ceil(value);
        }

        return static_cast<std::int64_t>(value);
      }

      double as_real() const
      {
        return m_value;
      }

    private:
      const double m_value;
    };
  }

  bool number::equals(const ref<class value>& that) const
  {
    ref<number> num;

    if (!that || !that->is(type_number))
    {
      return false;
    }
    num = that.cast<number>();
    if (is(number_type_real) || num->is(number_type_real))
    {
      return as_real() == num->as_real();
    } else {
      return as_int() == num->as_int();
    }
  }

  unistring number::to_string() const
  {
    if (is(number_type_real))
    {
      return to_unistring(as_real());
    } else {
      return to_unistring(as_int());
    }
  }

  unistring number::to_source() const
  {
    return to_string();
  }

  ref<class number> runtime::number(std::int64_t value)
  {
#if PLORTH_ENABLE_INTEGER_CACHE
    static const int offset = 128;

    if (value >= -128 && value <= 127)
    {
      const int index = value + offset;
      auto reference = m_integer_cache[index];

      if (!reference)
      {
        reference = new (*m_memory_manager) int_number(value);
        m_integer_cache[index] = reference;
      }

      return reference;
    }
#endif

    return new (*m_memory_manager) int_number(value);
  }

  ref<class number> runtime::number(double value)
  {
    return new (*m_memory_manager) real_number(value);
  }

  ref<class number> runtime::number(const unistring& value)
  {
    const auto dot_index = value.find('.');

    if (dot_index == unistring::npos)
    {
      return number(to_integer(value));
    } else {
      return number(to_real(value));
    }
  }

  /**
   * Word: nan?
   * Prototype: number
   *
   * Takes:
   * - number
   *
   * Gives:
   * - number
   * - boolean
   *
   * Returns true if given number if NaN.
   */
  static void w_is_nan(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      ctx->push(num);
      if (num->is(number::number_type_real))
      {
        ctx->push_boolean(std::isnan(num->as_real()));
      } else {
        ctx->push_boolean(false);
      }
    }
  }

  /**
   * Word: finite?
   * Prototype: number
   *
   * Takes:
   * - num
   *
   * Gives:
   * - number
   * - boolean
   *
   * Returns true if given number is finite.
   */
  static void w_is_finite(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      ctx->push(num);
      if (num->is(number::number_type_real))
      {
        ctx->push_boolean(std::isfinite(num->as_real()));
      } else {
        ctx->push_boolean(true);
      }
    }
  }

  /**
   * Word: times
   * Prototype: number
   *
   * Takes:
   * - quote
   * - number
   *
   * Executes quote given number of times.
   */
  static void w_times(const ref<context>& ctx)
  {
    ref<number> num;
    ref<quote> quo;

    if (ctx->pop_number(num) && ctx->pop_quote(quo))
    {
      std::int64_t count = num->as_int();

      if (count < 0)
      {
        count = -count;
      }
      while (count > 0)
      {
        --count;
        if (!quo->call(ctx))
        {
          return;
        }
      }
    }
  }

  /**
   * Word: abs
   * Prototype: number
   *
   * Takes:
   * - number
   *
   * Gives:
   * - number
   *
   * Returns absolute value of the number.
   */
  static void w_abs(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type_real))
      {
        ctx->push_real(std::fabs(num->as_real()));
      } else {
        ctx->push_int(std::abs(num->as_int()));
      }
    }
  }

  /**
   * Word: round
   * Prototype: number
   *
   * Takes:
   * - number
   *
   * Gives:
   * - number
   *
   * Rounds number to nearest integer value.
   */
  static void w_round(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type_real))
      {
        ctx->push_int(std::round(num->as_real()));
      } else {
        ctx->push(num);
      }
    }
  }

  /**
   * Word: ceil
   * Prototype: number
   *
   * Takes:
   * - number
   *
   * Gives:
   * - number
   *
   * Computes the smallest integer value not less than given number.
   */
  static void w_ceil(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type_real))
      {
        ctx->push_int(std::ceil(num->as_real()));
      } else {
        ctx->push(num);
      }
    }
  }

  /**
   * Word: floor
   * Prototype: number
   *
   * Takes:
   * - number
   *
   * Gives:
   * - number
   *
   * Computes the largest integer value not greater than given number.
   */
  static void w_floor(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type_real))
      {
        ctx->push_int(std::floor(num->as_real()));
      } else {
        ctx->push(num);
      }
    }
  }

  /**
   * Word: max
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Returns maximum of two numbers.
   */
  static void w_max(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push(a->as_real() > b->as_real() ? a : b);
      } else {
        ctx->push(a->as_int() > b->as_int() ? a : b);
      }
    }
  }

  /**
   * Word: min
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Returns minimum of two numbers.
   */
  static void w_min(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push(a->as_real() < b->as_real() ? a : b);
      } else {
        ctx->push(a->as_int() < b->as_int() ? a : b);
      }
    }
  }

  /**
   * Word: clamp
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Clamps top-most number between the minimum and maximum limits.
   */
  static void w_clamp(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;
    ref<number> c;

    if (ctx->pop_number(c) && ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real)
          || b->is(number::number_type_real)
          || c->is(number::number_type_real))
      {
        const double min = a->as_real();
        const double max = b->as_real();
        double number = c->as_real();

        if (number > max)
        {
          number = max;
        }
        if (number < min)
        {
          number = min;
        }
        ctx->push_real(number);
      } else {
        const std::int64_t min = a->as_int();
        const std::int64_t max = b->as_int();
        std::int64_t number = c->as_int();

        if (number > max)
        {
          number = max;
        }
        if (number < min)
        {
          number = min;
        }
        ctx->push_int(number);
      }
    }
  }

  /**
   * Word: in-range?
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   * - number
   *
   * Gives:
   * - boolean
   *
   * Tests whether the top-most number is in range of given minimum and maximum
   * numbers.
   */
  static void w_is_in_range(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;
    ref<number> c;

    if (ctx->pop_number(c) && ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real)
          || b->is(number::number_type_real)
          || c->is(number::number_type_real))
      {
        const double min = a->as_real();
        const double max = b->as_real();
        const double number = c->as_real();

        ctx->push_boolean(number >= min && number <= max);
      } else {
        const std::int64_t min = a->as_int();
        const std::int64_t max = b->as_int();
        const std::int64_t number = c->as_int();

        ctx->push_boolean(number >= min && number <= max);
      }
    }
  }

  /**
   * Word: +
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs addition on the two given numbers.
   */
  static void w_add(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_real(a->as_real() + b->as_real());
      } else {
        ctx->push_int(a->as_int() + b->as_int());
      }
    }
  }

  /**
   * Word: -
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs substraction on the two given numbers.
   */
  static void w_sub(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_real(a->as_real() - b->as_real());
      } else {
        ctx->push_int(a->as_int() - b->as_int());
      }
    }
  }

  /**
   * Word: *
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs multiplication on the two given numbers.
   */
  static void w_mul(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_real(a->as_real() * b->as_real());
      } else {
        ctx->push_int(a->as_int() * b->as_int());
      }
    }
  }

  /**
   * Word: /
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs division on the two given numbers.
   */
  static void w_div(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_real(a->as_real() / b->as_real());
      } else {
        const std::int64_t x = a->as_int();
        const std::int64_t y = a->as_int();

        if (y == 0)
        {
          ctx->push_real(y < 0 ? -INFINITY : INFINITY);
        } else {
          ctx->push_int(x / y);
        }
      }
    }
  }

  /**
   * Word: %
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Computes the floating-point remainder of the division operation between the
   * two given numbers.
   */
  static void w_mod(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_real(std::fmod(a->as_real(), b->as_real()));
      } else {
        const std::int64_t x = a->as_int();
        const std::int64_t y = a->as_int();

        if (y == 0)
        {
          ctx->push_real(NAN);
        } else {
          ctx->push_int(x % y);
        }
      }
    }
  }

  /**
   * Word: <
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - boolean
   *
   * Returns true if first number is lesser than the second one.
   */
  static void w_lt(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_boolean(a->as_real() < b->as_real());
      } else {
        ctx->push_boolean(a->as_int() < b->as_int());
      }
    }
  }

  /**
   * Word: >
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - boolean
   *
   * Returns true if first number is greater than the second one.
   */
  static void w_gt(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_boolean(a->as_real() > b->as_real());
      } else {
        ctx->push_boolean(a->as_int() > b->as_int());
      }
    }
  }

  /**
   * Word: <=
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - boolean
   *
   * Returns true if first number is less or equal than the second one.
   */
  static void w_lte(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_boolean(a->as_real() <= b->as_real());
      } else {
        ctx->push_boolean(a->as_int() <= b->as_int());
      }
    }
  }

  /**
   * Word: >=
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - boolean
   *
   * Returns true if first number is greater or equal than the second one.
   */
  static void w_gte(const ref<context>& ctx)
  {
    ref<number> a;
    ref<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type_real) || b->is(number::number_type_real))
      {
        ctx->push_boolean(a->as_real() >= b->as_real());
      } else {
        ctx->push_boolean(a->as_int() >= b->as_int());
      }
    }
  }

  namespace api
  {
    runtime::prototype_definition number_prototype()
    {
      return
      {
        { "nan?", w_is_nan },
        { "finite?", w_is_finite },

        { "times", w_times },

        { "abs", w_abs },
        { "round", w_round },
        { "floor", w_floor },
        { "ceil", w_ceil },
        { "max", w_max },
        { "min", w_min },
        { "clamp", w_clamp },
        { "in-range?", w_is_in_range },

        { "+", w_add },
        { "-", w_sub },
        { "*", w_mul },
        { "/", w_div },
        { "%", w_mod },

        { "<", w_lt },
        { ">", w_gt },
        { "<=", w_lte },
        { ">=", w_gte }
      };
    }
  }
}

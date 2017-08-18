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
#include <plorth/value-number.hpp>

#include "./utils.hpp"

#include <cmath>

namespace plorth
{
  number::number(double value)
    : m_value(value) {}

  bool number::equals(const ref<class value>& that) const
  {
    if (!that || !that->is(type_number))
    {
      return false;
    }

    return m_value == that.cast<number>()->m_value;
  }

  unistring number::to_string() const
  {
    return to_unistring(m_value);
  }

  unistring number::to_source() const
  {
    return to_string();
  }

  /**
   * nan? ( number -- number boolean )
   *
   * Returns true if given number if NaN.
   */
  static void w_is_nan(const ref<context>& ctx)
  {
    ref<value> num;

    if (ctx->pop(num, value::type_number))
    {
      ctx->push(num);
      ctx->push_boolean(std::isnan(num.cast<number>()->value()));
    }
  }

  /**
   * finite? ( num -- number boolean )
   *
   * Returns true if given number is finite.
   */
  static void w_is_finite(const ref<context>& ctx)
  {
    ref<value> num;

    if (ctx->pop(num, value::type_number))
    {
      ctx->push(num);
      ctx->push_boolean(std::isfinite(num.cast<number>()->value()));
    }
  }

  /**
   * times ( quote number -- )
   *
   * Executes quote given number of times.
   */
  static void w_times(const ref<context>& ctx)
  {
    double count;
    ref<class quote> quote;

    if (!ctx->pop_number(count) || !ctx->pop_quote(quote))
    {
      return;
    }

    if (count < 0.0)
    {
      count = -count;
    }

    while (count >= 1.0)
    {
      count -= 1.0;
      if (!quote->call(ctx))
      {
        return;
      }
    }
  }

  /**
   * abs ( number -- number )
   *
   * Returns absolute value of the number.
   */
  static void w_abs(const ref<context>& ctx)
  {
    double num;

    if (ctx->pop_number(num))
    {
      ctx->push_number(std::abs(num));
    }
  }

  /**
   * round ( number -- number )
   *
   * Rounds number to nearest integer value.
   */
  static void w_round(const ref<context>& ctx)
  {
    double num;

    if (ctx->pop_number(num))
    {
      ctx->push_number(std::round(num));
    }
  }

  /**
   * ceil ( number -- number )
   *
   * Computes the smallest integer value not less than given number.
   */
  static void w_ceil(const ref<context>& ctx)
  {
    double num;

    if (ctx->pop_number(num))
    {
      ctx->push_number(std::ceil(num));
    }
  }

  /**
   * floor ( number -- number )
   *
   * Computes the largest integer value not greater than given number.
   */
  static void w_floor(const ref<context>& ctx)
  {
    double num;

    if (ctx->pop_number(num))
    {
      ctx->push_number(std::floor(num));
    }
  }

  /**
   * max ( number number -- number )
   *
   * Returns maximum of two numbers.
   */
  static void w_max(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(b > a ? b : a);
    }
  }

  /**
   * min ( number number -- number )
   *
   * Returns minimum of two numbers.
   */
  static void w_min(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(b < a ? b : a);
    }
  }

  /**
   * clamp ( number number number -- number )
   *
   * Clamps top-most number between the minimum and maximum limits.
   */
  static void w_clamp(const ref<context>& ctx)
  {
    double num;
    double max;
    double min;

    if (ctx->pop_number(num) && ctx->pop_number(max) && ctx->pop_number(min))
    {
      if (num > max)
      {
        num = max;
      }
      if (num < min)
      {
        num = min;
      }
      ctx->push_number(num);
    }
  }

  /**
   * in-range? ( number number number -- boolean )
   *
   * Tests whether the top-most number is in range of given minimum and maximum
   * numbers.
   */
  static void w_is_in_range(const ref<context>& ctx)
  {
    double num;
    double max;
    double min;

    if (ctx->pop_number(num) && ctx->pop_number(max) && ctx->pop_number(min))
    {
      ctx->push_boolean(num >= min && num <= max);
    }
  }

  /**
   * + ( number number -- number )
   *
   * Performs addition on the two given numbers.
   */
  static void w_add(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(b + a);
    }
  }

  /**
   * - ( number number -- number )
   *
   * Performs substraction on the two given numbers.
   */
  static void w_sub(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(b - a);
    }
  }

  /**
   * * ( number number -- number )
   *
   * Performs multiplication on the two given numbers.
   */
  static void w_mul(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(b * a);
    }
  }

  /**
   * / ( number number -- number )
   *
   * Performs division on the two given numbers.
   */
  static void w_div(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(b / a);
    }
  }

  /**
   * % ( number number -- number )
   *
   * Computes the floating-point remainder of the division operation between the
   * two given numbers.
   */
  static void w_mod(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_number(std::fmod(b, a));
    }
  }

  /**
   * < ( number number -- boolean )
   *
   * Returns true if first number is lesser than the second one.
   */
  static void w_lt(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_boolean(b < a);
    }
  }

  /**
   * > ( number number -- boolean )
   *
   * Returns true if first number is greater than the second one.
   */
  static void w_gt(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_boolean(b > a);
    }
  }

  /**
   * <= ( number number -- boolean )
   *
   * Returns true if first number is less or equal than the second one.
   */
  static void w_lte(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_boolean(b <= a);
    }
  }

  /**
   * >= ( number number -- boolean )
   *
   * Returns true if first number is greater or equal than the second one.
   */
  static void w_gte(const ref<context>& ctx)
  {
    double a;
    double b;

    if (ctx->pop_number(a) && ctx->pop_number(b))
    {
      ctx->push_boolean(b >= a);
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

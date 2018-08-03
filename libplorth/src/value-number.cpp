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

#include "./utils.hpp"

#include <cfloat>
#include <cmath>
#include <climits>

namespace plorth
{
#if PLORTH_ENABLE_32BIT_INT
  const number::int_type number::int_min = INT32_MIN;
  const number::int_type number::int_max = INT32_MAX;
#else
  const number::int_type number::int_min = INT64_MIN;
  const number::int_type number::int_max = INT64_MAX;
#endif
  const number::real_type number::real_min = DBL_MIN;
  const number::real_type number::real_max = DBL_MAX;

  namespace
  {
    class int_number : public number
    {
    public:
      explicit int_number(int_type val)
        : m_value(val) {}

      enum number_type number_type() const
      {
        return number_type::integer;
      }

      int_type as_int() const
      {
        return m_value;
      }

      real_type as_real() const
      {
        return static_cast<number::real_type>(m_value);
      }

    private:
      const int_type m_value;
    };

    class real_number : public number
    {
    public:
      explicit real_number(number::real_type val)
        : m_value(val) {}

      enum number_type number_type() const
      {
        return number_type::real;
      }

      int_type as_int() const
      {
        number::real_type value = m_value;

        if (value > 0.0)
        {
          value = std::floor(value);
        }
        if (value < 0.0)
        {
          value = std::ceil(value);
        }

        return static_cast<int_type>(value);
      }

      real_type as_real() const
      {
        return m_value;
      }

    private:
      const real_type m_value;
    };
  }

  bool number::equals(const std::shared_ptr<class value>& that) const
  {
    std::shared_ptr<number> num;

    if (!value::is(that, type::number))
    {
      return false;
    }
    num = std::static_pointer_cast<number>(that);
    if (is(number_type::real) || num->is(number_type::real))
    {
      return as_real() == num->as_real();
    } else {
      return as_int() == num->as_int();
    }
  }

  std::u32string number::to_string() const
  {
    if (is(number_type::real))
    {
      return to_unistring(as_real());
    } else {
      return to_unistring(as_int());
    }
  }

  std::u32string number::to_source() const
  {
    return to_string();
  }

  std::shared_ptr<number> runtime::number(number::int_type value)
  {
#if PLORTH_ENABLE_INTEGER_CACHE
    static const int offset = 128;

    if (value >= -128 && value <= 127)
    {
      const int index = value + offset;
      auto reference = m_integer_cache[index];

      if (!reference)
      {
        reference = std::shared_ptr<class number>(
          new (*m_memory_manager) int_number(value)
        );
        m_integer_cache[index] = reference;
      }

      return reference;
    }
#endif

    return std::shared_ptr<class number>(
      new (*m_memory_manager) int_number(value)
    );
  }

  std::shared_ptr<number> runtime::number(number::real_type value)
  {
    return std::shared_ptr<class number>(
      new (*m_memory_manager) real_number(value)
    );
  }

  std::shared_ptr<class number> runtime::number(const std::u32string& value)
  {
    const auto dot_index = value.find('.');
    const auto exponent_index_lower_case = value.find('e');
    const auto exponent_index_upper_case = value.find('E');

    if (
      dot_index == std::u32string::npos &&
      exponent_index_lower_case == std::u32string::npos &&
      exponent_index_upper_case == std::u32string::npos
    )
    {
      const number::int_type result = to_integer(value);

      if (result == false)
      {
        const number::real_type big_result = to_real(value);

        if (big_result != result)
        {
          return number(big_result);
        }
      }

      return number(result);
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
   * Returns true if given number is NaN.
   */
  static void w_is_nan(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      ctx->push(num);
      if (num->is(number::number_type::real))
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
  static void w_is_finite(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      ctx->push(num);
      if (num->is(number::number_type::real))
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
   * Executes a quote given number of times.
   */
  static void w_times(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;
    std::shared_ptr<quote> quo;

    if (ctx->pop_number(num) && ctx->pop_quote(quo))
    {
      auto count = num->as_int();

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
  static void w_abs(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type::real))
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
   * Rounds given number to nearest integer value.
   */
  static void w_round(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type::real))
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
  static void w_ceil(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type::real))
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
  static void w_floor(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      if (num->is(number::number_type::real))
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
  static void w_max(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real) || b->is(number::number_type::real))
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
  static void w_min(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real) || b->is(number::number_type::real))
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
   * Clamps the topmost number between the minimum and maximum limits.
   */
  static void w_clamp(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;
    std::shared_ptr<number> c;

    if (ctx->pop_number(c) && ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real)
          || b->is(number::number_type::real)
          || c->is(number::number_type::real))
      {
        const number::real_type min = a->as_real();
        const number::real_type max = b->as_real();
        number::real_type number = c->as_real();

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
        const auto min = a->as_int();
        const auto max = b->as_int();
        auto number = c->as_int();

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
   * Tests whether the topmost number is in range of given minimum and maximum
   * numbers.
   */
  static void w_is_in_range(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;
    std::shared_ptr<number> c;

    if (ctx->pop_number(c) && ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real)
          || b->is(number::number_type::real)
          || c->is(number::number_type::real))
      {
        const number::real_type min = a->as_real();
        const number::real_type max = b->as_real();
        const number::real_type number = c->as_real();

        ctx->push_boolean(number >= min && number <= max);
      } else {
        const number::int_type min = a->as_int();
        const number::int_type max = b->as_int();
        const number::int_type number = c->as_int();

        ctx->push_boolean(number >= min && number <= max);
      }
    }
  }

  template<class RealOperation, class IntOperation>
  static void number_op(
    const std::shared_ptr<context>& ctx,
    const RealOperation& real_op,
    const IntOperation& int_op
  )
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;
    number::real_type result;

    if (!ctx->pop_number(b) || !ctx->pop_number(a))
    {
      return;
    }

    result = real_op(a->as_real(), b->as_real());

    if (a->is(number::number_type::integer) &&
        b->is(number::number_type::integer) &&
        std::fabs(result) <= number::int_max)
    {
      // Repeat the operation with full integer precision
      ctx->push_int(int_op(a->as_int(), b->as_int()));
      return;
    }

    // Otherwise keep it real as it seems to be integer overflow or either of
    // the arguments are real numbers.
    ctx->push_real(result);
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
  static void w_add(const std::shared_ptr<context>& ctx)
  {
    number_op(ctx, std::plus<number::real_type>(), std::plus<number::int_type>());
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
   * Subtracts the second number from the first and returns the result.
   */
  static void w_sub(const std::shared_ptr<context>& ctx)
  {
    number_op(ctx, std::minus<number::real_type>(), std::minus<number::int_type>());
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
  static void w_mul(const std::shared_ptr<context>& ctx)
  {
    number_op(ctx, std::multiplies<number::real_type>(), std::multiplies<number::int_type>());
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
   * Divides the first number by the second and returns the result.
   */
  static void w_div(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      ctx->push_real(a->as_real() / b->as_real());
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
   * Computes the modulo of the first number with respect to the second number
   * i.e. the remainder after floor division.
   */
  static void w_mod(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;
    number::real_type dividend;
    number::real_type divider;
    number::real_type result;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      dividend = a->as_real();
      divider = b->as_real();
      result = std::fmod(dividend, divider);
      if (std::signbit(dividend) != std::signbit(divider)) {
         result += divider;
      }
      ctx->push_real(result);
    }
  }

  template<typename Operation >
  static void number_bit_op(const std::shared_ptr<context>& ctx,
                            const Operation& op)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      ctx->push_int(op(a->as_int(), b->as_int()));
    }
  }

  /**
   * Word: &
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs bitwise and on the two given numbers.
   */
  static void w_bit_and(const std::shared_ptr<context>& ctx)
  {
    number_bit_op(ctx, std::bit_and<number::int_type>());
  }

  /**
   * Word: |
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs bitwise or on the two given numbers.
   */
  static void w_bit_or(const std::shared_ptr<context>& ctx)
  {
    number_bit_op(ctx, std::bit_or<number::int_type>());
  }

  /**
   * Word: ^
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Performs bitwise xor on the two given numbers.
   */
  static void w_bit_xor(const std::shared_ptr<context>& ctx)
  {
    number_bit_op(ctx, std::bit_xor<number::int_type>());
  }

  /**
   * Word: >>
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Returns the first value with bits shifted right by the second value.
   */
  static void w_shift_right(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      ctx->push_int(a->as_int() >> b->as_int());
    }
  }

  /**
   * Word: <<
   * Prototype: number
   *
   * Takes:
   * - number
   * - number
   *
   * Gives:
   * - number
   *
   * Returns the first value with bits shifted left by the second value.
   */
  static void w_shift_left(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      ctx->push_int(a->as_int() << b->as_int());
    }
  }

  /**
   * Word: ~
   * Prototype: number
   *
   * Takes:
   * - number
   *
   * Gives:
   * - number
   *
   * Flips the bits of the value.
   */
  static void w_bit_not(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;

    if (ctx->pop_number(a))
    {
      ctx->push_int(~a->as_int());
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
   * Returns true if the first number is less than the second one.
   */
  static void w_lt(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real) || b->is(number::number_type::real))
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
   * Returns true if the first number is greater than the second one.
   */
  static void w_gt(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real) || b->is(number::number_type::real))
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
   * Returns true if the first number is less than or equal to the second one.
   */
  static void w_lte(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real) || b->is(number::number_type::real))
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
   * Returns true if the first number is greater than or equal to the second
   * one.
   */
  static void w_gte(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> a;
    std::shared_ptr<number> b;

    if (ctx->pop_number(b) && ctx->pop_number(a))
    {
      if (a->is(number::number_type::real) || b->is(number::number_type::real))
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
        { U"nan?", w_is_nan },
        { U"finite?", w_is_finite },

        { U"times", w_times },

        { U"abs", w_abs },
        { U"round", w_round },
        { U"floor", w_floor },
        { U"ceil", w_ceil },
        { U"max", w_max },
        { U"min", w_min },
        { U"clamp", w_clamp },
        { U"in-range?", w_is_in_range },

        { U"+", w_add },
        { U"-", w_sub },
        { U"*", w_mul },
        { U"/", w_div },
        { U"%", w_mod },

        { U"&", w_bit_and },
        { U"|", w_bit_or },
        { U"^", w_bit_xor },
        { U"<<", w_shift_left },
        { U">>", w_shift_right },
        { U"~", w_bit_not },

        { U"<", w_lt },
        { U">", w_gt },
        { U"<=", w_lte },
        { U">=", w_gte }
      };
    }
  }
}

#include <cmath>

#include "context.hpp"

#define FITS_TO_INT64(x) ((x) <= INT64_MAX && (x) >= INT64_MIN)

namespace plorth
{
  /**
   * nan? ( num -- bool )
   *
   * Returns true if given number if NaN.
   */
  static void w_is_nan(const Ref<Context>& context)
  {
    Ref<Number> number;

    if (context->PopNumber(number))
    {
      context->PushBool(
        number->GetNumberType() == Number::NUMBER_TYPE_FLOAT
        && std::isnan(number.As<FloatNumber>()->GetValue())
      );
    }
  }

  /**
   * finite? ( num -- bool )
   *
   * Returns true if given number is finite.
   */
  static void w_is_finite(const Ref<Context>& context)
  {
    Ref<Number> number;

    if (context->PopNumber(number))
    {
      if (number->GetNumberType() == Number::NUMBER_TYPE_FLOAT)
      {
        context->PushBool(!std::isinf(number.As<FloatNumber>()->GetValue()));
      } else {
        context->PushBool(true);
      }
    }
  }

  /**
   * inf? ( num -- bool )
   *
   * Returns true if given number is infinity.
   */
  static void w_is_inf(const Ref<Context>& context)
  {
    Ref<Number> number;

    if (context->PopNumber(number))
    {
      context->PushBool(
        number->GetNumberType() == Number::NUMBER_TYPE_FLOAT
        && std::isinf(number.As<FloatNumber>()->GetValue())
      );
    }
  }

  /**
   * even? ( num -- bool )
   *
   * Tests whether given number is even.
   */
  static void w_is_even(const Ref<Context>& context)
  {
    Ref<Number> value;

    if (!context->PopNumber(value))
    {
      return;
    }
    else if (value->GetNumberType() == Number::NUMBER_TYPE_BIG_INT)
    {
      context->PushBool((value->AsBigInt() & 1) == 0);
    } else {
      context->PushBool((value->AsInt() & 1) == 0);
    }
  }

  /**
   * odd? ( num -- bool )
   *
   * Tests whether given number is odd.
   */
  static void w_is_odd(const Ref<Context>& context)
  {
    Ref<Number> value;

    if (!context->PopNumber(value))
    {
      return;
    }
    else if (value->GetNumberType() == Number::NUMBER_TYPE_BIG_INT)
    {
      context->PushBool((value->AsBigInt() & 1) != 0);
    } else {
      context->PushBool((value->AsInt() & 1) != 0);
    }
  }

  /**
   * + ( num num -- num )
   *
   * Performs addition on the two given numbers.
   */
  static void w_add(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      // TODO: Limit testing.
      context->PushNumber(x + y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushNumber(x + y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      if (FITS_TO_INT64(x + y))
      {
        context->PushNumber(x + y);
      } else {
        context->PushNumber(mpz_class(x) + mpz_class(y));
      }
    }
  }

  /**
   * - ( num num -- num )
   *
   * Performs substraction on the two given numbers.
   */
  static void w_sub(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      // TODO: Limit testing.
      context->PushNumber(x - y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushNumber(x - y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      if (FITS_TO_INT64(x - y))
      {
        context->PushNumber(x - y);
      } else {
        context->PushNumber(mpz_class(x) - mpz_class(y));
      }
    }
  }

  /**
   * * ( num num -- num )
   *
   * Performs multiplication on the two given numbers.
   */
  static void w_mul(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      context->PushNumber(x * y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushNumber(x * y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      if (FITS_TO_INT64(x * y))
      {
        context->PushNumber(x * y);
      } else {
        context->PushNumber(mpz_class(x) * mpz_class(y));
      }
    }
  }

  /**
   * / ( num num -- num )
   *
   * Performs division on the two given numbers.
   */
  static void w_div(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      if (y == 0.0)
      {
        context->PushNumber(std::numeric_limits<double>::infinity());
      } else {
        context->PushNumber(x / y);
      }
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      if (y == mpz_class(0))
      {
        context->PushNumber(std::numeric_limits<double>::infinity());
      } else {
        context->PushNumber(x / y);
      }
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      if (y == 0)
      {
        context->PushNumber(std::numeric_limits<double>::infinity());
      } else {
        context->PushNumber(x / y);
      }
    }
  }

  /**
   * = ( num num -- bool )
   *
   * Tests whether two numbers are equal.
   */
  static void w_eq(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      context->PushBool(x == y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushBool(x == y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      context->PushBool(x == y);
    }
  }

  static void w_lt(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      context->PushBool(x < y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushBool(x < y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      context->PushBool(x < y);
    }
  }

  static void w_gt(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      context->PushBool(x > y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushBool(x > y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      context->PushBool(x > y);
    }
  }

  static void w_lte(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      context->PushBool(x <= y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushBool(x <= y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      context->PushBool(x <= y);
    }
  }

  static void w_gte(const Ref<Context>& context)
  {
    Ref<Number> a;
    Ref<Number> b;
    Number::NumberType type_a;
    Number::NumberType type_b;

    if (!context->PopNumber(a) || !context->PopNumber(b))
    {
      return;
    }
    type_a = a->GetNumberType();
    type_b = b->GetNumberType();
    if (type_a == Number::NUMBER_TYPE_FLOAT
        || type_b == Number::NUMBER_TYPE_FLOAT)
    {
      const double x = b->AsFloat();
      const double y = a->AsFloat();

      context->PushBool(x >= y);
    }
    else if (type_a == Number::NUMBER_TYPE_BIG_INT
            || type_b == Number::NUMBER_TYPE_BIG_INT)
    {
      const mpz_class x = b->AsBigInt();
      const mpz_class y = a->AsBigInt();

      context->PushBool(x >= y);
    } else {
      const std::int64_t x = b->AsInt();
      const std::int64_t y = a->AsInt();

      context->PushBool(x >= y);
    }
  }

  /**
   * times ( quote num -- )
   *
   * Executes quote given number of times.
   */
  static void w_times(const Ref<Context>& context)
  {
    Ref<Number> number;
    Ref<Quote> quote;
    mpz_class times;

    if (!context->PopNumber(number) || !context->PopQuote(quote))
    {
      return;
    }
    if (number->GetNumberType() == Number::NUMBER_TYPE_INT)
    {
      std::int64_t times = number.As<IntNumber>()->GetValue();

      if (times < 0)
      {
        times = -times;
      }
      while (times-- > 0)
      {
        if (!quote->Call(context))
        {
          return;
        }
      }
    } else {
      mpz_class times = number->AsBigInt();

      if (times < 0)
      {
        times = -times;
      }
      while (times-- > 0)
      {
        if (!quote->Call(context))
        {
          return;
        }
      }
    }
  }

  Ref<Object> make_number_prototype(Runtime* runtime)
  {
    return runtime->NewPrototype({
      { "nan?", w_is_nan },
      { "finite?", w_is_finite },
      { "inf?", w_is_inf },
      { "even?", w_is_even },
      { "odd?", w_is_odd },

      { "+", w_add },
      { "-", w_sub },
      { "*", w_mul },
      { "/", w_div },

      { "=", w_eq },
      { "<", w_lt },
      { ">", w_gt },
      { "<=", w_lte },
      { ">=", w_gte },

      { "times", w_times },
    });
  }
}

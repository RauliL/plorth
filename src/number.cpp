#include <cmath>
#include <limits>

#include "context.hpp"

namespace plorth
{
  Ref<Object> Number::GetPrototype(const Ref<Runtime>& runtime) const
  {
    Ref<Value> value;

    if (runtime->FindWord("num", value) && value->GetType() == TYPE_OBJECT)
    {
      return value.As<Object>();
    } else {
      return Ref<Object>();
    }
  }

  IntNumber::IntNumber(std::int64_t value)
    : m_value(value) {}

  double IntNumber::AsFloat() const
  {
    return static_cast<double>(m_value);
  }

  mpz_class IntNumber::AsBigInt() const
  {
    return mpz_class(m_value);
  }

  bool IntNumber::Equals(const Ref<Value>& that) const
  {
    if (that->GetType() != TYPE_NUMBER)
    {
      return false;
    }
    switch (that.As<Number>()->GetNumberType())
    {
      case NUMBER_TYPE_INT:
        return m_value == that.As<IntNumber>()->m_value;

      case NUMBER_TYPE_FLOAT:
        return static_cast<double>(m_value) == that.As<FloatNumber>()->GetValue();

      case NUMBER_TYPE_BIG_INT:
        return that.As<BigIntNumber>()->GetValue() == m_value;
    }

    return false;
  }

  std::string IntNumber::ToString() const
  {
    return std::to_string(m_value);
  }

  FloatNumber::FloatNumber(double value)
    : m_value(value) {}

  std::int64_t FloatNumber::AsInt() const
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

  mpz_class FloatNumber::AsBigInt() const
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

    return mpz_class(value);
  }

  bool FloatNumber::Equals(const Ref<Value>& that) const
  {
    if (that->GetType() != TYPE_NUMBER)
    {
      return false;
    }
    switch (that.As<Number>()->GetNumberType())
    {
      case NUMBER_TYPE_FLOAT:
        return that.As<FloatNumber>()->m_value == m_value;

      case NUMBER_TYPE_INT:
        return static_cast<double>(that.As<IntNumber>()->GetValue()) == m_value;

      case NUMBER_TYPE_BIG_INT:
        return that.As<BigIntNumber>()->GetValue() == m_value;
    }

    return false;
  }

  std::string FloatNumber::ToString() const
  {
    if (std::isinf(m_value))
    {
      return m_value < 0.0 ? "-inf" : "inf";
    }
    else if (std::isnan(m_value))
    {
      return "nan";
    } else {
      char buffer[256];

      std::snprintf(buffer, 256, "%.17g", m_value);

      return buffer;
    }
  }

  BigIntNumber::BigIntNumber(const mpz_class& value)
    : m_value(value) {}

  std::int64_t BigIntNumber::AsInt() const
  {
    return m_value.get_si();
  }

  double BigIntNumber::AsFloat() const
  {
    return m_value.get_d();
  }

  bool BigIntNumber::Equals(const Ref<Value>& that) const
  {
    if (that->GetType() != TYPE_NUMBER)
    {
      return false;
    }
    switch (that.As<Number>()->GetNumberType())
    {
      case NUMBER_TYPE_BIG_INT:
        return m_value == that.As<BigIntNumber>()->m_value;

      case NUMBER_TYPE_FLOAT:
        return m_value == that.As<FloatNumber>()->GetValue();

      case NUMBER_TYPE_INT:
        return m_value == that.As<IntNumber>()->GetValue();
    }

    return false;
  }

  std::string BigIntNumber::ToString() const
  {
    return m_value.get_str(10);
  }

  /**
   * num? ( any -- any bool )
   *
   * Returns true if given value is number.
   */
  static void w_is_num(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_NUMBER);
    }
  }

  /**
   * e ( -- num )
   *
   * Returns Eulers number.
   */
  static void w_e(const Ref<Context>& context)
  {
    context->PushNumber(M_E);
  }

  /**
   * pi ( -- num )
   *
   * Returns value of PI.
   */
  static void w_pi(const Ref<Context>& context)
  {
    context->PushNumber(M_PI);
  }

  void api_init_number(Runtime* runtime)
  {
    runtime->AddWord("num?", w_is_num);

    // Constants.
    runtime->AddWord("e", w_e);
    runtime->AddWord("pi", w_pi);
  }
}

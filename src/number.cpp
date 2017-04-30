#include <cmath>

#include <plorth/plorth-runtime.hpp>

namespace plorth
{
  namespace
  {
    class IntNumber : public Number
    {
    public:
      explicit IntNumber(std::int64_t value)
        : m_value(value) {}

      inline NumberType GetNumberType() const
      {
        return NUMBER_TYPE_INT;
      }

      inline std::int64_t GetValue() const
      {
        return m_value;
      }

      inline std::int64_t AsInt() const
      {
        return m_value;
      }

      double AsFloat() const
      {
        return static_cast<double>(m_value);
      }

      mpz_class AsBigInt() const
      {
        return mpz_class(m_value);
      }

      bool Equals(const Ref<Value>& that) const;

      std::string ToString() const
      {
        return std::to_string(m_value);
      }

    private:
      const std::int64_t m_value;
    };

    class FloatNumber : public Number
    {
    public:
      explicit FloatNumber(double value)
        : m_value(value) {}

      inline NumberType GetNumberType() const
      {
        return NUMBER_TYPE_FLOAT;
      }

      inline double GetValue() const
      {
        return m_value;
      }

      std::int64_t AsInt() const
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

      inline double AsFloat() const
      {
        return m_value;
      }

      mpz_class AsBigInt() const
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

      bool Equals(const Ref<Value>& that) const;

      std::string ToString() const
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

    private:
      const double m_value;
    };

    class BigIntNumber : public Number
    {
    public:
      explicit BigIntNumber(const mpz_class& value)
        : m_value(value) {}

      inline NumberType GetNumberType() const
      {
        return NUMBER_TYPE_BIG_INT;
      }

      inline const mpz_class& GetValue() const
      {
        return m_value;
      }

      std::int64_t AsInt() const
      {
        return m_value.get_si();
      }

      double AsFloat() const
      {
        return m_value.get_d();
      }

      inline mpz_class AsBigInt() const
      {
        return m_value;
      }

      bool Equals(const Ref<Value>& that) const
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

      std::string ToString() const
      {
        return m_value.get_str(10);
      }

    private:
      const mpz_class m_value;
    };

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
  }

  Ref<Number> Runtime::NewNumber(std::int64_t value) const
  {
    return new (m_memory_manager) IntNumber(value);
  }

  Ref<Number> Runtime::NewNumber(double value) const
  {
    return new (m_memory_manager) FloatNumber(value);
  }

  Ref<Number> Runtime::NewNumber(const mpz_class& value) const
  {
    return new (m_memory_manager) BigIntNumber(value);
  }
}

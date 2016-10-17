#ifndef PLORTH_NUMBER_HPP_GUARD
#define PLORTH_NUMBER_HPP_GUARD

#include <cstdint>

#include <gmpxx.h>

#include "value.hpp"

namespace plorth
{
  class Number : public Value
  {
  public:
    enum NumberType
    {
      NUMBER_TYPE_INT,
      NUMBER_TYPE_FLOAT,
      NUMBER_TYPE_BIG_INT,
    };

    inline Type GetType() const
    {
      return TYPE_NUMBER;
    }

    virtual NumberType GetNumberType() const = 0;

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    virtual std::int64_t AsInt() const = 0;

    virtual double AsFloat() const = 0;

    virtual mpz_class AsBigInt() const = 0;
  };

  class IntNumber : public Number
  {
  public:
    explicit IntNumber(std::int64_t value);

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

    double AsFloat() const;

    mpz_class AsBigInt() const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

  private:
    const std::int64_t m_value;
  };

  class FloatNumber : public Number
  {
  public:
    explicit FloatNumber(double value);

    inline NumberType GetNumberType() const
    {
      return NUMBER_TYPE_FLOAT;
    }

    inline double GetValue() const
    {
      return m_value;
    }

    std::int64_t AsInt() const;

    inline double AsFloat() const
    {
      return m_value;
    }

    mpz_class AsBigInt() const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

  private:
    const double m_value;
  };

  class BigIntNumber : public Number
  {
  public:
    explicit BigIntNumber(const mpz_class& value);

    inline NumberType GetNumberType() const
    {
      return NUMBER_TYPE_BIG_INT;
    }

    inline const mpz_class& GetValue() const
    {
      return m_value;
    }

    std::int64_t AsInt() const;

    double AsFloat() const;

    inline mpz_class AsBigInt() const
    {
      return m_value;
    }

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

  private:
    const mpz_class m_value;
  };
}

#endif /* !PLORTH_NUMBER_HPP_GUARD */

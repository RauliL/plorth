#ifndef PLORTH_VALUE_NUMBER_HPP_GUARD
#define PLORTH_VALUE_NUMBER_HPP_GUARD

#include <plorth/value.hpp>

namespace plorth
{
  class number : public value
  {
  public:
    explicit number(double value);

    inline double value() const
    {
      return m_value;
    }

    inline enum type type() const
    {
      return type_number;
    }

    bool equals(const ref<class value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

  private:
    const double m_value;
  };
}

#endif /* !PLORTH_VALUE_NUMBER_HPP_GUARD */

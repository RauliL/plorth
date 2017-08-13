#ifndef PLORTH_VALUE_BOOLEAN_HPP_GUARD
#define PLORTH_VALUE_BOOLEAN_HPP_GUARD

#include <plorth/value.hpp>

namespace plorth
{
  class boolean : public value
  {
  public:
    explicit boolean(bool value);

    inline bool value() const
    {
      return m_value;
    }

    inline enum type type() const
    {
      return type_boolean;
    }

    bool equals(const ref<class value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

  private:
    const bool m_value;
  };
}

#endif /* !PLORTH_VALUE_BOOLEAN_HPP_GUARD */

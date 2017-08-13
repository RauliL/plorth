#ifndef PLORTH_VALUE_NULL_HPP_GUARD
#define PLORTH_VALUE_NULL_HPP_GUARD

#include <plorth/value.hpp>

namespace plorth
{
  class null : public value
  {
  public:
    explicit null();

    inline enum type type() const
    {
      return type_null;
    }

    bool equals(const ref<value>& that) const;
    unistring to_string() const;
    unistring to_source() const;
  };
}

#endif /* !PLORTH_VALUE_NULL_HPP_GUARD */

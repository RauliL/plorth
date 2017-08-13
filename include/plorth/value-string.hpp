#ifndef PLORTH_VALUE_STRING_HPP_GUARD
#define PLORTH_VALUE_STRING_HPP_GUARD

#include <plorth/value.hpp>

namespace plorth
{
  class string : public value
  {
  public:
    explicit string(const unistring& value);

    inline const unistring& value() const
    {
      return m_value;
    }

    inline enum type type() const
    {
      return type_string;
    }

    bool equals(const ref<class value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

  private:
    const unistring m_value;
  };
}

#endif /* !PLORTH_VALUE_STRING_HPP_GUARD */

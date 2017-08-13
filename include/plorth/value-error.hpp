#ifndef PLORTH_VALUE_ERROR_HPP_GUARD
#define PLORTH_VALUE_ERROR_HPP_GUARD

#include <plorth/value.hpp>

namespace plorth
{
  class error : public value
  {
  public:
    enum code
    {
      /** Syntax error. */
      code_syntax = 1,
      /** Reference error. */
      code_reference = 2,
      /** Type error. */
      code_type = 3,
      /** Range error. */
      code_range = 4,
      /** Import error. */
      code_import = 5,
      /** Unknown error. */
      code_unknown = 100
    };

    explicit error(enum code code, const unistring& message);

    inline enum code code() const
    {
      return m_code;
    }

    inline const unistring& message() const
    {
      return m_message;
    }

    inline enum type type() const
    {
      return type_error;
    }

    bool equals(const ref<value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

  private:
    const enum code m_code;
    const unistring m_message;
  };

  std::ostream& operator<<(std::ostream&, enum error::code);
}

#endif /* !PLORTH_VALUE_ERROR_HPP_GUARD */

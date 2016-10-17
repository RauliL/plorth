#ifndef PLORTH_ERROR_HPP_GUARD
#define PLORTH_ERROR_HPP_GUARD

#include "value.hpp"

namespace plorth
{
  class Error : public Value
  {
  public:
    enum ErrorCode
    {
      /** Syntax error. */
      ERROR_CODE_SYNTAX = 1,
      /** Reference error. */
      ERROR_CODE_REFERENCE = 2,
      /** Type error. */
      ERROR_CODE_TYPE = 3,
      /** Range error. */
      ERROR_CODE_RANGE = 4,
      /** Unknown error. */
      ERROR_CODE_UNKNOWN = 100
    };

    explicit Error(ErrorCode code, const std::string& message);

    inline Type GetType() const
    {
      return TYPE_ERROR;
    }

    inline ErrorCode GetCode() const
    {
      return m_code;
    }

    inline const std::string& GetMessage() const
    {
      return m_message;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const ErrorCode m_code;
    const std::string m_message;
  };

  std::ostream& operator<<(std::ostream&, Error::ErrorCode);
}

#endif /* !PLORTH_ERROR_HPP_GUARD */

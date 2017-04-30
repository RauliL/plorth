#include <sstream>

#include "context.hpp"

namespace plorth
{
  Error::Error(ErrorCode code, const std::string& message)
    : m_code(code)
    , m_message(message) {}

  Ref<Object> Error::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetErrorPrototype();
  }

  bool Error::Equals(const Ref<Value>& that) const
  {
    Ref<Error> other;

    if (that->GetType() != TYPE_ERROR)
    {
      return false;
    }
    other = that.As<Error>();

    return m_code == other->m_code && m_message == other->m_message;
  }

  std::string Error::ToString() const
  {
    std::stringstream ss;

    ss << m_code;
    if (!m_message.empty())
    {
      ss << ": " << m_message;
    }

    return ss.str();
  }

  std::string Error::ToSource() const
  {
    return "<" + ToString() + ">";
  }

  std::ostream& operator<<(std::ostream& os, Error::ErrorCode code)
  {
    const char* description = "unknown error";

    switch (code)
    {
      case Error::ERROR_CODE_SYNTAX:
        description = "syntax error";
        break;

      case Error::ERROR_CODE_REFERENCE:
        description = "reference error";
        break;

      case Error::ERROR_CODE_TYPE:
        description = "type error";
        break;

      case Error::ERROR_CODE_RANGE:
        description = "range error";
        break;

      case Error::ERROR_CODE_UNKNOWN:
        description = "unknown error";
        break;
    }
    os << description;

    return os;
  }
}

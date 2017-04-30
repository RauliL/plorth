#include "runtime.hpp"
#include "string.hpp"
#include "utils.hpp"

namespace plorth
{
  String::String(const std::string& value)
    : m_value(value) {}

  Ref<Object> String::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetStringPrototype();
  }

  bool String::Equals(const Ref<Value>& that) const
  {
    if (that->GetType() != TYPE_STRING)
    {
      return false;
    }

    return that.As<String>()->m_value == m_value;
  }

  std::string String::ToString() const
  {
    return m_value;
  }

  std::string String::ToSource() const
  {
    return to_json_string(m_value);
  }
}

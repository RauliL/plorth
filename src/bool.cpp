#include "bool.hpp"
#include "runtime.hpp"

namespace plorth
{
  Bool::Bool(bool value)
    : m_value(value) {}

  Ref<Object> Bool::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetBoolPrototype();
  }

  bool Bool::Equals(const Ref<Value>& that) const
  {
    if (that->GetType() != TYPE_BOOL)
    {
      return false;
    }

    return that.As<Bool>()->m_value == m_value;
  }

  std::string Bool::ToString() const
  {
    return m_value ? "true" : "false";
  }
}

#include "context.hpp"
#include "utils.hpp"

namespace plorth
{
  Object::Object() {}

  Object::Object(const Dictionary& properties)
    : m_properties(properties) {}

  Ref<Object> Object::GetPrototype(const Ref<Runtime>& runtime) const
  {
    const Ref<Value> value = GetOwnProperty("__proto__");

    if (value && value->GetType() == TYPE_OBJECT)
    {
      return value.As<Object>();
    } else {
      return runtime->GetObjectPrototype();
    }
  }

  Ref<Value> Object::GetOwnProperty(const std::string& name) const
  {
    const auto entry = m_properties.find(name);

    if (entry != end(m_properties))
    {
      return entry->second;
    }

    return Ref<Value>();
  }

  bool Object::Equals(const Ref<Value>& that) const
  {
    Ref<Object> other;

    if (that->GetType() != TYPE_OBJECT)
    {
      return false;
    }
    other = that.As<Object>();
    if (m_properties.size() != other->m_properties.size())
    {
      return false;
    }
    for (auto& property : m_properties)
    {
      const auto other_property = other->m_properties.find(property.first);

      if (other_property == end(other->m_properties)
          || !property.second->Equals(other_property->second))
      {
        return false;
      }
    }

    return true;
  }

  std::string Object::ToString() const
  {
    std::string result;
    bool first = true;

    for (auto& property : m_properties)
    {
      if (first)
      {
        first = false;
      } else {
        result += ", ";
      }
      result += property.first;
      result += "=";
      result += property.second->ToString();
    }

    return result;
  }

  std::string Object::ToSource() const
  {
    std::string result;
    bool first = true;

    result += "{";
    for (auto& property : m_properties)
    {
      if (first)
      {
        first = false;
      } else {
        result += ", ";
      }
      result += to_json_string(property.first);
      result += ": ";
      result += property.second->ToSource();
    }
    result += "}";

    return result;
  }
}

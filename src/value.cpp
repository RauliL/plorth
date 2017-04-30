#include <sstream>

#include <plorth/plorth-runtime.hpp>

#include "./utils.hpp"

namespace plorth
{
  Value::Value() {}

  Ref<Object> Value::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return Ref<Object>();
  }

  std::string Value::ToSource() const
  {
    return ToString();
  }

  Null::Null() {}

  bool Null::Equals(const Ref<Value>& that) const
  {
    return that->GetType() == TYPE_NULL;
  }

  std::string Null::ToString() const
  {
    return std::string();
  }

  std::string Null::ToSource() const
  {
    return "null";
  }

  std::ostream& operator<<(std::ostream& os, const Ref<Value>& value)
  {
    if (value)
    {
      os << value->ToString();
    } else {
      os << "<no value>";
    }

    return os;
  }

  std::ostream& operator<<(std::ostream& os, Value::Type type)
  {
    switch (type)
    {
      case Value::TYPE_NULL:
        os << "null";
        break;

      case Value::TYPE_BOOL:
        os << "bool";
        break;

      case Value::TYPE_NUMBER:
        os << "num";
        break;

      case Value::TYPE_STRING:
        os << "str";
        break;

      case Value::TYPE_ARRAY:
        os << "ary";
        break;

      case Value::TYPE_OBJECT:
        os << "obj";
        break;

      case Value::TYPE_QUOTE:
        os << "quote";
        break;

      case Value::TYPE_ERROR:
        os << "error";
        break;
    }

    return os;
  }

  Array::Array(const std::vector<Ref<Value>>& elements)
    : m_elements(elements) {}

  Ref<Object> Array::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetArrayPrototype();
  }

  bool Array::Equals(const Ref<Value>& that) const
  {
    Ref<Array> other;

    if (that->GetType() != TYPE_ARRAY)
    {
      return false;
    }
    other = that.As<Array>();
    if (m_elements.size() != other->m_elements.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (!m_elements[i]->Equals(other->m_elements[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::string Array::ToString() const
  {
    std::string result;

    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += ", ";
      }
      result += m_elements[i]->ToString();
    }

    return result;
  }

  std::string Array::ToSource() const
  {
    std::string result;

    result += "[";
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += ", ";
      }
      result += m_elements[i]->ToSource();
    }
    result += "]";

    return result;
  }

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

  Ref<Object> Number::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetNumberPrototype();
  }

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

  Ref<Object> Quote::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetQuotePrototype();
  }

  bool Quote::Equals(const Ref<Value>& that) const
  {
    return false; // TODO: Implement equality testing for native quotes.
  }

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

#include <sstream>

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

  /**
   * null ( -- null )
   *
   * Returns null value.
   */
  static void w_null(const Ref<Context>& context)
  {
    context->PushNull();
  }

  /**
   * obj? ( any -- any bool )
   *
   * Returns true if given value is object.
   */
  static void w_is_obj(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_OBJECT);
    }
  }

  /**
   * typeof ( any -- any str )
   *
   * Returns name of the type of given value as string.
   */
  static void w_typeof(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      std::stringstream ss;

      ss << value->GetType();
      context->PushString(ss.str());
    }
  }

  /**
   * proto ( any -- obj|null )
   *
   * Returns prototype of the value, or null if prototype cannot be determined
   * for some reason.
   */
  static void w_proto(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      const Ref<Object> prototype = value->GetPrototype(context->GetRuntime());

      if (prototype)
      {
        context->Push(prototype);
      } else {
        context->PushNull();
      }
    }
  }

  /**
   * repr ( any -- str )
   *
   * Returns source code representation of the value.
   */
  static void w_repr(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      context->PushString(value->ToSource());
    }
  }

  void api_init_object(Runtime* runtime)
  {
    runtime->AddWord("null", w_null);

    runtime->AddWord("obj?", w_is_obj);
    runtime->AddWord("typeof", w_typeof);
    runtime->AddWord("proto", w_proto);
    runtime->AddWord("repr", w_repr);
  }
}

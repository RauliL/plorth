#include <sstream>

#include "state.hpp"
#include "string.hpp"
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
      return runtime->GetObjectProperty();
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
  static void w_null(const Ref<State>& state)
  {
    state->PushNull();
  }

  /**
   * obj? ( any -- any bool )
   *
   * Returns true if given value is object.
   */
  static void w_is_obj(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Peek(value))
    {
      state->PushBool(value->GetType() == Value::TYPE_OBJECT);
    }
  }

  /**
   * typeof ( any -- any str )
   *
   * Returns name of the type of given value as string.
   */
  static void w_typeof(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Peek(value))
    {
      std::stringstream ss;

      ss << value->GetType();
      state->PushString(ss.str());
    }
  }

  /**
   * proto ( any -- obj|null )
   *
   * Returns prototype of the value, or null if prototype cannot be determined
   * for some reason.
   */
  static void w_proto(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Pop(value))
    {
      const Ref<Object> prototype = value->GetPrototype(state->GetRuntime());

      if (prototype)
      {
        state->Push(prototype);
      } else {
        state->PushNull();
      }
    }
  }

  /**
   * repr ( any -- str )
   *
   * Returns source code representation of the value.
   */
  static void w_repr(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Pop(value))
    {
      state->PushString(value->ToSource());
    }
  }

  /**
   * len ( obj -- obj num )
   *
   * Returns the number of key-value pairs from the object.
   */
  static void w_len(const Ref<State>& state)
  {
    Ref<Object> object;

    if (state->PeekObject(object))
    {
      state->PushNumber(static_cast<std::int64_t>(object->GetOwnProperties().size()));
    }
  }

  /**
   * keys ( obj -- obj ary )
   *
   * Retrieves all keys from the object and returns them in an array.
   */
  static void w_keys(const Ref<State>& state)
  {
    Ref<Object> object;
    std::vector<Ref<Value>> result;

    if (!state->PeekObject(object))
    {
      return;
    }
    for (const auto& property : object->GetOwnProperties())
    {
      result.push_back(state->GetRuntime()->NewString(property.first));
    }
    state->PushArray(result);
  }

  /**
   * values ( obj -- obj ary )
   *
   * Retrieves all values from the object and returns them in an array.
   */
  static void w_values(const Ref<State>& state)
  {
    Ref<Object> object;
    std::vector<Ref<Value>> result;

    if (!state->PeekObject(object))
    {
      return;
    }
    for (const auto& property : object->GetOwnProperties())
    {
      result.push_back(property.second);
    }
    state->PushArray(result);
  }

  /**
   * has? ( str obj -- bool )
   *
   * Tests whether value identified by given string exists in the object.
   */
  static void w_has(const Ref<State>& state)
  {
    Ref<Object> object;
    Ref<String> key;

    if (state->PopObject(object) && state->PopString(key))
    {
      const Ref<Value> value = object->GetOwnProperty(key->GetValue());

      state->PushBool(!!value);
    }
  }

  /**
   * @ ( str obj -- any )
   *
   * Retrieves value identified by given string from the object. If the object
   * does not have value for the identifier, null is returned instead.
   */
  static void w_get(const Ref<State>& state)
  {
    Ref<Object> object;
    Ref<String> key;

    if (state->PopObject(object) && state->PopString(key))
    {
      const Ref<Value> value = object->GetOwnProperty(key->GetValue());

      if (value)
      {
        state->Push(value);
      } else {
        state->Push(state->GetRuntime()->GetNullValue());
      }
    }
  }

  /**
   * ! ( any str obj -- obj )
   *
   * Assigns named value to the object and returns result.
   */
  static void w_set(const Ref<State>& state)
  {
    Ref<Object> object;
    Ref<String> key;
    Ref<Value> value;

    if (state->PopObject(object)
        && state->PopString(key)
        && state->Pop(value))
    {
      Object::Dictionary properties = object->GetOwnProperties();

      properties[key->GetValue()] = value;
      state->PushObject(properties);
    }
  }

  /**
   * + ( obj obj -- obj )
   *
   * Combines contents of two objects together and returns result.
   */
  static void w_plus(const Ref<State>& state)
  {
    Ref<Object> obj_a;
    Ref<Object> obj_b;

    if (state->PopObject(obj_a) && state->PopObject(obj_b))
    {
      Object::Dictionary result = obj_b->GetOwnProperties();

      for (const auto& property : obj_a->GetOwnProperties())
      {
        result[property.first] = property.second;
      }
      state->PushObject(result);
    }
  }

  void api_init_object(Runtime* runtime)
  {
    runtime->AddWord("null", w_null);

    runtime->AddWord("obj?", w_is_obj);
    runtime->AddWord("typeof", w_typeof);
    runtime->AddWord("proto", w_proto);
    runtime->AddWord("repr", w_repr);

    runtime->AddNamespace("obj", {
      { "len", w_len },
      { "keys", w_keys },
      { "values", w_values },

      { "has?", w_has },

      { "@", w_get },
      { "!", w_set },

      { "+", w_plus },
    });
  }
}

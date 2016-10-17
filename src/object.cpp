#include <sstream>

#include "state.hpp"
#include "string.hpp"
#include "utils.hpp"

namespace plorth
{
  Object::Object(const std::unordered_map<std::string, Ref<Value>>& entries)
    : m_entries(entries) {}

  Ref<Object> Object::GetPrototype(const Ref<Runtime>& runtime) const
  {
    Ref<Value> value = Find("__proto__");

    if (!value || value->GetType() != TYPE_OBJECT)
    {
      if (!runtime->FindWord("obj", value) || value->GetType() != TYPE_OBJECT)
      {
        return Ref<Object>();
      }
    }

    return value.As<Object>();
  }

  Ref<Value> Object::Find(const std::string& name) const
  {
    const auto entry = m_entries.find(name);

    if (entry != m_entries.end())
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
    if (m_entries.size() != other->m_entries.size())
    {
      return false;
    }
    for (auto& entry : m_entries)
    {
      const auto other_entry = other->m_entries.find(entry.first);

      if (other_entry == other->m_entries.end()
          || !entry.second->Equals(other_entry->second))
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

    for (auto& entry : m_entries)
    {
      if (first)
      {
        first = false;
      } else {
        result += ", ";
      }
      result += entry.first;
      result += "=";
      result += entry.second->ToString();
    }

    return result;
  }

  std::string Object::ToSource() const
  {
    std::string result;
    bool first = true;

    result += "{";
    for (auto& entry : m_entries)
    {
      if (first)
      {
        first = false;
      } else {
        result += ", ";
      }
      result += to_json_string(entry.first);
      result += ": ";
      result += entry.second->ToSource();
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
      state->PushNumber(static_cast<std::int64_t>(object->GetSize()));
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
    for (auto& entry : object->GetEntries())
    {
      result.push_back(state->GetRuntime()->NewString(entry.first));
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
    for (auto& entry : object->GetEntries())
    {
      result.push_back(entry.second);
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
      const Ref<Value> value = object->Find(key->GetValue());

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
      const Ref<Value> value = object->Find(key->GetValue());

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
      std::unordered_map<std::string, Ref<Value>> entries = object->GetEntries();

      entries[key->GetValue()] = value;
      state->PushObject(entries);
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
      std::unordered_map<std::string, Ref<Value>> result = obj_b->GetEntries();

      for (auto& entry : obj_a->GetEntries())
      {
        result[entry.first] = entry.second;
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

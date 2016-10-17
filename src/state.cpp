#include <sstream>

#include "array.hpp"
#include "state.hpp"
#include "string.hpp"
#include "utils.hpp"

namespace plorth
{
  State::State(const Ref<Runtime>& runtime)
    : m_runtime(runtime) {}

  void State::SetError(Error::ErrorCode code, const std::string& message)
  {
    m_error = new (m_runtime) Error(code, message);
  }

  void State::ClearError()
  {
    m_error.Clear();
  }

  Ref<Value> State::FindWord(const std::string& name) const
  {
    const auto entry = m_dictionary.find(name);

    if (entry != m_dictionary.end())
    {
      return entry->second;
    }

    return Ref<Value>();
  }

  void State::AddWord(const std::string& name, const Ref<Value>& value)
  {
    m_dictionary[name] = value;
  }

  bool State::CallWord(const std::string& name)
  {
    Ref<Value> value;

    // Look from prototype of current item.
    if (!m_data.empty())
    {
      const Ref<Object> prototype = m_data.back()->GetPrototype(m_runtime);

      if (prototype && (value = prototype->Find(name)))
      {
        if (value->GetType() == Value::TYPE_QUOTE)
        {
          return value.As<Quote>()->Call(this);
        }
        m_data.push_back(value);

        return true;
      }
    }

    // Look for word from dictionary of current context.
    if ((value = FindWord(name)))
    {
      if (value->GetType() == Value::TYPE_QUOTE)
      {
        return value.As<Quote>()->Call(this);
      }
      m_data.push_back(value);

      return true;
    }

    // TODO: If not found, see if it's a "fully qualified" name, e.g. a name
    // with a namespace name, colon and a word - Such as "num:+", and then look
    // for that from the specified namespace.

    // Look from global dictionary.
    if (m_runtime->FindWord(name, value))
    {
      if (value->GetType() == Value::TYPE_QUOTE)
      {
        return value.As<Quote>()->Call(this);
      }
      m_data.push_back(value);

      return true;
    }

    // If the name of the word can be converted into number, then do just that.
    if (str_is_number(name))
    {
      Push(m_runtime->NewNumber(name));

      return true;
    }

    // Otherwise it's reference error.
    SetError(Error::ERROR_CODE_REFERENCE, "Unrecognized word: `" + name + "'");

    return false;
  }

  bool State::Peek(Ref<Value>& slot)
  {
    if (m_data.empty())
    {
      SetError(Error::ERROR_CODE_RANGE, "Stack underflow.");

      return false;
    }
    slot = m_data.back();

    return true;
  }

  bool State::Peek(Ref<Value>& slot, Value::Type type)
  {
    if (!Peek(slot))
    {
      return false;
    }
    else if (slot->GetType() != type)
    {
      std::stringstream ss;

      ss << "Expected " << type << ", got " << slot->GetType() << " instead.";
      SetError(Error::ERROR_CODE_TYPE, ss.str());

      return false;
    }

    return true;
  }

  bool State::PeekString(Ref<String>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_STRING))
    {
      slot = value.As<String>();

      return true;
    }

    return false;
  }

  bool State::PeekArray(Ref<Array>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_ARRAY))
    {
      slot = value.As<Array>();

      return true;
    }

    return false;
  }

  bool State::PeekObject(Ref<Object>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_OBJECT))
    {
      slot = value.As<Object>();

      return true;
    }

    return false;
  }

  bool State::PeekError(Ref<Error>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_ERROR))
    {
      slot = value.As<Error>();

      return true;
    }

    return false;
  }

  void State::Push(const Ref<Value>& value)
  {
    m_data.push_back(value);
  }

  void State::PushNull()
  {
    Push(m_runtime->GetNullValue());
  }

  void State::PushBool(bool value)
  {
    Push(m_runtime->NewBool(value));
  }

  void State::PushNumber(std::int64_t value)
  {
    Push(m_runtime->NewNumber(value));
  }

  void State::PushNumber(double value)
  {
    Push(m_runtime->NewNumber(value));
  }

  void State::PushNumber(const mpz_class& value)
  {
    Push(m_runtime->NewNumber(value));
  }

  void State::PushNumber(const std::string& value)
  {
    Push(m_runtime->NewNumber(value));
  }

  void State::PushString(const std::string& value)
  {
    Push(m_runtime->NewString(value));
  }

  void State::PushArray(const std::vector<Ref<Value>>& elements)
  {
    Push(m_runtime->NewArray(elements));
  }

  void State::PushObject(const std::unordered_map<std::string, Ref<Value>>& entries)
  {
    Push(m_runtime->NewObject(entries));
  }

  bool State::Pop()
  {
    if (m_data.empty())
    {
      SetError(Error::ERROR_CODE_RANGE, "Stack underflow.");

      return false;
    }
    m_data.pop_back();

    return true;
  }

  bool State::Pop(Ref<Value>& slot)
  {
    if (!Peek(slot))
    {
      return false;
    }
    m_data.pop_back();

    return true;
  }

  bool State::Pop(Ref<Value>& slot, Value::Type type)
  {
    if (!Peek(slot, type))
    {
      return false;
    }
    m_data.pop_back();

    return true;
  }

  bool State::PopBool(Ref<Bool>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_BOOL))
    {
      slot = value.As<Bool>();

      return true;
    }

    return false;
  }

  bool State::PopNumber(Ref<Number>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_NUMBER))
    {
      slot = value.As<Number>();

      return true;
    }

    return false;
  }

  bool State::PopString(Ref<String>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_STRING))
    {
      slot = value.As<String>();

      return true;
    }

    return false;
  }

  bool State::PopArray(Ref<Array>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_ARRAY))
    {
      slot = value.As<Array>();

      return true;
    }

    return false;
  }

  bool State::PopObject(Ref<Object>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_OBJECT))
    {
      slot = value.As<Object>();

      return true;
    }

    return false;
  }

  bool State::PopQuote(Ref<Quote>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_QUOTE))
    {
      slot = value.As<Quote>();

      return true;
    }

    return false;
  }

  bool State::PopError(Ref<Error>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_ERROR))
    {
      slot = value.As<Error>();

      return true;
    }

    return false;
  }
}

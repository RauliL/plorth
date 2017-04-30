#include <sstream>

#include <plorth/plorth-context.hpp>

#include "./utils.hpp"

namespace plorth
{
  Context::Context(const Ref<Runtime>& runtime)
    : m_runtime(runtime) {}

  void Context::SetError(Error::ErrorCode code, const std::string& message)
  {
    m_error = new (m_runtime) Error(code, message);
  }

  void Context::ClearError()
  {
    m_error.Clear();
  }

  bool Context::CallWord(const std::string& name)
  {
    Ref<Value> value;

    // Look from prototype of current item.
    if (!m_stack.empty())
    {
      const Ref<Object> prototype = m_stack.back()->GetPrototype(m_runtime);

      if (prototype && (value = prototype->GetOwnProperty(name)))
      {
        if (value->GetType() == Value::TYPE_QUOTE)
        {
          return value.As<Quote>()->Call(this);
        }
        m_stack.push_back(value);

        return true;
      }
    }

    // Look for word from dictionary of current context.
    {
      const auto word = m_dictionary.find(name);

      if (word != end(m_dictionary))
      {
        value = word->second;
        if (value->GetType() == Value::TYPE_QUOTE)
        {
          return value.As<Quote>()->Call(this);
        }
        m_stack.push_back(value);

        return true;
      }
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
      m_stack.push_back(value);

      return true;
    }

    // If the name of the word can be converted into number, then do just that.
    if (str_is_number(name))
    {
      PushNumber(name);

      return true;
    }

    // Otherwise it's reference error.
    SetError(Error::ERROR_CODE_REFERENCE, "Unrecognized word: `" + name + "'");

    return false;
  }

  void Context::AddWord(const std::string& name, const Ref<Value>& value)
  {
    m_dictionary[name] = value;
  }

  bool Context::Peek(Ref<Value>& slot)
  {
    if (m_stack.empty())
    {
      SetError(Error::ERROR_CODE_RANGE, "Stack underflow.");

      return false;
    }
    slot = m_stack.back();

    return true;
  }

  bool Context::Peek(Ref<Value>& slot, Value::Type type)
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

  bool Context::PeekArray(Ref<Array>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_ARRAY))
    {
      slot = value.As<Array>();

      return true;
    }

    return false;
  }

  bool Context::PeekBool(bool& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_BOOL))
    {
      slot = value.As<Bool>()->GetValue();

      return true;
    }

    return false;
  }

  bool Context::PeekError(Ref<Error>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_ERROR))
    {
      slot = value.As<Error>();

      return true;
    }

    return false;
  }

  bool Context::PeekNumber(Ref<Number>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_NUMBER))
    {
      slot = value.As<Number>();

      return true;
    }

    return false;
  }

  bool Context::PeekObject(Ref<Object>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_OBJECT))
    {
      slot = value.As<Object>();

      return true;
    }

    return false;
  }

  bool Context::PeekQuote(Ref<Quote>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_QUOTE))
    {
      slot = value.As<Quote>();

      return true;
    }

    return false;
  }

  bool Context::PeekString(Ref<String>& slot)
  {
    Ref<Value> value;

    if (Peek(value, Value::TYPE_STRING))
    {
      slot = value.As<String>();

      return true;
    }

    return false;
  }

  void Context::Push(const Ref<Value>& value)
  {
    m_stack.push_back(value);
  }

  void Context::PushArray(const std::vector<Ref<Value>>& elements)
  {
    m_stack.push_back(m_runtime->NewArray(elements));
  }

  void Context::PushBool(bool value)
  {
    m_stack.push_back(value ? m_runtime->GetTrueValue() : m_runtime->GetFalseValue());
  }

  void Context::PushNull()
  {
    m_stack.push_back(m_runtime->GetNullValue());
  }

  void Context::PushNumber(std::int64_t value)
  {
    m_stack.push_back(m_runtime->NewNumber(value));
  }

  void Context::PushNumber(double value)
  {
    m_stack.push_back(m_runtime->NewNumber(value));
  }

  void Context::PushNumber(const mpz_class& value)
  {
    m_stack.push_back(m_runtime->NewNumber(value));
  }

  void Context::PushNumber(const std::string& value)
  {
    m_stack.push_back(m_runtime->NewNumber(value));
  }

  void Context::PushObject()
  {
    m_stack.push_back(m_runtime->NewObject());
  }

  void Context::PushObject(const Object::Dictionary& properties)
  {
    m_stack.push_back(m_runtime->NewObject(properties));
  }

  void Context::PushString(const std::string& value)
  {
    m_stack.push_back(m_runtime->NewString(value));
  }

  bool Context::Pop()
  {
    if (m_stack.empty())
    {
      SetError(Error::ERROR_CODE_RANGE, "Stack underflow.");

      return false;
    }
    m_stack.pop_back();

    return true;
  }

  bool Context::Pop(Ref<Value>& slot)
  {
    if (!Peek(slot))
    {
      return false;
    }
    m_stack.pop_back();

    return true;
  }

  bool Context::Pop(Ref<Value>& slot, Value::Type type)
  {
    if (!Peek(slot))
    {
      return false;
    }
    m_stack.pop_back();
    if (slot->GetType() != type)
    {
      std::stringstream ss;

      ss << "Expected " << type << ", got " << slot->GetType() << " instead.";
      SetError(Error::ERROR_CODE_TYPE, ss.str());

      return false;
    }

    return true;
  }

  bool Context::PopArray(Ref<Array>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_ARRAY))
    {
      slot = value.As<Array>();

      return true;
    }

    return false;
  }

  bool Context::PopBool(bool& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_BOOL))
    {
      slot = value.As<Bool>()->GetValue();

      return true;
    }

    return false;
  }

  bool Context::PopError(Ref<Error>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_ERROR))
    {
      slot = value.As<Error>();

      return true;
    }

    return false;
  }

  bool Context::PopNumber(Ref<Number>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_NUMBER))
    {
      slot = value.As<Number>();

      return true;
    }

    return false;
  }

  bool Context::PopObject(Ref<Object>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_OBJECT))
    {
      slot = value.As<Object>();

      return true;
    }

    return false;
  }

  bool Context::PopQuote(Ref<Quote>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_QUOTE))
    {
      slot = value.As<Quote>();

      return true;
    }

    return false;
  }

  bool Context::PopString(Ref<String>& slot)
  {
    Ref<Value> value;

    if (Pop(value, Value::TYPE_STRING))
    {
      slot = value.As<String>();

      return true;
    }

    return false;
  }
}

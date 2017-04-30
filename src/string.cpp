#include "context.hpp"
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

  /**
   * str? ( any -- any bool )
   *
   * Returns true if given value is string.
   */
  static void w_is_str(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_STRING);
    }
  }

  /**
   * >str ( any -- str )
   *
   * Converts value from top of the stack into string.
   */
  static void w_to_str(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      context->PushString(value->ToString());
    }
  }

  void api_init_string(Runtime* runtime)
  {
    runtime->AddWord("str?", w_is_str);
    runtime->AddWord(">str", w_to_str);
  }
}

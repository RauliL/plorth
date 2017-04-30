#include "bool.hpp"
#include "context.hpp"

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

  /**
   * true ( -- bool )
   *
   * Inserts true value on top of the stack.
   */
  static void w_true(const Ref<Context>& context)
  {
    context->PushBool(true);
  }

  /**
   * false ( -- bool )
   *
   * Inserts false value on top of the stack.
   */
  static void w_false(const Ref<Context>& context)
  {
    context->PushBool(false);
  }

  /**
   * bool? ( any -- any bool )
   *
   * Returns true if given value is boolean.
   */
  static void w_is_bool(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_BOOL);
    }
  }

  /**
   * >bool ( any -- bool )
   *
   * Converts value from top of the stack into boolean. Null and false values
   * becomes false while everything else becomes true.
   */
  static void w_to_bool(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (!context->Pop(value))
    {
      return;
    }
    if (value->GetType() == Value::TYPE_BOOL)
    {
      context->PushBool(value);
    } else {
      context->PushBool(value->GetType() != Value::TYPE_NULL);
    }
  }

  /**
   * if ( bool quote -- )
   *
   * Executes quote if the boolean value is true.
   */
  static void w_if(const Ref<Context>& context)
  {
    bool condition;
    Ref<Quote> quote;

    if (context->PopQuote(quote) && context->PopBool(condition) && condition)
    {
      quote->Call(context);
    }
  }

  /**
   * if-else ( bool quote quote -- )
   *
   * Calls first quote if boolean value is true, second quote otherwise.
   */
  static void w_if_else(const Ref<Context>& context)
  {
    bool condition;
    Ref<Quote> then_quote;
    Ref<Quote> else_quote;

    if (!context->PopQuote(else_quote)
        || !context->PopQuote(then_quote)
        || !context->PopBool(condition))
    {
      return;
    }
    if (condition)
    {
      then_quote->Call(context);
    } else {
      else_quote->Call(context);
    }
  }

  /**
   * while ( bool quote -- )
   *
   * Executes given quote while top of the stack value is true.
   */
  static void w_while(const Ref<Context>& context)
  {
    bool condition;
    Ref<Quote> quote;

    if (!context->PopQuote(quote) || !context->PopBool(condition))
    {
      return;
    }
    while (condition)
    {
      if (!quote->Call(context) || !context->PopBool(condition))
      {
        return;
      }
    }
  }

  void api_init_bool(Runtime* runtime)
  {
    runtime->AddWord("true", w_true);
    runtime->AddWord("false", w_false);

    runtime->AddWord("bool?", w_is_bool);
    runtime->AddWord(">bool", w_to_bool);

    runtime->AddWord("if", w_if);
    runtime->AddWord("if-else", w_if_else);
    runtime->AddWord("while", w_while);
  }
}

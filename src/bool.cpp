#include "bool.hpp"
#include "state.hpp"

namespace plorth
{
  Bool::Bool(bool value)
    : m_value(value) {}

  Ref<Object> Bool::GetPrototype(const Ref<Runtime>& runtime) const
  {
    Ref<Value> value;

    if (runtime->FindWord("bool", value) && value->GetType() == TYPE_OBJECT)
    {
      return value.As<Object>();
    } else {
      return Ref<Object>();
    }
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
  static void w_true(const Ref<State>& state)
  {
    state->PushBool(true);
  }

  /**
   * false ( -- bool )
   *
   * Inserts false value on top of the stack.
   */
  static void w_false(const Ref<State>& state)
  {
    state->PushBool(false);
  }

  /**
   * bool? ( any -- any bool )
   *
   * Returns true if given value is boolean.
   */
  static void w_is_bool(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Peek(value))
    {
      state->PushBool(value->GetType() == Value::TYPE_BOOL);
    }
  }

  /**
   * >bool ( any -- bool )
   *
   * Converts value from top of the stack into boolean. Null and false values
   * becomes false while everything else becomes true.
   */
  static void w_to_bool(const Ref<State>& state)
  {
    Ref<Value> value;

    if (!state->Pop(value))
    {
      return;
    }
    if (value->GetType() == Value::TYPE_BOOL)
    {
      state->PushBool(value);
    } else {
      state->PushBool(value->GetType() != Value::TYPE_NULL);
    }
  }

  /**
   * if ( bool quote -- )
   *
   * Executes quote if the boolean value is true.
   */
  static void w_if(const Ref<State>& state)
  {
    Ref<Bool> condition;
    Ref<Quote> quote;

    if (state->PopQuote(quote)
        && state->PopBool(condition)
        && condition->GetValue())
    {
      quote->Call(state);
    }
  }

  /**
   * if-else ( bool quote quote -- )
   *
   * Calls first quote if boolean value is true, second quote otherwise.
   */
  static void w_if_else(const Ref<State>& state)
  {
    Ref<Bool> condition;
    Ref<Quote> then_quote;
    Ref<Quote> else_quote;

    if (!state->PopQuote(else_quote)
        || !state->PopQuote(then_quote)
        || !state->PopBool(condition))
    {
      return;
    }
    if (condition->GetValue())
    {
      then_quote->Call(state);
    } else {
      else_quote->Call(state);
    }
  }

  /**
   * while ( bool quote -- )
   *
   * Executes given quote while top of the stack value is true.
   */
  static void w_while(const Ref<State>& state)
  {
    Ref<Bool> condition;
    Ref<Quote> quote;

    if (!state->PopQuote(quote) || !state->PopBool(condition))
    {
      return;
    }
    while (condition->GetValue())
    {
      if (!quote->Call(state) || !state->PopBool(condition))
      {
        return;
      }
    }
  }

  /**
   * and ( bool bool -- bool )
   *
   * Logical AND. Returns true if both values are true.
   */
  static void w_and(const Ref<State>& state)
  {
    Ref<Bool> a;
    Ref<Bool> b;

    if (state->PopBool(a) && state->PopBool(b))
    {
      state->PushBool(b->GetValue() && a->GetValue());
    }
  }

  /**
   * or ( bool bool -- bool )
   *
   * Logical OR. Returns true if either one of the values are true.
   */
  static void w_or(const Ref<State>& state)
  {
    Ref<Bool> a;
    Ref<Bool> b;

    if (state->PopBool(a) && state->PopBool(b))
    {
      state->PushBool(b->GetValue() || a->GetValue());
    }
  }

  /**
   * xor ( bool bool -- bool )
   *
   * Exclusive OR.
   */
  static void w_xor(const Ref<State>& state)
  {
    Ref<Bool> a;
    Ref<Bool> b;

    if (state->PopBool(a) && state->PopBool(b))
    {
      const bool x = b->GetValue();
      const bool y = a->GetValue();

      state->PushBool(x != y && (x || y));
    }
  }

  /**
   * not ( bool -- bool )
   *
   * Negates given boolean value.
   */
  static void w_not(const Ref<State>& state)
  {
    Ref<Bool> value;

    if (state->PopBool(value))
    {
      state->PushBool(!value->GetValue());
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

    runtime->AddNamespace("bool", {
        { "and", w_and },
        { "or", w_or },
        { "xor", w_xor },
        { "not", w_not },
      }
    );
  }
}

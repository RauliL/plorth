#include "state.hpp"

namespace plorth
{
  /**
   * null? ( any -- any bool )
   *
   * Returns true if item on top of the stack is null.
   */
  static void w_is_null(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Peek(value))
    {
      state->PushBool(value->GetType() == Value::TYPE_NULL);
    }
  }

  /**
   * nop ( -- )
   *
   * Does nothing. Can be used to construct empty quotes.
   */
  static void w_nop(const Ref<State>& state) {}

  /**
   * clear ( -- )
   *
   * Clears the entire stack.
   */
  static void w_clear(const Ref<State>& state)
  {
    state->GetDataStack().clear();
  }

  /**
   * depth ( -- num )
   *
   * Returns current depth of the stack.
   */
  static void w_depth(const Ref<State>& state)
  {
    state->PushNumber(static_cast<std::int64_t>(state->GetDataStack().size()));
  }

  /**
   * drop ( any -- )
   *
   * Discards top-most value of the stack.
   */
  static void w_drop(const Ref<State>& state)
  {
    state->Pop();
  }

  /**
   * drop2 ( any any -- )
   *
   * Discards two top-most values of the stack.
   */
  static void w_drop2(const Ref<State>& state)
  {
    if (state->Pop())
    {
      state->Pop();
    }
  }

  /**
   * dup ( any -- any any )
   *
   * Duplicates top-most value of the stack.
   */
  static void w_dup(const Ref<State>& state)
  {
    if (state->GetDataStack().empty())
    {
      state->SetError(Error::ERROR_CODE_RANGE, "Stack underflow");
    } else {
      state->Push(state->GetDataStack().back());
    }
  }

  /**
   * dup2 ( any any -- any any any any )
   *
   * Duplicates two top-most values on the stack.
   */
  static void w_dup2(const Ref<State>& state)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (state->Pop(a) && state->Pop(b))
    {
      state->Push(b);
      state->Push(a);
      state->Push(b);
      state->Push(a);
    }
  }

  /**
   * nip ( any any -- any )
   *
   * Drops the first value and pushes second value on the stack.
   */
  static void w_nip(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Pop(value) && state->Pop())
    {
      state->Push(value);
    }
  }

  /**
   * over ( any any -- any any any )
   *
   * Copies second top-most value from the stack into top-most value of the
   * stack.
   */
  static void w_over(const Ref<State>& state)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (state->Pop(a) && state->Pop(b))
    {
      state->Push(b);
      state->Push(a);
      state->Push(b);
    }
  }

  /**
   * rot ( any any any -- any any any )
   *
   * Rotates three top-most values on the stack.
   */
  static void w_rot(const Ref<State>& state)
  {
    Ref<Value> a;
    Ref<Value> b;
    Ref<Value> c;

    if (state->Pop(a) && state->Pop(b) && state->Pop(c))
    {
      state->Push(b);
      state->Push(a);
      state->Push(c);
    }
  }

  /**
   * swap ( any any -- any any )
   *
   * Swaps positions of two top-most values on the stack.
   */
  static void w_swap(const Ref<State>& state)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (state->Pop(a) && state->Pop(b))
    {
      state->Push(a);
      state->Push(b);
    }
  }

  void api_init_stack(Runtime* runtime)
  {
    runtime->AddWord("null?", w_is_null);
    runtime->AddWord("nop", w_nop);
    runtime->AddWord("clear", w_clear);
    runtime->AddWord("depth", w_depth);
    runtime->AddWord("drop", w_drop);
    runtime->AddWord("drop2", w_drop2);
    runtime->AddWord("dup", w_dup);
    runtime->AddWord("dup2", w_dup2);
    runtime->AddWord("nip", w_nip);
    runtime->AddWord("over", w_over);
    runtime->AddWord("rot", w_rot);
    runtime->AddWord("swap", w_swap);
  }
}

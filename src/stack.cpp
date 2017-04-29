#include "context.hpp"

namespace plorth
{
  /**
   * null? ( any -- any bool )
   *
   * Returns true if item on top of the stack is null.
   */
  static void w_is_null(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_NULL);
    }
  }

  /**
   * nop ( -- )
   *
   * Does nothing. Can be used to construct empty quotes.
   */
  static void w_nop(const Ref<Context>& context) {}

  /**
   * clear ( -- )
   *
   * Clears the entire stack.
   */
  static void w_clear(const Ref<Context>& context)
  {
    context->ClearStack();
  }

  /**
   * depth ( -- num )
   *
   * Returns current depth of the stack.
   */
  static void w_depth(const Ref<Context>& context)
  {
    context->PushNumber(static_cast<std::int64_t>(context->GetStack().size()));
  }

  /**
   * drop ( any -- )
   *
   * Discards top-most value of the stack.
   */
  static void w_drop(const Ref<Context>& context)
  {
    context->Pop();
  }

  /**
   * drop2 ( any any -- )
   *
   * Discards two top-most values of the stack.
   */
  static void w_drop2(const Ref<Context>& context)
  {
    if (context->Pop())
    {
      context->Pop();
    }
  }

  /**
   * dup ( any -- any any )
   *
   * Duplicates top-most value of the stack.
   */
  static void w_dup(const Ref<Context>& context)
  {
    if (context->GetStack().empty())
    {
      context->SetError(Error::ERROR_CODE_RANGE, "Stack underflow");
    } else {
      context->Push(context->GetStack().back());
    }
  }

  /**
   * dup2 ( any any -- any any any any )
   *
   * Duplicates two top-most values on the stack.
   */
  static void w_dup2(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (context->Pop(a) && context->Pop(b))
    {
      context->Push(b);
      context->Push(a);
      context->Push(b);
      context->Push(a);
    }
  }

  /**
   * nip ( any any -- any )
   *
   * Drops the first value and pushes second value on the stack.
   */
  static void w_nip(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value) && context->Pop())
    {
      context->Push(value);
    }
  }

  /**
   * over ( any any -- any any any )
   *
   * Copies second top-most value from the stack into top-most value of the
   * stack.
   */
  static void w_over(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (context->Pop(a) && context->Pop(b))
    {
      context->Push(b);
      context->Push(a);
      context->Push(b);
    }
  }

  /**
   * rot ( any any any -- any any any )
   *
   * Rotates three top-most values on the stack.
   */
  static void w_rot(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;
    Ref<Value> c;

    if (context->Pop(a) && context->Pop(b) && context->Pop(c))
    {
      context->Push(b);
      context->Push(a);
      context->Push(c);
    }
  }

  /**
   * swap ( any any -- any any )
   *
   * Swaps positions of two top-most values on the stack.
   */
  static void w_swap(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (context->Pop(a) && context->Pop(b))
    {
      context->Push(a);
      context->Push(b);
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

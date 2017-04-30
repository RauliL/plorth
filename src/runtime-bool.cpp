#include "context.hpp"

namespace plorth
{
  /**
   * and ( bool bool -- bool )
   *
   * Logical AND. Returns true if both values are true.
   */
  static void w_and(const Ref<Context>& context)
  {
    bool a;
    bool b;

    if (context->PopBool(a) && context->PopBool(b))
    {
      context->PushBool(b && a);
    }
  }

  /**
   * or ( bool bool -- bool )
   *
   * Logical OR. Returns true if either one of the values are true.
   */
  static void w_or(const Ref<Context>& context)
  {
    bool a;
    bool b;

    if (context->PopBool(a) && context->PopBool(b))
    {
      context->PushBool(b || a);
    }
  }

  /**
   * xor ( bool bool -- bool )
   *
   * Exclusive OR.
   */
  static void w_xor(const Ref<Context>& context)
  {
    bool a;
    bool b;

    if (context->PopBool(a) && context->PopBool(b))
    {
      context->PushBool(b != a && (b || a));
    }
  }

  /**
   * not ( bool -- bool )
   *
   * Negates given boolean value.
   */
  static void w_not(const Ref<Context>& context)
  {
    bool value;

    if (context->PopBool(value))
    {
      context->PushBool(!value);
    }
  }

  Ref<Object> make_bool_prototype(Runtime* runtime)
  {
    return runtime->NewPrototype({
      { "and", w_and },
      { "or", w_or },
      { "xor", w_xor },
      { "not", w_not },
    });
  }
}

#include <plorth/context.hpp>

namespace plorth
{
  boolean::boolean(bool value)
    : m_value(value) {}

  bool boolean::equals(const ref<class value>& that) const
  {
    if (!that->is(type_boolean))
    {
      return false;
    }

    return m_value == that.cast<boolean>()->m_value;
  }

  unistring boolean::to_string() const
  {
    return utf8_decode(m_value ? "true" : "false");
  }

  unistring boolean::to_source() const
  {
    return to_string();
  }

  /**
   * and ( bool bool -- bool )
   *
   * Logical AND. Returns true if both values are true.
   */
  static void w_and(const ref<context>& ctx)
  {
    bool a;
    bool b;

    if (ctx->pop_boolean(a) && ctx->pop_boolean(b))
    {
      ctx->push_boolean(b && a);
    }
  }

  /**
   * or ( bool bool -- bool )
   *
   * Logical OR. Returns true if either one of the values are true.
   */
  static void w_or(const ref<context>& ctx)
  {
    bool a;
    bool b;

    if (ctx->pop_boolean(a) && ctx->pop_boolean(b))
    {
      ctx->push_boolean(b || a);
    }
  }

  /**
   * xor ( bool bool -- bool )
   *
   * Exclusive OR.
   */
  static void w_xor(const ref<context>& ctx)
  {
    bool a;
    bool b;

    if (ctx->pop_boolean(a) && ctx->pop_boolean(b))
    {
      ctx->push_boolean(b != a && (b || a));
    }
  }

  /**
   * not ( bool -- bool )
   *
   * Negates given boolean value.
   */
  static void w_not(const ref<context>& ctx)
  {
    bool value;

    if (ctx->pop_boolean(value))
    {
      ctx->push_boolean(!value);
    }
  }

  namespace api
  {
    runtime::prototype_definition boolean_prototype()
    {
      return
      {
        { "and", w_and },
        { "or", w_or },
        { "xor", w_xor },
        { "not", w_not },
      };
    }
  }
}

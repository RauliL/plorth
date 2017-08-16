/*
 * Copyright (c) 2017, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <plorth/context.hpp>

namespace plorth
{
  boolean::boolean(bool value)
    : m_value(value) {}

  bool boolean::equals(const ref<class value>& that) const
  {
    if (!that || !that->is(type_boolean))
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

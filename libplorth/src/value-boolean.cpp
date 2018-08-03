/*
 * Copyright (c) 2017-2018, Rauli Laine
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

  bool boolean::equals(const std::shared_ptr<class value>& that) const
  {
    if (!is(that, type::boolean))
    {
      return false;
    }

    return m_value == std::static_pointer_cast<boolean>(that)->m_value;
  }

  std::u32string boolean::to_string() const
  {
    return m_value ? U"true" : U"false";
  }

  std::u32string boolean::to_source() const
  {
    return to_string();
  }

  /**
   * Word: and
   * Prototype: boolean
   *
   * Takes:
   * - boolean
   * - boolean
   *
   * Gives:
   * - boolean
   *
   * Logical AND. Returns true if both values are true.
   */
  static void w_and(const std::shared_ptr<context>& ctx)
  {
    bool a;
    bool b;

    if (ctx->pop_boolean(a) && ctx->pop_boolean(b))
    {
      ctx->push_boolean(b && a);
    }
  }

  /**
   * Word: or
   * Prototype: boolean
   *
   * Takes:
   * - boolean
   * - boolean
   *
   * Gives:
   * - boolean
   *
   * Logical OR. Returns true if either one of the values are true.
   */
  static void w_or(const std::shared_ptr<context>& ctx)
  {
    bool a;
    bool b;

    if (ctx->pop_boolean(a) && ctx->pop_boolean(b))
    {
      ctx->push_boolean(b || a);
    }
  }

  /**
   * Word: xor
   * Prototype: boolean
   *
   * Takes:
   * - boolean
   * - boolean
   *
   * Gives:
   * - boolean
   *
   * Exclusive OR.
   */
  static void w_xor(const std::shared_ptr<context>& ctx)
  {
    bool a;
    bool b;

    if (ctx->pop_boolean(a) && ctx->pop_boolean(b))
    {
      ctx->push_boolean(b != a && (b || a));
    }
  }

  /**
   * Word: not
   * Prototype: boolean
   *
   * Takes:
   * - boolean
   *
   * Gives:
   * - boolean
   *
   * Negates given boolean value.
   */
  static void w_not(const std::shared_ptr<context>& ctx)
  {
    bool value;

    if (ctx->pop_boolean(value))
    {
      ctx->push_boolean(!value);
    }
  }

  /**
   * Word: ?
   * Prototype: boolean
   *
   * Takes:
   * - any
   * - any
   * - boolean
   *
   * Gives:
   * - any
   *
   * Selects between two values based on the boolean value. First value is
   * returned when the boolean value is true and the second one is returned
   * when it's false.
   *
   *     "greater" "less" 5 6 > ?  #=> "less"
   */
  static void w_select(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> true_value;
    std::shared_ptr<value> false_value;
    bool condition;

    if (ctx->pop_boolean(condition) &&
        ctx->pop(false_value) &&
        ctx->pop(true_value))
    {
      ctx->push(condition ? true_value : false_value);
    }
  }

  namespace api
  {
    runtime::prototype_definition boolean_prototype()
    {
      return
      {
        { U"and", w_and },
        { U"or", w_or },
        { U"xor", w_xor },
        { U"not", w_not },
        { U"?", w_select },
      };
    }
  }
}

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
  error::error(enum code code,
               const std::u32string& message,
               const std::optional<struct position>& position)
    : m_code(code)
    , m_message(message)
    , m_position(position) {}

  std::u32string error::code_description() const
  {
    return code_description(m_code);
  }

  std::u32string error::code_description(enum code code)
  {
    switch (code)
    {
    case error::code::syntax:
      return U"Syntax error";

    case error::code::reference:
      return U"Reference error";

    case error::code::type:
      return U"Type error";

    case error::code::value:
      return U"Value error";

    case error::code::range:
      return U"Range error";

    case error::code::import:
      return U"Import error";

    case error::code::io:
      return U"I/O error";

    case error::code::unknown:
      return U"Unknown error";
    }

    return U"Unknown error";
  }

  bool error::equals(const ref<value>& that) const
  {
    ref<error> err;

    if (!is(that, type::error))
    {
      return false;
    }

    err = that.cast<error>();

    return m_code == err->m_code && !m_message.compare(err->m_message);
  }

  std::u32string error::to_string() const
  {
    std::u32string result;

    result += code_description();
    if (!m_message.empty())
    {
      result += U": " + m_message;
    }

    return result;
  }

  std::u32string error::to_source() const
  {
    return U"<" + to_string() + U">";
  }

  std::ostream& operator<<(std::ostream& out, enum error::code code)
  {
    out << utf8_encode(error::code_description(code));

    return out;
  }

  /**
   * Word: code
   * Prototype: error
   *
   * Takes:
   * - error
   *
   * Gives:
   * - error
   * - number
   *
   * Returns error code extracted from the error in numeric form.
   */
  static void w_code(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type::error))
    {
      ctx->push(err);
      ctx->push_int(static_cast<number::int_type>(err.cast<error>()->code()));
    }
  }

  /**
   * Word: message
   * Prototype: error
   *
   * Takes:
   * - error
   *
   * Gives:
   * - error
   * - string|null
   *
   * Returns error message extracted from the error, or null if the error does
   * not have any error message.
   */
  static void w_message(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type::error))
    {
      const auto& message = err.cast<error>()->message();

      ctx->push(err);
      if (message.empty())
      {
        ctx->push_null();
      } else {
        ctx->push_string(message);
      }
    }
  }

  /**
   * Word: position
   * Prototype: error
   *
   * Takes
   * - error
   *
   * Gives:
   * - error
   * - object|null
   *
   * Returns position in the source code where the error occurred, or null if
   * no such information is available.
   *
   * Position is returned as object with `filename`, `line` and `column`
   * properties.
   */
  static void w_position(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type::error))
    {
      const auto position = err.cast<error>()->position();

      ctx->push(err);
      if (position)
      {
        const auto& runtime = ctx->runtime();

        ctx->push_object({
          { U"filename", runtime->string(position->filename) },
          { U"line", runtime->number(number::int_type(position->line)) },
          { U"column", runtime->number(number::int_type(position->column)) }
        });
      } else {
        ctx->push_null();
      }
    }
  }

  /**
   * Word: throw
   * Prototype: error
   *
   * Takes:
   * - error
   *
   * Sets given error as current error of the execution context.
   */
  static void w_throw(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type::error))
    {
      ctx->error(err.cast<error>());
    }
  }

  namespace api
  {
    runtime::prototype_definition error_prototype()
    {
      return
      {
        { U"code", w_code },
        { U"message", w_message },
        { U"position", w_position },
        { U"throw", w_throw },
      };
    }
  }
}

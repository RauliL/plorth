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
#include <plorth/value-symbol.hpp>

#include "./utils.hpp"

namespace plorth
{
  symbol::symbol(const unistring& id)
    : m_id(id) {}

  bool symbol::equals(const ref<value>& that) const
  {
    if (that && that->is(type_symbol))
    {
      return !m_id.compare(that.cast<symbol>()->m_id);
    } else {
      return false;
    }
  }

  bool symbol::exec(const ref<context>& ctx)
  {
    return ctx->call(m_id);
  }

  bool symbol::eval(const ref<context>& ctx, ref<value>& slot)
  {
    if (!m_id.compare(U"null"))
    {
      slot.release();

      return true;
    }
    else if (!m_id.compare(U"true"))
    {
      slot = ctx->runtime()->true_value();

      return true;
    }
    else if (!m_id.compare(U"false"))
    {
      slot = ctx->runtime()->false_value();

      return true;
    }
    else if (!m_id.compare(U"drop"))
    {
      return ctx->pop(slot);
    }
    else if (is_number(m_id))
    {
      slot = ctx->runtime()->number(m_id);

      return true;
    }
    ctx->error(
      error::code_syntax,
      U"Unexpected `" + m_id + U"'; Missing value."
    );

    return false;
  }

  unistring symbol::to_string() const
  {
    return to_source();
  }

  unistring symbol::to_source() const
  {
    return m_id;
  }
}

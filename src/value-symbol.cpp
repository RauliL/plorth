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
    ref<class value> value;

    // Look from prototype of the current item.
    {
      const auto& stack = ctx->data();

      if (!stack.empty() && stack.back())
      {
        const ref<object> prototype = stack.back()->prototype(ctx->runtime());

        if (prototype && prototype->property(ctx->runtime(), m_id, value))
        {
          if (value->is(value::type_quote))
          {
            return value.cast<quote>()->call(ctx);
          }
          ctx->push(value);

          return true;
        }
      }
    }

    // Look for a word from dictionary of current context.
    {
      const auto& local_dictionary = ctx->dictionary();
      const auto entry = local_dictionary.find(m_id);

      if (entry != std::end(local_dictionary))
      {
        value = entry->second;
        if (value && value->is(value::type_quote))
        {
          return value.cast<quote>()->call(ctx);
        }
        ctx->push(value);

        return true;
      }
    }

    // TODO: If not found, see if it's a "fully qualified" name, e.g. a name
    // with a namespace name, colon and a word - Such as "num:+", and then look
    // for that from the specified namespace.

    // Look from global dictionary.
    {
      const auto& global_dictionary = ctx->runtime()->dictionary();
      const auto entry = global_dictionary.find(m_id);

      if (entry != std::end(global_dictionary))
      {
        value = entry->second;
        if (value && value->is(value::type_quote))
        {
          return value.cast<quote>()->call(ctx);
        }
        ctx->push(value);

        return true;
      }
    }

    // If the name of the word can be converted into number, then do just that.
    if (is_number(m_id))
    {
      ctx->push_number(m_id);

      return true;
    }

    // Otherwise it's reference error.
    ctx->error(error::code_reference, U"Unrecognized word: `" + m_id + U"'");

    return false;
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

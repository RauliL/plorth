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
  symbol::symbol(const unistring& id, const struct position* position)
    : m_id(id)
    , m_position(position ? new struct position(*position) : nullptr)
    , m_hash(0) {}

  symbol::~symbol()
  {
    if (m_position)
    {
      delete m_position;
    }
  }

  std::size_t symbol::hash() const
  {
    std::size_t h = m_hash;

    if (h == 0)
    {
      symbol* sym = const_cast<symbol*>(this);
#if PLORTH_ENABLE_MUTEXES
      std::lock_guard<std::mutex> lock(sym->m_mutex);
#endif

      h = sym->m_hash = std::hash<unistring>()(sym->m_id);
    }

    return h;
  }

  bool symbol::equals(const ref<value>& that) const
  {
    if (that && that->is(type_symbol))
    {
      return !m_id.compare(that.cast<symbol>()->m_id);
    } else {
      return false;
    }
  }

  bool symbol::exec(const std::shared_ptr<context>& ctx)
  {
    // Update source code position of the context, if this symbol has such
    // information.
    if (m_position)
    {
      ctx->position() = *m_position;
    }

    // Look from prototype of the current item.
    {
      const auto& stack = ctx->data();

      if (!stack.empty() && stack.back())
      {
        const ref<object> prototype = stack.back()->prototype(ctx->runtime());
        ref<value> val;

        if (prototype && prototype->property(ctx->runtime(), m_id, val))
        {
          if (val && val->is(value::type_quote))
          {
            return val.cast<quote>()->call(ctx);
          }
          ctx->push(val);

          return true;
        }
      }
    }

    // Look for a word from dictionary of current context.
    {
      const auto& local_dictionary = ctx->dictionary();
      const auto entry = local_dictionary.find(this);

      if (entry != std::end(local_dictionary) && entry->second)
      {
        return entry->second->call(ctx);
      }
    }

    // TODO: If not found, see if it's a "fully qualified" name, e.g. a name
    // with a namespace name, colon and a word - Such as "num:+", and then look
    // for that from the specified namespace.

    // Look from global dictionary.
    {
      const auto& global_dictionary = ctx->runtime()->dictionary();
      const auto entry = global_dictionary.find(this);

      if (entry != std::end(global_dictionary) && entry->second)
      {
        return entry->second->call(ctx);
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

  bool symbol::eval(const std::shared_ptr<context>& ctx, ref<value>& slot)
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

  ref<class symbol> runtime::symbol(const unistring& id,
                                    const struct position* position)
  {
#if PLORTH_ENABLE_SYMBOL_CACHE
    const auto entry = m_symbol_cache.find(id);

    if (entry == std::end(m_symbol_cache))
    {
      const ref<class symbol> reference = new (*m_memory_manager) class symbol(id);

      m_symbol_cache[id] = reference;

      return reference;
    }

    return entry->second;
#else
    return new (*m_memory_manager) class symbol(id, position);
#endif
  }

  /**
   * Word: position
   * Prototype: symbol
   *
   * Takes:
   * - symbol
   *
   * Gives:
   * - symbol
   * - object|null
   *
   * Returns position in source code where the symbols was encountered, or null
   * if no such information is available. If symbol caching has been enabled in
   * the interpreter, source code position is not stored in symbols.
   *
   * Position is returnedd as object with `filename`, `line` and `column`
   * properties.
   */
  static void w_position(const std::shared_ptr<context>& ctx)
  {
    ref<value> sym;

    if (ctx->pop(sym, value::type_symbol))
    {
      const auto position = sym.cast<symbol>()->position();

      ctx->push(sym);
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
   * Word: call
   * Prototype: symbol
   *
   * Takes:
   * - symbol
   *
   * Resolves given symbol into word or value, depending on the contents of the
   * data stack, local dictionary and global dictionary and executes it. If the
   * symbol does not resolve into any kind of word or value, number conversion
   * is attempted on it. If that also fails, reference error will be thrown.
   */
  static void w_call(const std::shared_ptr<context>& ctx)
  {
    ref<symbol> sym;

    if (ctx->pop_symbol(sym))
    {
      sym->exec(ctx);
    }
  }

  namespace api
  {
    runtime::prototype_definition symbol_prototype()
    {
      return
      {
        { U"position", w_position },
        { U"call", w_call }
      };
    }
  }
}

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
  symbol::symbol(const std::u32string& id, const struct position* position)
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

      h = sym->m_hash = std::hash<std::u32string>()(sym->m_id);
    }

    return h;
  }

  bool symbol::equals(const std::shared_ptr<value>& that) const
  {
    if (is(that, type::symbol))
    {
      return !m_id.compare(std::static_pointer_cast<symbol>(that)->m_id);
    } else {
      return false;
    }
  }

  std::u32string symbol::to_string() const
  {
    return to_source();
  }

  std::u32string symbol::to_source() const
  {
    return m_id;
  }

  std::shared_ptr<class symbol> runtime::symbol(const std::u32string& id,
                                    const struct position* position)
  {
#if PLORTH_ENABLE_SYMBOL_CACHE
    const auto entry = m_symbol_cache.find(id);

    if (entry == std::end(m_symbol_cache))
    {
      const auto reference = std::shared_ptr<class symbol>(
        new (*m_memory_manager) class symbol(id)
      );

      m_symbol_cache[id] = reference;

      return reference;
    }

    return entry->second;
#else
    return std::shared_ptr<class symbol>(
      new (*m_memory_manager) class symbol(id, position)
    );
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
    std::shared_ptr<value> sym;

    if (ctx->pop(sym, value::type::symbol))
    {
      const auto position = std::static_pointer_cast<symbol>(sym)->position();

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
    std::shared_ptr<symbol> sym;

    if (ctx->pop_symbol(sym))
    {
      value::exec(ctx, sym);
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

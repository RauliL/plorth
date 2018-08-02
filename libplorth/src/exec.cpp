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
#include <plorth/value-word.hpp>
#include "./utils.hpp"

namespace plorth
{
  static bool exec_val(const std::shared_ptr<context>&,
                       const std::shared_ptr<value>&);
  static bool exec_sym(const std::shared_ptr<context>&,
                       const std::shared_ptr<symbol>&);
  static bool exec_wrd(const std::shared_ptr<context>&,
                       const std::shared_ptr<word>&);

  bool value::exec(const std::shared_ptr<context>& ctx,
                   const std::shared_ptr<value>& val)
  {
    if (!val)
    {
      ctx->push_null();

      return true;
    }
    switch (val->type())
    {
      case value::type::symbol:
        return exec_sym(ctx, std::static_pointer_cast<symbol>(val));

      case value::type::word:
        return exec_wrd(ctx, std::static_pointer_cast<word>(val));

      default:
        return exec_val(ctx, val);
    }
  }

  static bool exec_val(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<value>& val)
  {
    std::shared_ptr<value> slot;

    if (!value::eval(ctx, val, slot))
    {
      return false;
    }
    ctx->push(slot);

    return true;
  }

  static bool exec_sym(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<symbol>& sym)
  {
    const auto position = sym->position();
    const auto id = sym->id();

    // Update source code position of the context, if the symbol has such
    // information.
    if (position)
    {
      ctx->position() = *position;
    }

    // Look for prototype of the current item.
    {
      const auto& stack = ctx->data();

      if (!stack.empty() && stack.back())
      {
        const auto prototype = stack.back()->prototype(ctx->runtime());
        std::shared_ptr<value> val;

        if (prototype && prototype->property(ctx->runtime(), id, val))
        {
          if (value::is(val, value::type::quote))
          {
            return std::static_pointer_cast<quote>(val)->call(ctx);
          }
          ctx->push(val);

          return true;
        }
      }
    }

    // Look for a word from dictionary of current context.
    if (auto word = ctx->dictionary().find(sym))
    {
      return word->quote()->call(ctx);
    }

    // TODO: If not found, see if it's a "fully qualified" name, e.g. a name
    // with a namespace name, colon and a word - Such as "num:+", and then look
    // for that from the specified namespace.

    // Look from global dictionary.
    if (auto word = ctx->runtime()->dictionary().find(sym))
    {
      return word->quote()->call(ctx);
    }

    // If the name of the word can be converted into number, then do just that.
    if (is_number(id))
    {
      ctx->push_number(id);

      return true;
    }

    // Otherwise it's reference error.
    ctx->error(error::code::reference, U"Unrecognized word: `" + id + U"'");

    return false;
  }

  static bool exec_wrd(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<word>& wrd)
  {
    ctx->dictionary().insert(wrd);

    return true;
  }
}

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

#include "../../libplorth/src/utils.hpp"

namespace plorth
{
  namespace cli
  {
    /**
     * .q ( -- )
     *
     * Exits the interpreter.
     */
    static void w_quit(const ref<context>&)
    {
      std::exit(EXIT_SUCCESS);
    }

    /**
     * .s ( -- )
     *
     * Displays ten of the top-most values from the data stack.
     */
    static void w_stack(const ref<context>& ctx)
    {
      const auto& runtime = ctx->runtime();
      const auto& stack = ctx->data();
      const std::size_t size = stack.size();

      if (!size)
      {
        runtime->println(U"Stack is empty.");
        return;
      }

      for (std::size_t i = 0; i < size && i < 10; ++i)
      {
        const auto& value = stack[size - i - 1];

        runtime->print(
          to_unistring(static_cast<number::int_type>(size - i)) + U": "
        );
        runtime->print(value ? value->to_source() : U"null");
        runtime->println();
      }
    }

    void initialize_repl_api(const ref<runtime>& runtime)
    {
      auto& dictionary = runtime->dictionary();

      dictionary.insert(runtime->word(
        runtime->symbol(U".q"),
        runtime->native_quote(w_quit)
      ));
      dictionary.insert(runtime->word(
        runtime->symbol(U".s"),
        runtime->native_quote(w_stack)
      ));
    }
  }
}

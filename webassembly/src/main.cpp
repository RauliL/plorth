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
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace plorth;

static memory::manager* memory_manager;
static std::shared_ptr<runtime> plorth_runtime;
static std::shared_ptr<context> plorth_context;

static void plorth_initialize();

void initialize_repl_api(const std::shared_ptr<runtime>&);

/**
 * Compiles and executes given source code.
 */
static void plorth_execute(const std::wstring& source)
{
  std::shared_ptr<quote> script;

  if (!plorth_context)
  {
    plorth_initialize();
  }

  script = plorth_context->compile(utf32le_decode(source));
  if (!script || !script->call(plorth_context))
  {
    const auto err = plorth_context->error();
    std::wstring message;

    plorth_context->clear_error();
    if (err)
    {
      message = utf32le_encode(err->to_string());
    } else {
      message = L"Unknown error.";
    }
    EM_ASM_({ throw new Error(UTF32ToString($0)); }, message.c_str());
  }
}

/**
 * Returns the number of values in the stack.
 */
static int plorth_depth()
{
  return plorth_context ? plorth_context->size() : 0;
}

/**
 * Extracts source code representation of value from the stack, based on it's
 * index on the stack.
 */
static std::wstring plorth_stack(int index)
{
  std::wstring result;

  if (plorth_context)
  {
    const auto& stack = plorth_context->data();
    const auto size = stack.size();

    if (index >= 0 && index < size)
    {
      const auto& value = stack[size - index - 1];

      if (value)
      {
        result = utf32le_encode(value->to_source());
      } else {
        result = L"null";
      }
    } else {
      EM_ASM({ throw new Error('Stack index out of bounds.'); });
    }
  } else {
    EM_ASM({ throw new Error('Plorth runtime not initialized.'); });
  }

  return result;
}

EMSCRIPTEN_BINDINGS(plorth)
{
  emscripten::constant("VERSION", utf32le_encode(PLORTH_VERSION));

  emscripten::function("execute", &plorth_execute);
  emscripten::function("depth", &plorth_depth);
  emscripten::function("stack", &plorth_stack);
}

static void plorth_initialize()
{
  if (plorth_context)
  {
    return;
  }
  memory_manager = new memory::manager();
  plorth_runtime = runtime::make(*memory_manager);
  plorth_context = context::make(plorth_runtime);
  initialize_repl_api(plorth_runtime);
}

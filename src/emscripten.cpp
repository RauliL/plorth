#include <plorth/context.hpp>
#include <emscripten/bind.h>

using namespace plorth;

static memory::manager* memory_manager;
static ref<runtime> script_runtime;
static ref<context> script_context;

void initialize_repl_api(const ref<runtime>&);

/**
 * Returns depth of the data stack.
 */
static int emscripten_depth()
{
  return script_context ? script_context->size() : 0;
}

/**
 * Compiles and executes given source code under the REPL context.
 */
static void emscripten_execute(const std::string& source)
{
  ref<class quote> quote;

  if (!script_context)
  {
    memory_manager = new memory::manager();
    script_runtime = memory_manager->new_runtime();
    script_context = script_runtime->new_context();
    initialize_repl_api(script_runtime);
  }

  if ((quote = script_context->compile(source)))
  {
    quote->call(script_context);
  }

  if (script_context->error())
  {
    std::cerr << script_context->error() << std::endl;
    script_context->clear_error();
  }
}

EMSCRIPTEN_BINDINGS(plorth)
{
  emscripten::function("depth", &emscripten_depth);
  emscripten::function("execute", &emscripten_execute);
}

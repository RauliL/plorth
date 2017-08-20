#include <emscripten/bind.h>

using namespace plorth;

static memory::manager* memory_manager;
static ref<class runtime> runtime;
static ref<class context> context;

void initialize_repl_api(const ref<class runtime>&);

static int emscripten_depth()
{
  return context ? context->size() : 0;
}

static void emscripten_execute(const std::string& source)
{
  if (!context)
  {
    memory_manager = new memory::manager();
    runtime = memory_manager->new_runtime();
    context = runtime->new_context();
    initialize_repl_api(runtime);
  }
}

EMSCRIPTEN_BINDINGS(plorth)
{
  emscripten::function("depth", &emscripten_depth);
  emscripten::function("execute", &emscripten_execute);
}

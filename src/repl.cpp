#include <plorth/context.hpp>

#include "./utils.hpp"

using namespace plorth;

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

    runtime->print(to_unistring(static_cast<number::int_type>(size - i)) + U": ");
    runtime->print(value ? value->to_source() : U"null");
    runtime->println();
  }
}

void initialize_repl_api(const ref<runtime>& runtime)
{
  auto& dictionary = runtime->dictionary();

  dictionary[U".q"] = runtime->native_quote(w_quit);
  dictionary[U".s"] = runtime->native_quote(w_stack);
}

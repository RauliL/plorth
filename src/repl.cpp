#include <plorth/context.hpp>

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
static void w_stack(const ref<class context>& context)
{
  const auto& stack = context->data();
  const std::size_t size = stack.size();

  if (!size)
  {
    std::cout << "Stack is empty." << std::endl;
    return;
  }

  for (std::size_t i = 0; i < size && i < 10; ++i)
  {
    const ref<class value>& value = stack[size - i - 1];
    std::cout << (size - i) << ": ";
    if (value)
    {
      std::cout << value->to_source();
    } else {
      std::cout << "null";
    }
    std::cout << std::endl;
  }
}

void initialize_repl_api(const ref<runtime>& runtime)
{
  auto& dictionary = runtime->dictionary();

  dictionary[utf8_decode(".q")] = runtime->native_quote(w_quit);
  dictionary[utf8_decode(".s")] = runtime->native_quote(w_stack);
}

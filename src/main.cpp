#include <plorth/context.hpp>
#include <plorth/value-quote.hpp>

#include <fstream>
#include <stack>

#if defined(PLORTH_HAVE_UNISTD_H)
# include <unistd.h>
#endif

using namespace plorth;

static inline bool is_console_interactive();
static void compile_and_run(const ref<context>&, const std::string&);
static void console_loop(const ref<context>&);

int main(int argc, char** argv)
{
  memory::manager memory_manager;
  ref<runtime> runtime = memory_manager.new_runtime();
  ref<context> context = runtime->new_context();

  if (argc > 1)
  {
    for (int i = 1; i < argc; ++i)
    {
      std::fstream is(argv[i], std::ios_base::in);

      if (is.good())
      {
        const std::string source = std::string(
          std::istreambuf_iterator<char>(is),
          std::istreambuf_iterator<char>()
        );

        is.close();
        context->clear();
        compile_and_run(context, source);
      } else {
        std::cerr << "unable to open file `" << argv[i] << "' for reading" << std::endl;
        std::exit(EXIT_FAILURE);
      }
    }
  }
  else if (is_console_interactive())
  {
    console_loop(context);
  } else {
    compile_and_run(
      context,
      std::string(
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>()
      )
    );
  }

  return EXIT_SUCCESS;
}

static inline bool is_console_interactive()
{
#if defined(PLORTH_HAVE_ISATTY)
  return isatty(fileno(stdin));
#else
  return false;
#endif
}

static void compile_and_run(const ref<class context>& context, const std::string& source)
{
  const ref<class quote> quote = context->compile(source);

  if (!quote || !quote->call(context))
  {
    const ref<class error>& error = context->error();

    if (!error)
    {
      std::exit(EXIT_FAILURE);
    }

    std::cerr << "Error: "
              << error->code()
              << " - "
              << error->message()
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

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
    std::cout << (size - i)
              << ": "
              << stack[size - i - 1]->to_source()
              << std::endl;
  }
}

static void initialize_console_api(const ref<class runtime>& runtime)
{
  auto& dictionary = runtime->dictionary();

  dictionary[utf8_decode(".q")] = runtime->quote(w_quit);
  dictionary[utf8_decode(".s")] = runtime->quote(w_stack);
}

static void count_open_braces(const std::string& input, std::stack<char>& open_braces)
{
  const std::size_t length = input.length();

  for (std::size_t i = 0; i < length; ++i)
  {
    switch (input[i])
    {
      case '#':
        return;

      case '(':
        open_braces.push(')');
        break;

      case '[':
        open_braces.push(']');
        break;

      case '{':
        open_braces.push('}');
        break;

      case ')':
      case ']':
      case '}':
        if (!open_braces.empty() && open_braces.top() == input[i])
        {
          open_braces.pop();
        }
        break;

      case '"':
        while (i < length)
        {
          if (input[i] == '"')
          {
            break;
          }
          else if (input[i] == '\\' && i + 1 < length)
          {
            i += 2;
          } else {
            ++i;
          }
        }
    }
  }
}

static void console_loop(const ref<class context>& context)
{
  int line_counter = 0;
  std::string source;
  std::stack<char> open_braces;

  initialize_console_api(context->runtime());

  while (std::cin.good())
  {
    std::string line;

    std::cout << "plorth:"
              << ++line_counter
              << ':'
              << context->size()
              << (open_braces.empty() ? '>' : '*')
              << ' ';
    if (!std::getline(std::cin, line))
    {
      break;
    }
    else if (!line.empty())
    {
      count_open_braces(line, open_braces);
      source.append(line).append(1, '\n');
      if (open_braces.empty())
      {
        const ref<class quote> quote = context->compile(source);

        source.clear();
        if (quote)
        {
          quote->call(context);
        }
        if (context->error())
        {
          std::cout << context->error() << std::endl;
          context->clear_error();
        }
      }
    }
  }
}

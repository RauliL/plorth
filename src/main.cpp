#include <fstream>

#include "context.hpp"

#if defined(PLORTH_HAVE_UNISTD_H)
# include <unistd.h>
#endif

using namespace plorth;

static bool is_console_interactive();
static void compile_and_run(const Ref<Context>&, const std::string&);

namespace plorth
{
  void console_loop(const Ref<Context>&);
}

int main(int argc, char** argv)
{
  MemoryManager memory_manager;
  const Ref<Runtime> runtime = memory_manager.CreateRuntime();
  const Ref<Context> context = new (runtime) Context(runtime);

  if (argc > 1)
  {
    for (int i = 1; i < argc; ++i)
    {
      std::fstream is(argv[i], std::ios_base::in);

      if (is.good())
      {
        compile_and_run(
          context,
          std::string(
            std::istreambuf_iterator<char>(is),
            std::istreambuf_iterator<char>()
          )
        );
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

static bool is_console_interactive()
{
#if defined(PLORTH_HAVE_ISATTY)
  return isatty(fileno(stdin));
#else
  return false;
#endif
}

static void compile_and_run(const Ref<Context>& context,
                            const std::string& source)
{
  const Ref<Quote> quote = Quote::Compile(context, source);

  if (!quote || !quote->Call(context))
  {
    const Ref<Error>& error = context->GetError();

    std::cout << "Error: "
              << error->GetCode()
              << " - "
              << error->GetMessage()
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

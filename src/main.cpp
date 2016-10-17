#include <fstream>

#include "state.hpp"

#if defined(PLORTH_HAVE_UNISTD_H)
# include <unistd.h>
#endif

using namespace plorth;

static bool is_console_interactive();

namespace plorth
{
  void console_loop(const Ref<State>&);
}

int main(int argc, char** argv)
{
  MemoryManager memory_manager;
  const Ref<Runtime> runtime = memory_manager.CreateRuntime();
  const Ref<State> state = runtime->CreateState();

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
        const Ref<Quote> quote = Quote::Compile(state, source);

        if (!quote || !quote->Call(state))
        {
          std::cout << "Error: "
                    << state->GetError()->GetMessage()
                    << std::endl;
          std::exit(EXIT_FAILURE);
        }
      } else {
        std::cerr << "unable to open file `" << argv[i] << "' for reading" << std::endl;
        std::exit(EXIT_FAILURE);
      }
    }
  }
  else if (is_console_interactive())
  {
    console_loop(state);
  } else {
    const std::string source = std::string(
      std::istreambuf_iterator<char>(std::cin),
      std::istreambuf_iterator<char>()
    );
    const Ref<Quote> quote = Quote::Compile(state, source);

    if (!quote || !quote->Call(state))
    {
      std::cout << "Error: "
                << state->GetError()->GetMessage()
                << std::endl;
      std::exit(EXIT_FAILURE);
    }
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

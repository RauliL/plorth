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
#include <plorth/plorth.hpp>

#include <cstring>
#include <fstream>
#include <stack>
#if PLORTH_ENABLE_MODULES
# include <unordered_set>
#endif

#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(HAVE_SYSEXITS_H)
# include <sysexits.h>
#endif

#if !defined(EX_USAGE)
# define EX_USAGE 64
#endif

using namespace plorth;

static const char* script_filename = nullptr;
static bool flag_test_syntax = false;
static bool flag_fork = false;
#if PLORTH_ENABLE_MODULES
static std::unordered_set<unistring> imported_modules;
#endif

static void scan_arguments(const ref<runtime>&, int, char**);
#if PLORTH_ENABLE_MODULES
static void scan_module_path(const ref<runtime>&);
#endif
static inline bool is_console_interactive();
static void compile_and_run(const ref<context>&,
                            const std::string&,
                            const unistring&);
static void console_loop(const ref<context>&);
static void handle_error(const ref<context>&);

void initialize_repl_api(const ref<runtime>&);

int main(int argc, char** argv)
{
  memory::manager memory_manager;
  auto runtime = runtime::make(memory_manager);
  auto context = context::make(runtime);

#if PLORTH_ENABLE_MODULES
  scan_module_path(runtime);
#endif

  scan_arguments(runtime, argc, argv);

#if PLORTH_ENABLE_MODULES
  for (const auto& module_path : imported_modules)
  {
    if (!runtime->import(context, module_path))
    {
      handle_error(context);
    }
  }
#endif

  if (script_filename)
  {
    const unistring decoded_script_filename = utf8_decode(script_filename);
    std::ifstream is(script_filename, std::ios_base::in);

    if (is.good())
    {
      const std::string source = std::string(
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()
        );

      is.close();
      context->clear();
#if PLORTH_ENABLE_MODULES
      context->filename(decoded_script_filename);
#endif
      compile_and_run(context, source, decoded_script_filename);
    } else {
      std::cerr << argv[0]
                << ": Unable to open file `"
                << script_filename
                << "' for reading."
                << std::endl;
      std::exit(EXIT_FAILURE);
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
      ),
      U"<stdin>"
    );
  }

  return EXIT_SUCCESS;
}

static void print_usage(std::ostream& out, const char* executable)
{
  out << std::endl
      << "Usage: "
      << executable
      << " [switches] [--] [programfile] [arguments]"
      << std::endl;
  out << "  -c        Check syntax only." << std::endl;
#if HAVE_FORK
  out << "  -f        Fork to background before executing script." << std::endl;
#endif
#if PLORTH_ENABLE_MODULES
  out << "  -r <path> Import module before executing script." << std::endl;
#endif
  out << "  --version Print the version." << std::endl;
  out << "  --help    Display this message." << std::endl;
  out << std::endl;
}

static void scan_arguments(const ref<class runtime>& runtime,
                           int argc,
                           char** argv)
{
  int offset = 1;

  while (offset < argc)
  {
    const char* arg = argv[offset++];

    if (!*arg)
    {
      continue;
    }
    else if (*arg != '-')
    {
      script_filename = arg;
      break;
    }
    else if (!arg[1])
    {
      break;
    }
    else if (arg[1] == '-')
    {
      if (!std::strcmp(arg, "--help"))
      {
        print_usage(std::cout, argv[0]);
        std::exit(EXIT_SUCCESS);
      }
      else if (!std::strcmp(arg, "--version"))
      {
        std::cerr << "Plorth " << utf8_encode(PLORTH_VERSION) << std::endl;
        std::exit(EXIT_SUCCESS);
      }
      else if (!std::strcmp(arg, "--"))
      {
        if (offset < argc)
        {
          script_filename = argv[offset++];
        }
        break;
      } else {
        std::cerr << "Unrecognized switch: " << arg << std::endl;
        print_usage(std::cerr, argv[0]);
        std::exit(EX_USAGE);
      }
    }
    for (int i = 1; arg[i]; ++i)
    {
      // TODO: Add support for these command line switches:
      // -e: Compile and execute inline script.
      switch (arg[i])
      {
      case 'c':
        flag_test_syntax = true;
        break;

      case 'f':
        flag_fork = true;
        break;

#if PLORTH_ENABLE_MODULES
      case 'r':
        if (offset < argc)
        {
          unistring module_path;

          if (!utf8_decode_test(argv[offset++], module_path))
          {
            std::cerr << "Unable to decode given module path." << std::endl;
            std::exit(EXIT_FAILURE);
          }
          imported_modules.insert(module_path);
          ++offset;
        } else {
          std::cerr << "Argument expected for the -r option." << std::endl;
          print_usage(std::cerr, argv[0]);
          std::exit(EX_USAGE);
        }
        break;
#else
      case 'r':
        std::cerr << "Modules have been disabled." << std::endl;
        std::exit(EXIT_FAILURE);
        break;
#endif

      case 'h':
        print_usage(std::cout, argv[0]);
        std::exit(EXIT_SUCCESS);
        break;

      default:
        std::cerr << "Unrecognized switch: `" << arg[i] << "'" << std::endl;
        print_usage(std::cerr, argv[0]);
        std::exit(EX_USAGE);
      }
    }
  }

  while (offset < argc)
  {
    runtime->arguments().push_back(utf8_decode(argv[offset++]));
  }
}

#if PLORTH_ENABLE_MODULES
static void scan_module_path(const ref<class runtime>& runtime)
{
#if defined(_WIN32)
  static const unichar path_separator = ';';
#else
  static const unichar path_separator = ':';
#endif
  auto& module_paths = runtime->module_paths();
  const char* begin = std::getenv("PLORTHPATH");
  const char* end = begin;

  if (end)
  {
    for (; *end; ++end)
    {
      if (*end != path_separator)
      {
        continue;
      }

      if (end - begin > 0)
      {
        module_paths.push_back(utf8_decode(std::string(begin, end - begin)));
      }
      begin = end + 1;
    }

    if (end - begin > 0)
    {
      module_paths.push_back(utf8_decode(std::string(begin, end - begin)));
    }
  }

#if defined(PLORTH_RUNTIME_LIBRARY_PATH)
  if (module_paths.empty())
  {
    module_paths.push_back(PLORTH_RUNTIME_LIBRARY_PATH);
  }
#endif
}
#endif

static inline bool is_console_interactive()
{
#if defined(HAVE_ISATTY)
  return isatty(fileno(stdin));
#else
  return false;
#endif
}

static void handle_error(const ref<context>& ctx)
{
  const ref<error>& err = ctx->error();

  if (err)
  {
    const auto position = err->position();

    std::cerr << "Error: ";
    if (position && (!position->filename.empty() || position->line))
    {
      std::cerr << *position << ':';
    }
    std::cerr << err->code() << " - " << err->message();
  } else {
    std::cerr << "Unknown error.";
  }
  std::cerr << std::endl;
  std::exit(EXIT_FAILURE);
}

static void compile_and_run(const ref<context>& ctx,
                            const std::string& input,
                            const unistring& filename)
{
  unistring source;
  ref<quote> script;

  if (!utf8_decode_test(input, source))
  {
    std::cerr << "Import error: Unable to decode source code as UTF-8." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if (!(script = ctx->compile(source, filename)))
  {
    handle_error(ctx);
    return;
  }

  if (flag_test_syntax)
  {
    std::cerr << "Syntax OK." << std::endl;
    std::exit(EXIT_SUCCESS);
    return;
  }

  if (flag_fork)
  {
#if HAVE_FORK
    if (fork())
    {
      std::exit(EXIT_SUCCESS);
    }
#else
    std::cerr << "Forking to background is not supported on this platform." << std::endl;
#endif
  }

  if (!script->call(ctx))
  {
    handle_error(ctx);
  }
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
  unistring source;
  std::stack<char> open_braces;

  initialize_repl_api(context->runtime());

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
      // Output final newline after ^D has been pressed by the user.
      std::cout << std::endl;
      break;
    }
    else if (!line.empty())
    {
      if (!utf8_decode_test(line, source))
      {
        std::cout << "Unable to decode given input as UTF-8." << std::endl;
        continue;
      }
      source.append(1, '\n');
      count_open_braces(line, open_braces);
      if (open_braces.empty())
      {
        const auto script = context->compile(source, U"<repl>", line_counter);

        source.clear();
        if (script)
        {
          script->call(context);
        }
        if (context->error())
        {
          const auto& error = context->error();

          if (error->position())
          {
            std::cout << *error->position() << ':';
          }
          std::cout << error << std::endl;
          context->clear_error();
        }
      }
    }
  }
}

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
#include <plorth/cli/config.hpp>

#include <cstring>
#include <fstream>
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
# include <unordered_set>
#endif

#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(HAVE_SYSEXITS_H)
# include <sysexits.h>
#endif

#include "./utils.hpp"

#if !defined(EX_USAGE)
# define EX_USAGE 64
#endif

using namespace plorth;

static const char* script_filename = nullptr;
static bool flag_test_syntax = false;
static bool flag_fork = false;
static std::string inline_script;
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
static std::unordered_set<std::u32string> imported_modules;
#endif

static void scan_arguments(const std::shared_ptr<runtime>&, int, char**);
static void compile_and_run(const std::shared_ptr<context>&,
                            const std::string&,
                            const std::u32string&);
static void handle_error(const std::shared_ptr<context>&);

#if PLORTH_CLI_ENABLE_REPL
static inline bool is_console_interactive();

namespace plorth
{
  namespace cli
  {
    void repl_loop(const std::shared_ptr<context>&);
  }
}
#endif

int main(int argc, char** argv)
{
  memory::manager memory_manager;
  auto runtime = runtime::make(memory_manager);
  auto context = context::make(runtime);

#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
  plorth::cli::utils::scan_module_path(runtime);
#endif

  scan_arguments(runtime, argc, argv);

#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
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
    const auto decoded_script_filename = utf8_decode(script_filename);
    std::ifstream is(script_filename, std::ios_base::in);

    if (is.good())
    {
      const std::string source = std::string(
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()
      );

      is.close();
      context->clear();
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
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
  else if (!inline_script.empty())
  {
    compile_and_run(context, inline_script, U"-e");
#if PLORTH_CLI_ENABLE_REPL
  }
  else if (is_console_interactive())
  {
    plorth::cli::repl_loop(context);
#endif
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
  out << "  -c           Check syntax only." << std::endl;
#if HAVE_FORK
  out << "  -f           Fork to background before executing script."
      << std::endl;
#endif
  out << "  -e <program> One line of program. (Several -e's allowed, omit "
      << "programfile.)"
      << std::endl;
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
  out << "  -r <path>    Import module before executing script." << std::endl;
#endif
  out << "  --version    Print the version." << std::endl;
  out << "  --help       Display this message." << std::endl;
  out << std::endl;
}

static void scan_arguments(const std::shared_ptr<class runtime>& runtime,
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
      if (inline_script.empty())
      {
        script_filename = arg;
      }
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
      switch (arg[i])
      {
      case 'c':
        flag_test_syntax = true;
        break;

      case 'e':
        if (offset < argc)
        {
          inline_script.append(argv[offset++]);
          inline_script.append('\n', 1);
        } else {
          std::cerr << "Argument expected for the -e option." << std::endl;
          print_usage(std::cerr, argv[0]);
          std::exit(EX_USAGE);
        }
        break;

      case 'f':
        flag_fork = true;
        break;

#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
      case 'r':
        if (offset < argc)
        {
          std::u32string module_path;

          if (!utf8_decode_test(argv[offset++], module_path))
          {
            std::cerr << "Unable to decode given module path." << std::endl;
            std::exit(EXIT_FAILURE);
          }
          imported_modules.insert(module_path);
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

#if PLORTH_CLI_ENABLE_REPL
static inline bool is_console_interactive()
{
#if defined(HAVE_ISATTY)
  return isatty(fileno(stdin));
#else
  return false;
#endif
}
#endif

static void handle_error(const std::shared_ptr<context>& ctx)
{
  const std::shared_ptr<error>& err = ctx->error();

  if (err)
  {
    const auto position = err->position();

    std::cerr << "Error: ";
    if (position && (!position->filename.empty() || position->line))
    {
      std::cerr << *position << ':';
    }
    std::cerr << err->code() << " - " << utf8_encode(err->message());
  } else {
    std::cerr << "Unknown error.";
  }
  std::cerr << std::endl;
  std::exit(EXIT_FAILURE);
}

static void compile_and_run(const std::shared_ptr<context>& ctx,
                            const std::string& input,
                            const std::u32string& filename)
{
  std::u32string source;
  std::shared_ptr<quote> script;

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

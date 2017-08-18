/*
 * Copyright (c) 2017, Rauli Laine
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
#include <plorth/value-quote.hpp>

#include <fstream>
#include <stack>

#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

using namespace plorth;

#if PLORTH_ENABLE_MODULES
static void scan_module_path(const ref<runtime>&);
#endif
static inline bool is_console_interactive();
static void compile_and_run(const ref<context>&, const std::string&);
static void console_loop(const ref<context>&);

int main(int argc, char** argv)
{
  memory::manager memory_manager;
  ref<runtime> runtime = memory_manager.new_runtime();
  ref<context> context = runtime->new_context();

#if PLORTH_ENABLE_MODULES
  scan_module_path(runtime);
#endif

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

  if (!end)
  {
    return;
  }

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
#endif

static inline bool is_console_interactive()
{
#if defined(HAVE_ISATTY)
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

static void initialize_console_api(const ref<class runtime>& runtime)
{
  auto& dictionary = runtime->dictionary();

  dictionary[utf8_decode(".q")] = runtime->native_quote(w_quit);
  dictionary[utf8_decode(".s")] = runtime->native_quote(w_stack);
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
      // Output final newline after ^D has been pressed by the user.
      std::cout << std::endl;
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

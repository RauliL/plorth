#include <stack>

#include "state.hpp"

namespace plorth
{
  static void api_init_console(const Ref<Runtime>&);
  static void count_open_braces(const std::string&, std::stack<char>&);

  void console_loop(const Ref<State>& state)
  {
    int line_counter = 0;
    std::string source;
    std::stack<char> open_braces;

    api_init_console(state->GetRuntime());
    while (std::cin.good())
    {
      std::string line;

      std::cout << "plorth:"
                << ++line_counter
                << ":"
                << state->GetDataStack().size()
                << "> ";
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
          const Ref<Quote> quote = Quote::Compile(state, source);

          source.clear();
          if (quote)
          {
            quote->Call(state);
          }
          if (state->GetError())
          {
            std::cout << state->GetError()->GetMessage() << std::endl;
            state->ClearError();
          }
        }
      }
    }
  }

  static void count_open_braces(const std::string& input,
                                std::stack<char>& open_braces)
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

  /**
   * .q ( -- )
   *
   * Exits the interpreter.
   */
  static void w_quit(const Ref<State>& state)
  {
    std::exit(EXIT_SUCCESS);
  }

  /**
   * .s ( -- )
   *
   * Outputs contents of the stack.
   */
  static void w_stack(const Ref<State>& state)
  {
    const std::vector<Ref<Value>>& stack = state->GetDataStack();
    const std::size_t size = stack.size();

    if (stack.empty())
    {
      std::cout << "Stack is empty." << std::endl;
    } else {
      for (std::size_t i = 0; i < size && i < 10; ++i)
      {
        std::cout << (size - i)
                  << ": "
                  << stack[size - i - 1]->ToSource()
                  << std::endl;
      }
    }
  }

  static void api_init_console(const Ref<Runtime>& runtime)
  {
    runtime->AddWord(".q", w_quit);
    runtime->AddWord(".s", w_stack);
  }
}

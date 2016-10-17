#include "state.hpp"
#include "unicode.hpp"

namespace plorth
{
  /**
   * print ( any -- )
   *
   * Prints top-most value of the stack to stdout.
   */
  static void w_print(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Pop(value))
    {
      std::cout << value;
    }
  }

  /**
   * println ( any -- )
   *
   * Prints top-most value of the stack to stdout with terminating new line.
   */
  static void w_println(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Pop(value))
    {
      std::cout << value << std::endl;
    }
  }

  /**
   * . ( any -- )
   *
   * Pretty prints value from top of the stack into standard output stream.
   */
  static void w_dot(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Pop(value))
    {
      std::cout << value->ToSource() << std::endl;
    }
  }

  /**
   * bl ( -- )
   *
   * Outputs whitespace into the standard output stream.
   */
  static void w_bl(const Ref<State>& state)
  {
    std::cout << ' ';
  }

  /**
   * nl ( -- )
   *
   * Outputs new line into the standard output stream.
   */
  static void w_nl(const Ref<State>& state)
  {
    std::cout << std::endl;
  }

  /**
   * emit ( num -- )
   *
   * Outputs given Unicode code point into the standard output stream.
   */
  static void w_emit(const Ref<State>& state)
  {
    Ref<Number> value;

    if (state->PopNumber(value))
    {
      char buffer[5];

      if (unicode_encode(value->AsInt(), buffer))
      {
        std::cout << buffer;
      }
    }
  }

  void api_init_io(Runtime* runtime)
  {
    runtime->AddWord("print", w_print);
    runtime->AddWord("println", w_println);
    runtime->AddWord(".", w_dot);
    runtime->AddWord("bl", w_bl);
    runtime->AddWord("nl", w_nl);
    runtime->AddWord("emit", w_emit);
  }
}

#include <cmath>
#include <sstream>

#include "context.hpp"
#include "string.hpp"
#include "unicode.hpp"

namespace plorth
{
  /**
   * ary? ( any -- any bool )
   *
   * Returns true if given value is array.
   */
  static void w_is_ary(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_ARRAY);
    }
  }

  /**
   * bool? ( any -- any bool )
   *
   * Returns true if given value is boolean.
   */
  static void w_is_bool(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_BOOL);
    }
  }

  /**
   * error? ( any -- any bool )
   *
   * Returns true if given value is error.
   */
  static void w_is_error(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_ERROR);
    }
  }

  /**
   * null? ( any -- any bool )
   *
   * Returns true if item on top of the stack is null.
   */
  static void w_is_null(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_NULL);
    }
  }

  /**
   * num? ( any -- any bool )
   *
   * Returns true if given value is number.
   */
  static void w_is_num(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_NUMBER);
    }
  }

  /**
   * obj? ( any -- any bool )
   *
   * Returns true if given value is object.
   */
  static void w_is_obj(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_OBJECT);
    }
  }

  /**
   * quote? ( any -- any bool )
   *
   * Returns true if given value is quote.
   */
  static void w_is_quote(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_QUOTE);
    }
  }

  /**
   * str? ( any -- any bool )
   *
   * Returns true if given value is string.
   */
  static void w_is_str(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_STRING);
    }
  }

  /**
   * >bool ( any -- bool )
   *
   * Converts value from top of the stack into boolean. Null and false values
   * becomes false while everything else becomes true.
   */
  static void w_to_bool(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (!context->Pop(value))
    {
      return;
    }
    if (value->GetType() == Value::TYPE_BOOL)
    {
      context->PushBool(value);
    } else {
      context->PushBool(value->GetType() != Value::TYPE_NULL);
    }
  }

  /**
   * >str ( any -- str )
   *
   * Converts value from top of the stack into string.
   */
  static void w_to_str(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      context->PushString(value->ToString());
    }
  }

  /**
   * null ( -- null )
   *
   * Returns null value.
   */
  static void w_null(const Ref<Context>& context)
  {
    context->PushNull();
  }

  /**
   * true ( -- bool )
   *
   * Inserts true value on top of the stack.
   */
  static void w_true(const Ref<Context>& context)
  {
    context->PushBool(true);
  }

  /**
   * false ( -- bool )
   *
   * Inserts false value on top of the stack.
   */
  static void w_false(const Ref<Context>& context)
  {
    context->PushBool(false);
  }

  /**
   * e ( -- num )
   *
   * Returns Eulers number.
   */
  static void w_e(const Ref<Context>& context)
  {
    context->PushNumber(M_E);
  }

  /**
   * pi ( -- num )
   *
   * Returns value of PI.
   */
  static void w_pi(const Ref<Context>& context)
  {
    context->PushNumber(M_PI);
  }

  /**
   * nop ( -- )
   *
   * Does nothing. Can be used to construct empty quotes.
   */
  static void w_nop(const Ref<Context>& context) {}

  /**
   * clear ( -- )
   *
   * Clears the entire stack.
   */
  static void w_clear(const Ref<Context>& context)
  {
    context->ClearStack();
  }

  /**
   * depth ( -- num )
   *
   * Returns current depth of the stack.
   */
  static void w_depth(const Ref<Context>& context)
  {
    context->PushNumber(static_cast<std::int64_t>(context->GetStack().size()));
  }

  /**
   * drop ( any -- )
   *
   * Discards top-most value of the stack.
   */
  static void w_drop(const Ref<Context>& context)
  {
    context->Pop();
  }

  /**
   * drop2 ( any any -- )
   *
   * Discards two top-most values of the stack.
   */
  static void w_drop2(const Ref<Context>& context)
  {
    if (context->Pop())
    {
      context->Pop();
    }
  }

  /**
   * dup ( any -- any any )
   *
   * Duplicates top-most value of the stack.
   */
  static void w_dup(const Ref<Context>& context)
  {
    if (context->GetStack().empty())
    {
      context->SetError(Error::ERROR_CODE_RANGE, "Stack underflow");
    } else {
      context->Push(context->GetStack().back());
    }
  }

  /**
   * dup2 ( any any -- any any any any )
   *
   * Duplicates two top-most values on the stack.
   */
  static void w_dup2(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (context->Pop(a) && context->Pop(b))
    {
      context->Push(b);
      context->Push(a);
      context->Push(b);
      context->Push(a);
    }
  }

  /**
   * nip ( any any -- any )
   *
   * Drops the first value and pushes second value on the stack.
   */
  static void w_nip(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value) && context->Pop())
    {
      context->Push(value);
    }
  }

  /**
   * over ( any any -- any any any )
   *
   * Copies second top-most value from the stack into top-most value of the
   * stack.
   */
  static void w_over(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (context->Pop(a) && context->Pop(b))
    {
      context->Push(b);
      context->Push(a);
      context->Push(b);
    }
  }

  /**
   * rot ( any any any -- any any any )
   *
   * Rotates three top-most values on the stack.
   */
  static void w_rot(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;
    Ref<Value> c;

    if (context->Pop(a) && context->Pop(b) && context->Pop(c))
    {
      context->Push(b);
      context->Push(a);
      context->Push(c);
    }
  }

  /**
   * swap ( any any -- any any )
   *
   * Swaps positions of two top-most values on the stack.
   */
  static void w_swap(const Ref<Context>& context)
  {
    Ref<Value> a;
    Ref<Value> b;

    if (context->Pop(a) && context->Pop(b))
    {
      context->Push(a);
      context->Push(b);
    }
  }

  /**
   * typeof ( any -- any str )
   *
   * Returns name of the type of given value as string.
   */
  static void w_typeof(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      std::stringstream ss;

      ss << value->GetType();
      context->PushString(ss.str());
    }
  }

  /**
   * proto ( any -- obj|null )
   *
   * Returns prototype of the value, or null if prototype cannot be determined
   * for some reason.
   */
  static void w_proto(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      const Ref<Object> prototype = value->GetPrototype(context->GetRuntime());

      if (prototype)
      {
        context->Push(prototype);
      } else {
        context->PushNull();
      }
    }
  }

  /**
   * repr ( any -- str )
   *
   * Returns source code representation of the value.
   */
  static void w_repr(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      context->PushString(value->ToSource());
    }
  }

  /**
   * if ( bool quote -- )
   *
   * Executes quote if the boolean value is true.
   */
  static void w_if(const Ref<Context>& context)
  {
    bool condition;
    Ref<Quote> quote;

    if (context->PopQuote(quote) && context->PopBool(condition) && condition)
    {
      quote->Call(context);
    }
  }

  /**
   * if-else ( bool quote quote -- )
   *
   * Calls first quote if boolean value is true, second quote otherwise.
   */
  static void w_if_else(const Ref<Context>& context)
  {
    bool condition;
    Ref<Quote> then_quote;
    Ref<Quote> else_quote;

    if (!context->PopQuote(else_quote)
        || !context->PopQuote(then_quote)
        || !context->PopBool(condition))
    {
      return;
    }
    if (condition)
    {
      then_quote->Call(context);
    } else {
      else_quote->Call(context);
    }
  }

  /**
   * while ( bool quote -- )
   *
   * Executes given quote while top of the stack value is true.
   */
  static void w_while(const Ref<Context>& context)
  {
    bool condition;
    Ref<Quote> quote;

    if (!context->PopQuote(quote) || !context->PopBool(condition))
    {
      return;
    }
    while (condition)
    {
      if (!quote->Call(context) || !context->PopBool(condition))
      {
        return;
      }
    }
  }

  /**
   * try ( quote quote -- )
   *
   * Executes first quote and if it throws an error, calls second quote with
   * the error on top of the stack.
   */
  static void w_try(const Ref<Context>& context)
  {
    Ref<Quote> try_block;
    Ref<Quote> catch_block;

    if (!context->PopQuote(catch_block) || !context->PopQuote(try_block))
    {
      return;
    }
    if (!try_block->Call(context))
    {
      context->Push(context->GetError());
      context->ClearError();
      catch_block->Call(context);
    }
  }

  /**
   * compile ( str -- quote )
   *
   * Compiles given string into quote.
   */
  static void w_compile(const Ref<Context>& context)
  {
    Ref<String> source;

    if (context->PopString(source))
    {
      const Ref<Quote> quote = Quote::Compile(context, source->GetValue());

      if (quote)
      {
        context->Push(quote);
      }
    }
  }

  /**
   * globals ( -- obj )
   *
   * Returns global dictionary as object.
   */
  static void w_globals(const Ref<Context>& context)
  {
    context->PushObject(context->GetRuntime()->GetDictionary());
  }

  /**
   * locals ( -- obj )
   *
   * Returns local dictionary of current program execution context as object.
   */
  static void w_locals(const Ref<Context>& context)
  {
    context->PushObject(context->GetDictionary());
  }

  /**
   * print ( any -- )
   *
   * Prints top-most value of the stack to stdout.
   */
  static void w_print(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      std::cout << value;
    }
  }

  /**
   * println ( any -- )
   *
   * Prints top-most value of the stack to stdout with terminating new line.
   */
  static void w_println(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      std::cout << value << std::endl;
    }
  }

  /**
   * . ( any -- )
   *
   * Pretty prints value from top of the stack into standard output stream.
   */
  static void w_dot(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      std::cout << value->ToSource() << std::endl;
    }
  }

  /**
   * bl ( -- )
   *
   * Outputs whitespace into the standard output stream.
   */
  static void w_bl(const Ref<Context>& context)
  {
    std::cout << ' ';
  }

  /**
   * nl ( -- )
   *
   * Outputs new line into the standard output stream.
   */
  static void w_nl(const Ref<Context>& context)
  {
    std::cout << std::endl;
  }

  /**
   * emit ( num -- )
   *
   * Outputs given Unicode code point into the standard output stream.
   */
  static void w_emit(const Ref<Context>& context)
  {
    Ref<Number> value;

    if (context->PopNumber(value))
    {
      char buffer[5];

      if (unicode_encode(value->AsInt(), buffer))
      {
        std::cout << buffer;
      }
    }
  }

  void init_global_dictionary(Runtime* runtime)
  {
    runtime->AddWord("ary?", w_is_ary);
    runtime->AddWord("bool?", w_is_bool);
    runtime->AddWord("error?", w_is_error);
    runtime->AddWord("null?", w_is_null);
    runtime->AddWord("num?", w_is_num);
    runtime->AddWord("obj?", w_is_obj);
    runtime->AddWord("quote?", w_is_quote);
    runtime->AddWord("str?", w_is_str);

    runtime->AddWord(">bool", w_to_bool);
    runtime->AddWord(">str", w_to_str);

    // Constants.
    runtime->AddWord("null", w_null);
    runtime->AddWord("true", w_true);
    runtime->AddWord("false", w_false);
    runtime->AddWord("e", w_e);
    runtime->AddWord("pi", w_pi);

    runtime->AddWord("typeof", w_typeof);
    runtime->AddWord("proto", w_proto);
    runtime->AddWord("repr", w_repr);

    runtime->AddWord("nop", w_nop);
    runtime->AddWord("clear", w_clear);
    runtime->AddWord("depth", w_depth);
    runtime->AddWord("drop", w_drop);
    runtime->AddWord("drop2", w_drop2);
    runtime->AddWord("dup", w_dup);
    runtime->AddWord("dup2", w_dup2);
    runtime->AddWord("nip", w_nip);
    runtime->AddWord("over", w_over);
    runtime->AddWord("rot", w_rot);
    runtime->AddWord("swap", w_swap);

    runtime->AddWord("if", w_if);
    runtime->AddWord("if-else", w_if_else);
    runtime->AddWord("while", w_while);
    runtime->AddWord("try", w_try);

    runtime->AddWord("compile", w_compile);
    runtime->AddWord("globals", w_globals);
    runtime->AddWord("locals", w_locals);

    runtime->AddWord("print", w_print);
    runtime->AddWord("println", w_println);
    runtime->AddWord(".", w_dot);
    runtime->AddWord("bl", w_bl);
    runtime->AddWord("nl", w_nl);
    runtime->AddWord("emit", w_emit);
  }
}

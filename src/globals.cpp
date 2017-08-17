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
#include <plorth/value-number.hpp>
#include <plorth/value-string.hpp>

#include <cmath>
#include <ctime>

#include <sstream>

namespace plorth
{
  /**
   * null ( -- null )
   *
   * Returns null value.
   */
  static void w_null(const ref<context>& ctx)
  {
    ctx->push_null();
  }

  /**
   * true ( -- boolean )
   *
   * Returns boolean value of true.
   */
  static void w_true(const ref<context>& ctx)
  {
    ctx->push_boolean(true);
  }

  /**
   * false ( -- boolean )
   *
   * Returns boolean value of false.
   */
  static void w_false(const ref<context>& ctx)
  {
    ctx->push_boolean(false);
  }

  /**
   * e ( -- number )
   *
   * Returns Eulers number.
   */
  static void w_e(const ref<context>& ctx)
  {
    ctx->push_number(M_E);
  }

  /**
   * pi ( -- number )
   *
   * Returns value of pi.
   */
  static void w_pi(const ref<context>& ctx)
  {
    ctx->push_number(M_PI);
  }

  /**
   * nop ( -- )
   *
   * Does nothing. Can be used to construct empty quotes.
   */
  static void w_nop(const ref<context>&) {}

  /**
   * clear ( -- )
   *
   * Clears the entire stack of current context.
   */
  static void w_clear(const ref<context>& ctx)
  {
    ctx->clear();
  }

  /**
   * depth ( -- number )
   *
   * Returns current depth of the stack.
   */
  static void w_depth(const ref<context>& ctx)
  {
    ctx->push_number(ctx->size());
  }

  /**
   * drop ( any -- )
   *
   * Discards top-most value from the stack.
   */
  static void w_drop(const ref<context>& ctx)
  {
    ctx->pop();
  }

  /**
   * 2drop ( any any -- )
   *
   * Discards two top-most values from the stack.
   */
  static void w_drop2(const ref<context>& ctx)
  {
    if (ctx->pop())
    {
      ctx->pop();
    }
  }

  /**
   * dup ( any -- any any )
   *
   * Duplicates top-most value of the stack.
   */
  static void w_dup(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push(value);
    }
  }

  /**
   * 2dup ( any any -- any any any any )
   *
   * Duplicates two top-most values of the stack.
   */
  static void w_dup2(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push(b);
      ctx->push(a);
      ctx->push(b);
      ctx->push(a);
    }
  }

  /**
   * nip ( any any -- any )
   *
   * Drops the first value and pushes second value on the stack.
   */
  static void w_nip(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value) && ctx->pop())
    {
      ctx->push(value);
    }
  }

  /**
   * over ( any any -- any any any )
   *
   * Copies second top-most value of the stack into top-most value of the
   * stack.
   */
  static void w_over(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push(b);
      ctx->push(a);
      ctx->push(b);
    }
  }

  /**
   * rot ( any any any -- any any any )
   *
   * Rotates three top-most values on the stack.
   */
  static void w_rot(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;
    ref<value> c;

    if (ctx->pop(a) && ctx->pop(b) && ctx->pop(c))
    {
      ctx->push(b);
      ctx->push(a);
      ctx->push(c);
    }
  }

  /**
   * swap ( any any -- any any )
   *
   * Swaps positions of two top-most values on the stack.
   */
  static void w_swap(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push(a);
      ctx->push(b);
    }
  }

  /**
   * tuck ( any any -- any any any )
   *
   * Copies top-most value of the stack as the third top-most value of the
   * stack.
   */
  static void w_tuck(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push(a);
      ctx->push(b);
      ctx->push(a);
    }
  }

  /**
   * array? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is array.
   */
  static void w_is_array(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_array));
    }
  }

  /**
   * boolean? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is boolean.
   */
  static void w_is_boolean(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_boolean));
    }
  }

  /**
   * error? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is error.
   */
  static void w_is_error(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_error));
    }
  }

  /**
   * number? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is boolean.
   */
  static void w_is_number(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_number));
    }
  }

  /**
   * null? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is null.
   */
  static void w_is_null(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(!value);
    }
  }

  /**
   * object? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is object.
   */
  static void w_is_object(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_object));
    }
  }

  /**
   * quote? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is quote.
   */
  static void w_is_quote(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_quote));
    }
  }

  /**
   * string? ( any -- any boolean )
   *
   * Returns true if top-most value of the stack is string.
   */
  static void w_is_string(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value && value->is(value::type_string));
    }
  }

  /**
   * typeof ( any -- any string )
   *
   * Returns name of the type of given value as string.
   */
  static void w_typeof(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      if (value)
      {
        std::stringstream ss;

        ss << value->type();
        ctx->push_string(utf8_decode(ss.str()));
      } else {
        ctx->push_string(utf8_decode("null"));
      }
    }
  }

  /**
   * prototype ( any -- any object )
   *
   * Retrieves prototype of the top-most value. If the top-most value of the
   * stack is null, null will be returned instead.
   */
  static void w_prototype(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      if (value)
      {
        ctx->push(value->prototype(ctx->runtime()));
      } else {
        ctx->push_null();
      }
    }
  }

  /**
   * >boolean ( any -- boolean )
   *
   * Converts top-most value of the stack into boolean. Null and false will
   * become false while everything else will be true.
   */
  static void w_to_boolean(const ref<context>& ctx)
  {
    ref<class value> value;

    if (!ctx->pop(value))
    {
      return;
    }
    else if (value && value->is(value::type_boolean))
    {
      ctx->push(value);
    } else {
      ctx->push_boolean(!!value);
    }
  }

  /**
   * >string ( any -- string )
   *
   * Converts top-most value of the stack into string.
   */
  static void w_to_string(const ref<context>& ctx)
  {
    ref<class value> value;

    if (!ctx->pop(value))
    {
      return;
    }
    else if (value)
    {
      ctx->push_string(value->to_string());
    } else {
      ctx->push_string(utf8_decode(""));
    }
  }

  /**
   * >source ( any -- string )
   *
   * Converts top-most value of the stack into a string that most accurately
   * represents what the value would look like in source code.
   */
  static void w_to_source(const ref<context>& ctx)
  {
    ref<class value> value;

    if (!ctx->pop(value))
    {
      return;
    }
    else if (value)
    {
      ctx->push_string(value->to_source());
    } else {
      ctx->push_string(utf8_decode("null"));
    }
  }

  /**
   * print ( any -- )
   *
   * Prints top-most value of the stack to stdout.
   */
  static void w_print(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value) && value)
    {
      std::cout << value;
    }
  }

  /**
   * if ( boolean quote -- )
   *
   * Executes quote if the boolean value is true.
   */
  static void w_if(const ref<context>& ctx)
  {
    bool condition;
    ref<class quote> quote;

    if (ctx->pop_quote(quote) && ctx->pop_boolean(condition) && condition)
    {
      quote->call(ctx);
    }
  }

  /**
   * if-else ( boolean quote quote -- )
   *
   * Calls first quote if boolean value is true, second quote otherwise.
   */
  static void w_if_else(const ref<context>& ctx)
  {
    bool condition;
    ref<quote> then_quote;
    ref<quote> else_quote;

    if (!ctx->pop_quote(else_quote)
        || !ctx->pop_quote(then_quote)
        || !ctx->pop_boolean(condition))
    {
      return;
    }

    if (condition)
    {
      then_quote->call(ctx);
    } else {
      else_quote->call(ctx);
    }
  }

  /**
   * while ( quote quote -- )
   *
   * Executes second quote as long as the first quote returns true.
   */
  static void w_while(const ref<context>& ctx)
  {
    ref<quote> test;
    ref<quote> body;

    if (!ctx->pop_quote(body) || !ctx->pop_quote(test))
    {
      return;
    }

    for (;;)
    {
      bool b;

      if (!test->call(ctx) || !ctx->pop_boolean(b) || !b || !body->call(ctx))
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
  static void w_try(const ref<context>& ctx)
  {
    ref<quote> try_quote;
    ref<quote> catch_quote;

    if (!ctx->pop_quote(catch_quote) || !ctx->pop_quote(try_quote))
    {
      return;
    }

    if (!try_quote->call(ctx))
    {
      ctx->push(ctx->error());
      ctx->clear_error();
      catch_quote->call(ctx);
    }
  }

  /**
   * try ( quote quote quote -- )
   *
   * Executes first quote and if it throws an error, calls second quote with
   * the error on top of the stack. If no error was thrown, third quote will
   * be called instead.
   */
  static void w_try_else(const ref<context>& ctx)
  {
    ref<quote> try_quote;
    ref<quote> catch_quote;
    ref<quote> else_quote;

    if (!ctx->pop_quote(else_quote)
        || !ctx->pop_quote(catch_quote)
        || !ctx->pop_quote(try_quote))
    {
      return;
    }

    if (!try_quote->call(ctx))
    {
      ctx->push(ctx->error());
      ctx->clear_error();
      catch_quote->call(ctx);
    } else {
      else_quote->call(ctx);
    }
  }

  /**
   * compile ( string -- quote )
   *
   * Compiles given string of source code into quote.
   */
  static void w_compile(const ref<context>& ctx)
  {
    ref<string> source;
    ref<class quote> quote;

    if (!ctx->pop_string(source))
    {
      return;
    }

    quote = ctx->compile(utf8_encode(source->value()));
    if (quote)
    {
      ctx->push(quote);
    }
  }

  /**
   * globals ( -- object )
   *
   * Returns global dictionary as object.
   */
  static void w_globals(const ref<context>& ctx)
  {
    ctx->push_object(ctx->runtime()->dictionary());
  }

  /**
   * locals ( -- object )
   *
   * Returns local dictionary of current execution context as object.
   */
  static void w_locals(const ref<context>& ctx)
  {
    ctx->push_object(ctx->dictionary());
  }

  /**
   * const ( any string -- )
   *
   * Declares given value as constant in the current context with name
   * identified by given string.
   */
  static void w_const(const ref<context>& ctx)
  {
    ref<string> id;
    ref<value> val;

    if (ctx->pop_string(id) && ctx->pop(val))
    {
      ctx->declare(id->value(), ctx->runtime()->constant(val));
    }
  }

  /**
   * import ( string -- )
   *
   * Imports module from given path and adds all of it's exported words into
   * this execution context.
   */
  static void w_import(const ref<context>& ctx)
  {
    ref<string> path;

    if (ctx->pop_string(path))
    {
      ctx->runtime()->import(ctx, path->value());
    }
  }

  static void make_error(const ref<context>& ctx, enum error::code code)
  {
    ref<value> val;
    unistring message;

    if (!ctx->pop(val))
    {
      return;
    }

    if (val)
    {
      if (val->is(value::type_string))
      {
        message = val.cast<string>()->value();
      } else {
        std::stringstream ss;

        ss << "Expected string, got " << val->type() << " instead.";
        ctx->error(error::code_type, utf8_decode(ss.str()));
        return;
      }
    }

    ctx->push(ctx->runtime()->value<error>(code, message));
  }

  /**
   * type-error ( string|null -- error )
   *
   * Construct instance of type error with with given optional error message
   * and places it on the stack.
   */
  static void w_type_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_type);
  }

  /**
   * value-error ( string|null -- error )
   *
   * Constructs instance of value error with given optional error message and
   * places it on the stack.
   */
  static void w_value_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_value);
  }

  /**
   * range-error ( string|null -- error )
   *
   * Construct instance of range error with with given optional error message
   * and places it on the stack.
   */
  static void w_range_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_range);
  }

  /**
   * unknown-error ( string|null -- error )
   *
   * Construct instance of unknown error with with given optional error
   * message and places it on the stack.
   */
  static void w_unknown_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_unknown);
  }

  /**
   * println ( any -- )
   *
   * Prints top-most value of the stack to stdout with terminating new line.
   */
  static void w_println(const ref<context>& ctx)
  {
    ref<class value> value;

    if (ctx->pop(value))
    {
      if (value)
      {
        std::cout << value;
      }
      std::cout << std::endl;
    }
  }

  /**
   * emit ( number -- )
   *
   * Outputs given Unicode code point into the standard output stream. Range
   * error will be thrown if the given number is not valid Unicode code point.
   */
  static void w_emit(const ref<context>& ctx)
  {
    double number;

    if (!ctx->pop_number(number))
    {
      return;
    }

    if (!unichar_validate(number))
    {
      ctx->error(error::code_range, "Invalid Unicode code point.");
      return;
    }

    std::cout << unistring(1, number);
  }

  /**
   * now ( -- number )
   *
   * Returns the timestamp of the number of seconds that have elapsed since the
   * Unix epoch (1 January 1970 00:00:00 UTC).
   */
  static void w_now(const ref<context>& ctx)
  {
    ctx->push_number(std::time(nullptr));
  }

  /**
   * = ( any any -- boolean )
   *
   * Tests whether the two top-most values of the stack are equal.
   */
  static void w_eq(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push_boolean(b == a);
    }
  }

  /**
   * != ( any any -- boolean )
   *
   * Tests whether the two top-most values of the stack are equal.
   */
  static void w_ne(const ref<context>& ctx)
  {
    ref<value> a;
    ref<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push_boolean(b != a);
    }
  }

  namespace api
  {
    runtime::prototype_definition global_dictionary()
    {
      return
      {
        // Constants.
        { "null", w_null },
        { "true", w_true },
        { "false", w_false },
        { "e", w_e },
        { "pi", w_pi },

        // Stack manipulation.
        { "nop", w_nop },
        { "clear", w_clear },
        { "depth", w_depth },
        { "drop", w_drop },
        { "2drop", w_drop2 },
        { "dup", w_dup },
        { "2dup", w_dup2 },
        { "nip", w_nip },
        { "over", w_over },
        { "rot", w_rot },
        { "swap", w_swap },
        { "tuck", w_tuck },

        // Value types.
        { "array?", w_is_array },
        { "boolean?", w_is_boolean },
        { "error?", w_is_error },
        { "null?", w_is_null },
        { "number?", w_is_number },
        { "object?", w_is_object },
        { "quote?", w_is_quote },
        { "string?", w_is_string },
        { "typeof" , w_typeof },
        { "prototype", w_prototype },

        // Conversions.
        { ">boolean", w_to_boolean },
        { ">string", w_to_string },
        { ">source", w_to_source },

        // Logic.
        { "if", w_if },
        { "if-else", w_if_else },
        { "while", w_while },
        { "try", w_try },
        { "try-else", w_try_else },

        // Interpreter related.
        { "compile", w_compile },
        { "globals", w_globals },
        { "locals", w_locals },
        { "const", w_const },
        { "import", w_import },

        // Different types of errors.
        { "type-error", w_type_error },
        { "value-error", w_value_error },
        { "range-error", w_range_error },
        { "unknown-error", w_unknown_error },

        // I/O related.
        { "print", w_print },
        { "println", w_println },
        { "emit", w_emit },

        // Random utilities.
        { "now", w_now },

        // Global operators.
        { "=", w_eq },
        { "!=", w_ne },
      };
    }
  }
}

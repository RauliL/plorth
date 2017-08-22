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

#include <cmath>
#include <ctime>

#include <sstream>

namespace plorth
{
  /**
   * Word: null
   *
   * Gives:
   * - null
   *
   * Pushes the null value onto stack.
   */
  static void w_null(const ref<context>& ctx)
  {
    ctx->push_null();
  }

  /**
   * Word: true
   *
   * Gives:
   * - boolean
   *
   * Pushes the boolean value true onto stack.
   */
  static void w_true(const ref<context>& ctx)
  {
    ctx->push_boolean(true);
  }

  /**
   * Word: false
   *
   * Gives:
   * - boolean
   *
   * Pushes the boolean value false onto stack.
   */
  static void w_false(const ref<context>& ctx)
  {
    ctx->push_boolean(false);
  }

  /**
   * Word: e
   *
   * Gives:
   * - number
   *
   * Pushes Euler's number onto stack.
   */
  static void w_e(const ref<context>& ctx)
  {
    ctx->push_real(M_E);
  }

  /**
   * Word: pi
   *
   * Gives:
   * - number
   *
   * Pushes the value of pi onto stack.
   */
  static void w_pi(const ref<context>& ctx)
  {
    ctx->push_real(M_PI);
  }

  /**
   * Word: nop
   *
   * Does nothing. Can be used to construct empty quotes.
   */
  static void w_nop(const ref<context>&) {}

  /**
   * Word: clear
   *
   * Clears the entire stack of current context.
   */
  static void w_clear(const ref<context>& ctx)
  {
    ctx->clear();
  }

  /**
   * Word: depth
   *
   * Gives:
   * - number
   *
   * Pushes current depth of the stack onto stack.
   */
  static void w_depth(const ref<context>& ctx)
  {
    ctx->push_int(ctx->size());
  }

  /**
   * Word: drop
   *
   * Takes:
   * - any
   *
   * Discards topmost value from the stack.
   *
   *     1 drop #=> empty stack
   */
  static void w_drop(const ref<context>& ctx)
  {
    ctx->pop();
  }

  /**
   * Word: 2drop
   *
   * Takes:
   * - any
   * - any
   *
   * Discards the two topmost values from the stack.
   *
   *     1 2 3 2drop #=> 1
   */
  static void w_drop2(const ref<context>& ctx)
  {
    if (ctx->pop())
    {
      ctx->pop();
    }
  }

  /**
   * Word: dup
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - any
   *
   * Duplicates the topmost value of the stack.
   *
   *     1 dup #=> 1 1
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
   * Word: 2dup
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - any
   * - any
   * - any
   * - any
   *
   * Duplicates two topmost values of the stack.
   *
   *     1 2 2dup #=> 1 2 1 2
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
   * Word: nip
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - any
   *
   * Drops the first value and pushes the second value onto stack.
   *
   *     1 2 nip #=> 2
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
   * Word: over
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - any
   * - any
   * - any
   *
   * Copies second topmost value of the stack into topmost value of the
   * stack.
   *
   *     1 2 over #=> 1 2 1
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
   * Word: rot
   *
   * Takes:
   * - any
   * - any
   * - any
   *
   * Gives:
   * - any
   * - any
   * - any
   *
   * Rotates the three topmost values on the stack.
   *
   *     1 2 3 rot #=> 2 3 1
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
   * Word: swap
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - any
   * - any
   *
   * Swaps positions of the two topmost values on the stack.
   *
   *     1 2 swap #=> 2 1
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
   * Word: tuck
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - any
   * - any
   * - any
   *
   * Copies the topmost value of the stack as the third topmost value of the
   * stack.
   *
   *     1 2 tuck #=> 2 1 2
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
   * Word: array?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is an array.
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
   * Word: boolean?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is a boolean.
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
   * Word: error?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is an error.
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
   * Word: number?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is a number.
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
   * Word: null?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is null.
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
   * Word: object?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is an object.
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
   * Word: quote?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is a quote.
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
   * Word: string?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is a string.
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
   * Word: typeof
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - string
   *
   * Returns name of the type of the topmost value as a string.
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
   * Word: proto
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - object
   *
   * Retrieves proto of the topmost value. If the topmost value of the stack
   * is null, null will be returned instead.
   */
  static void w_proto(const ref<context>& ctx)
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
   * Word: >boolean
   *
   * Takes:
   * - any
   *
   * Gives:
   * - boolean
   *
   * Converts the topmost value of the stack into a boolean. Null and false
   * will become false while everything else will become true.
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
   * Word: >string
   *
   * Takes:
   * - any
   *
   * Gives:
   * - string
   *
   * Converts the topmost value of the stack into a string. Null will become
   * an empty string.
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
   * Word: >source
   *
   * Takes:
   * - any
   *
   * Gives:
   * - string
   *
   * Converts the topmost value of the stack into a string that most accurately
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
   * Word: if
   *
   * Takes:
   * - boolean
   * - quote
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
   * Word: if-else
   *
   * Takes:
   * - boolean
   * - quote
   * - quote
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
   * Word: while
   *
   * Takes:
   * - quote
   * - quote
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
   * Word: try
   *
   * Takes:
   * - quote
   * - quote
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
   * Word: try-else
   *
   * Takes:
   * - quote
   * - quote
   * - quote
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
   * Word: compile
   *
   * Takes:
   * - string
   *
   * Gives:
   * - quote
   *
   * Compiles given string of source code into a quote.
   */
  static void w_compile(const ref<context>& ctx)
  {
    ref<string> source;
    ref<class quote> quote;

    if (!ctx->pop_string(source))
    {
      return;
    }

    quote = ctx->compile(source->to_string());
    if (quote)
    {
      ctx->push(quote);
    }
  }

  /**
   * Word: globals
   *
   * Gives:
   * - object
   *
   * Returns the global dictionary as an object.
   */
  static void w_globals(const ref<context>& ctx)
  {
    ctx->push_object(ctx->runtime()->dictionary());
  }

  /**
   * Word: locals
   *
   * Gives:
   * - object
   *
   * Returns the local dictionary of current execution context as an object.
   */
  static void w_locals(const ref<context>& ctx)
  {
    ctx->push_object(ctx->dictionary());
  }

  /**
   * Word: const
   *
   * Takes:
   * - any
   * - string
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
      ctx->declare(id->to_string(), ctx->runtime()->constant(val));
    }
  }

  /**
   * Word: import
   *
   * Takes:
   * - string
   *
   * Imports module from given path and adds all of its exported words into
   * this execution context.
   */
  static void w_import(const ref<context>& ctx)
  {
    ref<string> path;

    if (ctx->pop_string(path))
    {
      ctx->runtime()->import(ctx, path->to_string());
    }
  }

  /**
   * Word: args
   *
   * Gives:
   * - array<string>
   *
   * Returns command line arguments given to the interpreter as an array of
   * strings.
   */
  static void w_args(const ref<context>& ctx)
  {
    const ref<class runtime>& runtime = ctx->runtime();
    const auto& arguments = runtime->arguments();
    const auto size = arguments.size();
    std::vector<ref<value>> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      result.push_back(runtime->string(arguments[i]));
    }
    ctx->push_array(result.data(), size);
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
        message = val.cast<string>()->to_string();
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
   * Word: type-error
   *
   * Takes:
   * - string|null
   *
   * Gives:
   * - error
   *
   * Construct an instance of type error with with given optional error
   * message and places it on the stack.
   */
  static void w_type_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_type);
  }

  /**
   * Word: value-error
   *
   * Takes:
   * - string|null
   *
   * Gives:
   * - error
   *
   * Constructs an instance of value error with given optional error message
   * and places it on the stack.
   */
  static void w_value_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_value);
  }

  /**
   * Word: range-error
   *
   * Takes:
   * - string|null
   *
   * Gives:
   * - error
   *
   * Construct an instance of range error with given optional error message
   * and places it on the stack.
   */
  static void w_range_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_range);
  }

  /**
   * Word: unknown-error
   *
   * Takes:
   * - string|null
   *
   * Gives:
   * - error
   *
   * Construct an instance of unknown error with with given optional error
   * message and places it on the stack.
   */
  static void w_unknown_error(const ref<context>& ctx)
  {
    make_error(ctx, error::code_unknown);
  }

  /**
   * Word: print
   *
   * Takes:
   * - any
   *
   * Prints topmost value of the stack to stdout.
   */
  static void w_print(const ref<context>& ctx)
  {
    ref<value> val;

    if (ctx->pop(val) && val)
    {
      ctx->runtime()->print(val->to_string());
    }
  }

  /**
   * Word: println
   *
   * Takes:
   * - any
   *
   * Prints the topmost value of the stack to stdout with a terminating new
   * line.
   */
  static void w_println(const ref<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    ref<value> val;

    if (ctx->pop(val))
    {
      if (val)
      {
        runtime->print(val->to_string());
      }
      runtime->println();
    }
  }

  /**
   * Word: emit
   *
   * Takes:
   * - number
   *
   * Outputs given Unicode code point into the standard output stream. Range
   * error will be thrown if the given number is not a valid Unicode code
   * point.
   */
  static void w_emit(const ref<context>& ctx)
  {
    ref<number> num;

    if (ctx->pop_number(num))
    {
      std::int64_t c = num->as_int();

      if (!unichar_validate(c))
      {
        ctx->error(error::code_range, "Invalid Unicode code point.");
      } else {
        ctx->runtime()->print(unistring(1, static_cast<unichar>(c)));
      }
    }
  }

  /**
   * Word: now
   *
   * Gives:
   * - number
   *
   * Returns the number of seconds that have elapsed since the  Unix epoch
   * (1 January 1970 00:00:00 UTC) rounded to the nearest integer.
   */
  static void w_now(const ref<context>& ctx)
  {
    ctx->push_int(std::time(nullptr));
  }

  /**
   * Word: =
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - boolean
   *
   * Tests whether the two topmost values of the stack are equal.
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
   * Word: !=
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - boolean
   *
   * Tests whether the two topmost values of the stack are not equal.
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
        { "proto", w_proto },

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
        { "args", w_args },

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

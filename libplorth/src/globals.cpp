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
#include <plorth/context.hpp>

#include <cmath>
#include <chrono>

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
  static void w_null(const std::shared_ptr<context>& ctx)
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
  static void w_true(const std::shared_ptr<context>& ctx)
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
  static void w_false(const std::shared_ptr<context>& ctx)
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
  static void w_e(const std::shared_ptr<context>& ctx)
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
  static void w_pi(const std::shared_ptr<context>& ctx)
  {
    ctx->push_real(M_PI);
  }

  /**
   * Word: inf
   *
   * Gives:
   * - number
   *
   * Pushes the value of positive infinity onto stack.
   */
  static void w_inf(const std::shared_ptr<context>& ctx)
  {
    ctx->push_real(INFINITY);
  }

  /**
   * Word: -inf
   *
   * Gives:
   * - number
   *
   * Pushes the value of negative infinity onto stack.
   */
  static void w_minus_inf(const std::shared_ptr<context>& ctx)
  {
    ctx->push_real(-INFINITY);
  }

  /**
   * Word: nan
   *
   * Gives:
   * - number
   *
   * Pushes the value of NaN (not a number) onto stack.
   */
  static void w_nan(const std::shared_ptr<context>& ctx)
  {
    ctx->push_real(NAN);
  }


  /**
   * Word: nop
   *
   * Does nothing. Can be used to construct empty quotes.
   */
  static void w_nop(const std::shared_ptr<context>&) {}

  /**
   * Word: clear
   *
   * Clears the entire stack of current context.
   */
  static void w_clear(const std::shared_ptr<context>& ctx)
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
  static void w_depth(const std::shared_ptr<context>& ctx)
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
  static void w_drop(const std::shared_ptr<context>& ctx)
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
  static void w_drop2(const std::shared_ptr<context>& ctx)
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
  static void w_dup(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

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
  static void w_dup2(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;

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
  static void w_nip(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

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
  static void w_over(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;

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
  static void w_rot(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;
    std::shared_ptr<value> c;

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
  static void w_swap(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;

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
  static void w_tuck(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;

    if (ctx->pop(a) && ctx->pop(b))
    {
      ctx->push(a);
      ctx->push(b);
      ctx->push(a);
    }
  }

  static inline void type_test(const std::shared_ptr<context>& ctx,
                               enum value::type type)
  {
    std::shared_ptr<value> val;

    if (ctx->pop(val))
    {
      ctx->push(val);
      ctx->push_boolean(value::is(val, type));
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
  static void w_is_array(const std::shared_ptr<context>& ctx)
  {
    type_test(ctx, value::type::array);
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
  static void w_is_boolean(const std::shared_ptr<context>& ctx)
  {
    type_test(ctx, value::type::boolean);
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
  static void w_is_error(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value::is(value, value::type::error));
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
  static void w_is_number(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value::is(value, value::type::number));
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
  static void w_is_null(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value::is(value, value::type::null));
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
  static void w_is_object(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value::is(value, value::type::object));
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
  static void w_is_quote(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value::is(value, value::type::quote));
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
  static void w_is_string(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      ctx->push_boolean(value::is(value, value::type::string));
    }
  }

  /**
   * Word: symbol?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is symbol.
   */
  static void w_is_symbol(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val;

    if (ctx->pop(val))
    {
      ctx->push(val);
      ctx->push_boolean(value::is(val, value::type::symbol));
    }
  }

  /**
   * Word: word?
   *
   * Takes:
   * - any
   *
   * Gives:
   * - any
   * - boolean
   *
   * Returns true if the topmost value of the stack is word.
   */
  static void w_is_word(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val;

    if (ctx->pop(val))
    {
      ctx->push(val);
      ctx->push_boolean(value::is(val, value::type::word));
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
  static void w_typeof(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (ctx->pop(value))
    {
      ctx->push(value);
      if (value)
      {
        ctx->push_string(value->type_description());
      } else {
        ctx->push_string(U"null");
      }
    }
  }

  /**
   * Word: instance-of?
   *
   * Takes:
   * - any
   * - object
   *
   * Gives:
   * - any
   * - boolean
   *
   * Tests whether prototype chain of given value inherits from given object.
   */
  static void w_is_instance_of(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<value> val;
    std::shared_ptr<object> obj;

    if (ctx->pop_object(obj) && ctx->pop(val))
    {
      std::shared_ptr<value> prototype1;
      std::shared_ptr<value> prototype2 = val->prototype(runtime);

      ctx->push(val);

      if (!obj->own_property(U"prototype", prototype1) ||
          !value::is(prototype1, value::type::object) ||
          !prototype2)
      {
        ctx->push_boolean(false);
        return;
      }
      else if (prototype1->equals(prototype2))
      {
        ctx->push_boolean(true);
        return;
      }

      while (std::static_pointer_cast<object>(prototype2)->own_property(
              U"__proto__",
              prototype2
             ) &&
            value::is(prototype2, value::type::object))
      {
        if (prototype1->equals(prototype2))
        {
          ctx->push_boolean(true);
          return;
        }
      }

      ctx->push_boolean(false);
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
  static void w_proto(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

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
  static void w_to_boolean(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (!ctx->pop(value))
    {
      return;
    }
    else if (value::is(value, value::type::boolean))
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
  static void w_to_string(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (!ctx->pop(value))
    {
      return;
    }
    else if (value)
    {
      ctx->push_string(value->to_string());
    } else {
      ctx->push_string(U"");
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
  static void w_to_source(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<class value> value;

    if (!ctx->pop(value))
    {
      return;
    }
    else if (value)
    {
      ctx->push_string(value->to_source());
    } else {
      ctx->push_string(U"null");
    }
  }

  /**
   * Word: 1array
   *
   * Takes:
   * - any
   *
   * Gives:
   * - array
   *
   * Constructs array from given value.
   */
  static void w_1array(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val;

    if (ctx->pop(val))
    {
      ctx->push_array(&val, 1);
    }
  }

  /**
   * Word: 2array
   *
   * Takes:
   * - any
   * - any
   *
   * Gives:
   * - array
   *
   * Constructs array from given two values.
   */
  static void w_2array(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val1;
    std::shared_ptr<value> val2;

    if (ctx->pop(val2) && ctx->pop(val1))
    {
      std::shared_ptr<value> buffer[2];

      buffer[0] = val1;
      buffer[1] = val2;
      ctx->push_array(buffer, 2);
    }
  }

  /**
   * Word: narray
   *
   * Takes:
   * - any...
   * - number
   *
   * Gives:
   * - array
   *
   * Constructs array from given amount of values from the stack.
   */
  static void w_narray(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      const number::int_type size = num->as_int();
      std::shared_ptr<value>* buffer;

      if (size < 0)
      {
        ctx->error(error::code::range, U"Negative array size.");
        return;
      }

      buffer = new std::shared_ptr<value>[size];

      for (number::int_type i = 0; i < size; ++i)
      {
        std::shared_ptr<value> val;

        if (!ctx->pop(val))
        {
          delete[] buffer;
          return;
        }
        buffer[size - i - 1] = val;
      }

      ctx->push_array(buffer, size);
      delete[] buffer;
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
  static void w_if(const std::shared_ptr<context>& ctx)
  {
    bool condition;
    std::shared_ptr<class quote> quote;

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
  static void w_if_else(const std::shared_ptr<context>& ctx)
  {
    bool condition;
    std::shared_ptr<quote> then_quote;
    std::shared_ptr<quote> else_quote;

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
  static void w_while(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<quote> test;
    std::shared_ptr<quote> body;

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
  static void w_try(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<quote> try_quote;
    std::shared_ptr<quote> catch_quote;

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
  static void w_try_else(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<quote> try_quote;
    std::shared_ptr<quote> catch_quote;
    std::shared_ptr<quote> else_quote;

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
  static void w_compile(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> source;
    std::shared_ptr<class quote> quote;

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
  static void w_globals(const std::shared_ptr<context>& ctx)
  {
    const auto& dictionary = ctx->runtime()->dictionary();
    std::vector<object::value_type> result;

    result.reserve(dictionary.size());
    for (const auto& word : dictionary.words())
    {
      result.push_back({ word->symbol()->id(), word->quote() });
    }
    ctx->push_object(result);
  }

  /**
   * Word: locals
   *
   * Gives:
   * - object
   *
   * Returns the local dictionary of current execution context as an object.
   */
  static void w_locals(const std::shared_ptr<context>& ctx)
  {
    const auto& dictionary = ctx->dictionary();
    std::vector<object::value_type> result;

    result.reserve(dictionary.size());
    for (const auto& word : dictionary.words())
    {
      result.push_back({ word->symbol()->id(), word->quote() });
    }
    ctx->push_object(result);
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
  static void w_const(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> id;
    std::shared_ptr<value> val;

    if (ctx->pop_string(id) && ctx->pop(val))
    {
      const auto& runtime = ctx->runtime();

      ctx->dictionary().insert(runtime->word(
        runtime->symbol(id->to_string()),
        runtime->compiled_quote({ val })
      ));
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
  static void w_import(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<string> path;

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
  static void w_args(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    const auto& arguments = runtime->arguments();
    const auto size = arguments.size();
    std::vector<std::shared_ptr<value>> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      result.push_back(runtime->string(arguments[i]));
    }
    ctx->push_array(result.data(), size);
  }

  /**
   * Word: version
   *
   * Gives:
   * - string
   *
   * Returns version of the Plorth interpreter as string.
   */
  static void w_version(const std::shared_ptr<context>& ctx)
  {
    ctx->push_string(PLORTH_VERSION);
  }

  static void make_error(const std::shared_ptr<context>& ctx,
                         enum error::code code)
  {
    std::shared_ptr<value> val;
    std::u32string message;

    if (!ctx->pop(val))
    {
      return;
    }

    if (val)
    {
      if (val->is(value::type::string))
      {
        message = std::static_pointer_cast<string>(val)->to_string();
      } else {
        ctx->error(
          error::code::type,
          U"Expected string, got " + val->type_description() + U" instead."
        );
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
  static void w_type_error(const std::shared_ptr<context>& ctx)
  {
    make_error(ctx, error::code::type);
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
  static void w_value_error(const std::shared_ptr<context>& ctx)
  {
    make_error(ctx, error::code::value);
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
  static void w_range_error(const std::shared_ptr<context>& ctx)
  {
    make_error(ctx, error::code::range);
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
  static void w_unknown_error(const std::shared_ptr<context>& ctx)
  {
    make_error(ctx, error::code::unknown);
  }

  /**
   * Word: read
   *
   * Gives:
   * - string|null
   *
   * Reads all available input from standard input stream, decodes it as UTF-8
   * encoded text and returns result. If end of input has been reached, null
   * will be returned instead.
   */
  static void w_read(const std::shared_ptr<context>& ctx)
  {
    std::u32string output;
    io::input::size_type read;
    auto result = ctx->runtime()->read(0, output, read);

    if (result == io::input::result::failure)
    {
      ctx->error(error::code::io, U"Unable to decode input as UTF-8.");
    }
    else if (result == io::input::result::eof && output.empty())
    {
      ctx->push_null();
    } else {
      ctx->push_string(output);
    }
  }

  /**
   * Word: nread
   *
   * Takes:
   * - number
   *
   * Gives:
   * - string|null
   *
   * Reads given number of Unicode characters from standard input stream and
   * returns them in a string. If there is no more input to be read, null will
   * be returned instead. The resulting string might have less than given
   * number of characters if there isn't that much characters available from
   * the standard input stream.
   */
  static void w_nread(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      const number::int_type amount = num->as_int();
      std::u32string output;
      io::input::size_type read;
      io::input::result result;

      if (amount < 0)
      {
        ctx->error(error::code::range, U"Negative size to be read.");
        return;
      }
      else if (amount == 0)
      {
        ctx->error(error::code::range, U"Zero size to be read.");
        return;
      }
      result = ctx->runtime()->read(amount, output, read);
      if (result == io::input::result::failure)
      {
        ctx->error(error::code::io, U"Unable to decode input as UTF-8.");
        return;
      }
      else if (result == io::input::result::eof && output.empty())
      {
        ctx->push_null();
      } else {
        ctx->push_string(output);
      }
    }
  }

  /**
   * Word: print
   *
   * Takes:
   * - any
   *
   * Prints topmost value of the stack to stdout.
   */
  static void w_print(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> val;

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
  static void w_println(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<value> val;

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
  static void w_emit(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<number> num;

    if (ctx->pop_number(num))
    {
      number::int_type c = num->as_int();

      if (!unicode_validate(c))
      {
        ctx->error(error::code::range, U"Invalid Unicode code point.");
      } else {
        ctx->runtime()->print(std::u32string(1, static_cast<char32_t>(c)));
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
  static void w_now(const std::shared_ptr<context>& ctx)
  {
    const auto timestamp = std::chrono::system_clock::now().time_since_epoch();

    ctx->push_int(std::chrono::duration_cast<std::chrono::seconds>(timestamp).count());
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
  static void w_eq(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;

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
  static void w_ne(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<value> a;
    std::shared_ptr<value> b;

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
        { U"null", w_null },
        { U"true", w_true },
        { U"false", w_false },
        { U"e", w_e },
        { U"pi", w_pi },
        { U"inf", w_inf },
        { U"-inf", w_minus_inf },
        { U"nan", w_nan },

        // Stack manipulation.
        { U"nop", w_nop },
        { U"clear", w_clear },
        { U"depth", w_depth },
        { U"drop", w_drop },
        { U"2drop", w_drop2 },
        { U"dup", w_dup },
        { U"2dup", w_dup2 },
        { U"nip", w_nip },
        { U"over", w_over },
        { U"rot", w_rot },
        { U"swap", w_swap },
        { U"tuck", w_tuck },

        // Value types.
        { U"array?", w_is_array },
        { U"boolean?", w_is_boolean },
        { U"error?", w_is_error },
        { U"null?", w_is_null },
        { U"number?", w_is_number },
        { U"object?", w_is_object },
        { U"quote?", w_is_quote },
        { U"string?", w_is_string },
        { U"symbol?", w_is_symbol },
        { U"word?", w_is_word },
        { U"typeof" , w_typeof },
        { U"instance-of?", w_is_instance_of },
        { U"proto", w_proto },

        // Conversions.
        { U">boolean", w_to_boolean },
        { U">string", w_to_string },
        { U">source", w_to_source },

        // Constructors.
        { U"1array", w_1array },
        { U"2array", w_2array },
        { U"narray", w_narray },

        // Logic.
        { U"if", w_if },
        { U"if-else", w_if_else },
        { U"while", w_while },
        { U"try", w_try },
        { U"try-else", w_try_else },

        // Interpreter related.
        { U"compile", w_compile },
        { U"globals", w_globals },
        { U"locals", w_locals },
        { U"const", w_const },
        { U"import", w_import },
        { U"args", w_args },
        { U"version", w_version },

        // Different types of errors.
        { U"type-error", w_type_error },
        { U"value-error", w_value_error },
        { U"range-error", w_range_error },
        { U"unknown-error", w_unknown_error },

        // I/O related.
        { U"read", w_read },
        { U"nread", w_nread },
        { U"print", w_print },
        { U"println", w_println },
        { U"emit", w_emit },

        // Random utilities.
        { U"now", w_now },

        // Global operators.
        { U"=", w_eq },
        { U"!=", w_ne },
      };
    }
  }
}

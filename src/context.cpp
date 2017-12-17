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
#include <plorth/value-word.hpp>

#include "./utils.hpp"

namespace plorth
{
  context::context(const ref<class runtime>& runtime)
    : m_runtime(runtime) {}

  void context::error(enum error::code code,
                      const unistring& message,
                      const struct position* position)
  {
    m_error = new (m_runtime->memory_manager()) class error(
      code,
      message,
      position
    );
  }

  void context::push_null()
  {
    push(ref<value>());
  }

  void context::push_boolean(bool value)
  {
    push(m_runtime->boolean(value));
  }

  void context::push_int(number::int_type value)
  {
    push(m_runtime->number(value));
  }

  void context::push_real(number::real_type value)
  {
    push(m_runtime->number(value));
  }

  void context::push_number(const unistring& value)
  {
    push(m_runtime->number(value));
  }

  void context::push_string(const unistring& value)
  {
    push(m_runtime->string(value.c_str(), value.length()));
  }

  void context::push_string(string::const_pointer chars, string::size_type length)
  {
    push(m_runtime->string(chars, length));
  }

  void context::push_array(array::const_pointer elements, array::size_type size)
  {
    push(m_runtime->array(elements, size));
  }

  void context::push_object(const object::container_type& properties)
  {
    push(m_runtime->value<object>(properties));
  }

  void context::push_symbol(const unistring& id)
  {
    push(m_runtime->symbol(id));
  }

  void context::push_quote(const std::vector<ref<value>>& values)
  {
    push(m_runtime->compiled_quote(values));
  }

  void context::push_word(const ref<class symbol>& symbol,
                          const ref<class quote>& quote)
  {
    push(m_runtime->value<word>(symbol, quote));
  }

  bool context::pop()
  {
    if (!m_data.empty())
    {
      m_data.pop_back();

      return true;
    }
    error(error::code_range, U"Stack underflow.");

    return false;
  }

  bool context::pop(enum value::type type)
  {
    if (!m_data.empty())
    {
      const ref<class value>& value = m_data.back();

      if ((!value && type != value::type_null) || (value && !value->is(type)))
      {
        error(
          error::code_type,
          U"Expected " + value::type_description(type) + U", got " +
          (value ? value->type_description().c_str() : U"null") +
          U" instead."
        );

        return false;
      }
      m_data.pop_back();

      return true;
    }
    error(error::code_range, U"Stack underflow.");

    return false;
  }

  bool context::pop(ref<value>& slot)
  {
    if (!m_data.empty())
    {
      slot = m_data.back();
      m_data.pop_back();

      return true;
    }
    error(error::code_range, U"Stack underflow.");

    return false;
  }

  bool context::pop(ref<value>& slot, enum value::type type)
  {
    if (!m_data.empty())
    {
      slot = m_data.back();
      if ((!slot && type != value::type_null) || (slot && !slot->is(type)))
      {
        error(
          error::code_type,
          U"Expected " + value::type_description(type) + U", got " +
          (slot ? slot->type_description().c_str() : U"null") +
          U" instead."
        );

        return false;
      }
      m_data.pop_back();

      return true;
    }
    error(error::code_range, U"Stack underflow.");

    return false;
  }

  bool context::pop_boolean(bool& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_boolean))
    {
      return false;
    }
    slot = value.cast<boolean>()->value();

    return true;
  }

  bool context::pop_number(ref<number>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_number))
    {
      return false;
    }
    slot = value.cast<number>();

    return true;
  }

  bool context::pop_string(ref<string>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_string))
    {
      return false;
    }
    slot = value.cast<string>();

    return true;
  }

  bool context::pop_array(ref<array>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_array))
    {
      return false;
    }
    slot = value.cast<array>();

    return true;
  }

  bool context::pop_object(ref<object>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_object))
    {
      return false;
    }
    slot = value.cast<object>();

    return true;
  }

  bool context::pop_quote(ref<quote>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_quote))
    {
      return false;
    }
    slot = value.cast<quote>();

    return true;
  }

  bool context::pop_symbol(ref<symbol>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_symbol))
    {
      return false;
    }
    slot = value.cast<symbol>();

    return true;
  }

  bool context::pop_word(ref<word>& slot)
  {
    ref<class value> value;

    if (!pop(value, value::type_word))
    {
      return false;
    }
    slot = value.cast<word>();

    return true;
  }
}

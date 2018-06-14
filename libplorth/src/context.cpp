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
#include <plorth/value-word.hpp>

#include "./utils.hpp"

namespace plorth
{
  ref<context> context::make(const ref<class runtime>& runtime)
  {
    return ref<context>(new (runtime->memory_manager()) context(
      runtime
    ));
  }

  context::context(const ref<class runtime>& runtime)
    : m_runtime(runtime.get())
    , m_error(nullptr) {}

  void context::error(enum error::code code,
                      const unistring& message,
                      const struct position* position)
  {
    if (!position && (m_position.filename.empty() || m_position.line > 0))
    {
      position = &m_position;
    }
    m_error = m_runtime->value<class error>(code, message, position).get();
  }

  bool context::peek(reference slot) const
  {
    if (m_data.empty())
    {
      return false;
    }
    slot = m_data.back();

    return true;
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

  void context::push_string(string::const_pointer chars,
                            string::size_type length)
  {
    push(m_runtime->string(chars, length));
  }

  void context::push_array(array::const_pointer elements,
                           array::size_type size)
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

  void context::push_quote(const std::vector<value_type>& values)
  {
    push(m_runtime->compiled_quote(values));
  }

  void context::push_word(const ref<class symbol>& symbol,
                          const ref<class quote>& quote)
  {
    push(m_runtime->word(symbol, quote));
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
      const auto& value = m_data.back();

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

  bool context::pop(reference slot)
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

  bool context::pop(reference slot, enum value::type type)
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

  template< typename T >
  inline bool typed_context_pop(context* ctx,
                                ref<T>& slot,
                                enum value::type type)
  {
    ref<class value> value;

    if (!ctx->pop(value, type))
    {
      return false;
    }
    slot = value.cast<T>();

    return true;
  }

  bool context::pop_number(ref<number>& slot)
  {
    return typed_context_pop<number>(this, slot, value::type_number);
  }

  bool context::pop_string(ref<string>& slot)
  {
    return typed_context_pop<string>(this, slot, value::type_string);
  }

  bool context::pop_array(ref<array>& slot)
  {
    return typed_context_pop<array>(this, slot, value::type_array);
  }

  bool context::pop_object(ref<object>& slot)
  {
    return typed_context_pop<object>(this, slot, value::type_object);
  }

  bool context::pop_quote(ref<quote>& slot)
  {
    return typed_context_pop<quote>(this, slot, value::type_quote);
  }

  bool context::pop_symbol(ref<symbol>& slot)
  {
    return typed_context_pop<symbol>(this, slot, value::type_symbol);
  }

  bool context::pop_word(ref<word>& slot)
  {
    return typed_context_pop<word>(this, slot, value::type_word);
  }

  void context::mark()
  {
    managed::mark();
    if (!m_runtime->marked())
    {
      m_runtime->mark();
    }
    if (m_error && !m_error->marked())
    {
      m_error->mark();
    }
    for (auto& value : m_data)
    {
      if (value && !value->marked())
      {
        value->mark();
      }
    }
    m_dictionary.mark();
  }
}

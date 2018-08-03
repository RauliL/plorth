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
  std::shared_ptr<context> context::make(
    const std::shared_ptr<class runtime>& runtime
  )
  {
    return std::shared_ptr<context>(new (runtime->memory_manager()) context(
      runtime
    ));
  }

  context::context(const std::shared_ptr<class runtime>& runtime)
    : m_runtime(runtime) {}

  void context::error(enum error::code code,
                      const std::u32string& message,
                      const struct position* position)
  {
    if (!position && (m_position.filename.empty() || m_position.line > 0))
    {
      position = &m_position;
    }
    m_error = m_runtime->value<class error>(code, message, position);
  }

  void context::push_null()
  {
    push(std::shared_ptr<value>());
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

  void context::push_number(const std::u32string& value)
  {
    push(m_runtime->number(value));
  }

  void context::push_string(const std::u32string& value)
  {
    push(m_runtime->string(value.c_str(), value.length()));
  }

  void context::push_string(string::const_pointer chars,
                            string::size_type length)
  {
    push(m_runtime->string(chars, length));
  }

  void context::push_array(const std::vector<std::shared_ptr<value>>& elements)
  {
    push_array(elements.data(), elements.size());
  }

  void context::push_array(array::const_pointer elements,
                           array::size_type size)
  {
    push(m_runtime->array(elements, size));
  }

  void context::push_object(const std::vector<object::value_type>& properties)
  {
    push(m_runtime->object(properties));
  }

  void context::push_symbol(const std::u32string& id)
  {
    push(m_runtime->symbol(id));
  }

  void context::push_quote(const std::vector<std::shared_ptr<value>>& values)
  {
    push(m_runtime->compiled_quote(values));
  }

  void context::push_word(const std::shared_ptr<class symbol>& symbol,
                          const std::shared_ptr<class quote>& quote)
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
    error(error::code::range, U"Stack underflow.");

    return false;
  }

  bool context::pop(enum value::type type)
  {
    if (!m_data.empty())
    {
      const auto& value = m_data.back();

      if (!value::is(value, type))
      {
        error(
          error::code::type,
          U"Expected " +
          value::type_description(type) +
          U", got " +
          value::type_description(value ? value->type() : value::type::null) +
          U" instead."
        );

        return false;
      }
      m_data.pop_back();

      return true;
    }
    error(error::code::range, U"Stack underflow.");

    return false;
  }

  bool context::pop(std::shared_ptr<value>& slot)
  {
    if (!m_data.empty())
    {
      slot = m_data.back();
      m_data.pop_back();

      return true;
    }
    error(error::code::range, U"Stack underflow.");

    return false;
  }

  bool context::pop(std::shared_ptr<value>& slot, enum value::type type)
  {
    if (!m_data.empty())
    {
      slot = m_data.back();
      if (!value::is(slot, type))
      {
        error(
          error::code::type,
          U"Expected " +
          value::type_description(type) +
          U", got " +
          value::type_description(slot ? slot->type() : value::type::null) +
          U" instead."
        );

        return false;
      }
      m_data.pop_back();

      return true;
    }
    error(error::code::range, U"Stack underflow.");

    return false;
  }

  bool context::pop_boolean(bool& slot)
  {
    std::shared_ptr<class value> value;

    if (!pop(value, value::type::boolean))
    {
      return false;
    }
    slot = std::static_pointer_cast<boolean>(value)->value();

    return true;
  }

  template< typename T >
  inline bool typed_context_pop(context* ctx,
                                std::shared_ptr<T>& slot,
                                enum value::type type)
  {
    std::shared_ptr<class value> value;

    if (!ctx->pop(value, type))
    {
      return false;
    }
    slot = std::static_pointer_cast<T>(value);

    return true;
  }

  bool context::pop_number(std::shared_ptr<number>& slot)
  {
    return typed_context_pop<number>(this, slot, value::type::number);
  }

  bool context::pop_string(std::shared_ptr<string>& slot)
  {
    return typed_context_pop<string>(this, slot, value::type::string);
  }

  bool context::pop_array(std::shared_ptr<array>& slot)
  {
    return typed_context_pop<array>(this, slot, value::type::array);
  }

  bool context::pop_object(std::shared_ptr<object>& slot)
  {
    return typed_context_pop<object>(this, slot, value::type::object);
  }

  bool context::pop_quote(std::shared_ptr<quote>& slot)
  {
    return typed_context_pop<quote>(this, slot, value::type::quote);
  }

  bool context::pop_symbol(std::shared_ptr<symbol>& slot)
  {
    return typed_context_pop<symbol>(this, slot, value::type::symbol);
  }

  bool context::pop_word(std::shared_ptr<word>& slot)
  {
    return typed_context_pop<word>(this, slot, value::type::word);
  }
}

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
#include <plorth/value-quote.hpp>
#include <plorth/value-string.hpp>

#include "./utils.hpp"

#include <sstream>

namespace plorth
{
  context::context(const ref<class runtime>& runtime)
    : m_runtime(runtime) {}

  void context::error(enum error::code code, const unistring& message)
  {
    m_error = new (m_runtime->memory_manager()) class error(code, message);
  }

  bool context::call(const unistring& word)
  {
    ref<class value> value;

    // Look from prototype of the current item.
    if (!m_data.empty() && m_data.back())
    {
      const ref<object> prototype = m_data.back()->prototype(m_runtime);

      if (prototype && prototype->property(m_runtime, word, value))
      {
        if (value->is(value::type_quote))
        {
          return value.cast<quote>()->call(this);
        }
        m_data.push_back(value);

        return true;
      }
    }

    // Look for a word from dictionary of current context.
    {
      const auto entry = m_dictionary.find(word);

      if (entry != std::end(m_dictionary))
      {
        value = entry->second;
        if (value && value->is(value::type_quote))
        {
          return value.cast<quote>()->call(this);
        }
        m_data.push_back(value);

        return true;
      }
    }

    // TODO: If not found, see if it's a "fully qualified" name, e.g. a name
    // with a namespace name, colon and a word - Such as "num:+", and then look
    // for that from the specified namespace.

    // Look from global dictionary.
    {
      const auto& global_dictionary = m_runtime->dictionary();
      const auto entry = global_dictionary.find(word);

      if (entry != std::end(global_dictionary))
      {
        value = entry->second;
        if (value && value->is(value::type_quote))
        {
          return value.cast<quote>()->call(this);
        }
        m_data.push_back(value);

        return true;
      }
    }

    // If the name of the word can be converted into number, then do just that.
    if (is_number(word))
    {
      push_number(word);

      return true;
    }

    // Otherwise it's reference error.
    error(error::code_reference, "Unrecognized word: `" + word + "'");

    return false;
  }

  void context::declare(const unistring& word, const ref<class quote>& quote)
  {
    m_dictionary[word] = quote;
  }

  void context::push_null()
  {
    push(ref<value>());
  }

  void context::push_boolean(bool value)
  {
    push(m_runtime->boolean(value));
  }

  void context::push_int(std::int64_t value)
  {
    push(m_runtime->number(value));
  }

  void context::push_real(double value)
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

  bool context::pop()
  {
    if (!m_data.empty())
    {
      m_data.pop_back();

      return true;
    }
    error(error::code_range, "Stack underflow.");

    return false;
  }

  bool context::pop(enum value::type type)
  {
    if (!m_data.empty())
    {
      const ref<class value>& value = m_data.back();

      if ((!value && type != value::type_null) || (value && !value->is(type)))
      {
        std::stringstream ss;

        ss << "Expected " << type << ", got ";
        if (value)
        {
          ss << value->type();
        } else {
          ss << "null";
        }
        ss << " instead.";
        error(error::code_type, ss.str().c_str());

        return false;
      }
      m_data.pop_back();

      return true;
    }
    error(error::code_range, "Stack underflow.");

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
    error(error::code_range, "Stack underflow.");

    return false;
  }

  bool context::pop(ref<value>& slot, enum value::type type)
  {
    if (!m_data.empty())
    {
      slot = m_data.back();
      if ((!slot && type != value::type_null) || (slot && !slot->is(type)))
      {
        std::stringstream ss;

        ss << "Expected " << type << ", got ";
        if (slot)
        {
          ss << slot->type();
        } else {
          ss << "null";
        }
        ss << " instead.";
        error(error::code_type, ss.str().c_str());

        return false;
      }
      m_data.pop_back();

      return true;
    }
    error(error::code_range, "Stack underflow.");

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
}

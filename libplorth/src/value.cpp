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

namespace plorth
{
  unistring value::type_description() const
  {
    return type_description(type());
  }

  unistring value::type_description(enum type type)
  {
    switch (type)
    {
    case value::type_null:
      return U"null";

    case value::type_boolean:
      return U"boolean";

    case value::type_number:
      return U"number";

    case value::type_string:
      return U"string";

    case value::type_array:
      return U"array";

    case value::type_object:
      return U"object";

    case value::type_symbol:
      return U"symbol";

    case value::type_quote:
      return U"quote";

    case value::type_word:
      return U"word";

    case value::type_error:
      return U"error";
    }

    return U"unknown";
  }

  ref<object> value::prototype(const ref<class runtime>& runtime) const
  {
    switch (type())
    {
      case type_null:
        return runtime->object_prototype();

      case type_boolean:
        return runtime->boolean_prototype();

      case type_number:
        return runtime->number_prototype();

      case type_string:
        return runtime->string_prototype();

      case type_array:
        return runtime->array_prototype();

      case type_symbol:
        return runtime->symbol_prototype();

      case type_quote:
        return runtime->quote_prototype();

      case type_word:
        return runtime->word_prototype();

      case type_error:
        return runtime->error_prototype();

      case type_object:
        {
          const auto& properties = static_cast<const object*>(this)->properties();
          const auto property = properties.find(U"__proto__");

          if (property == std::end(properties))
          {
            return runtime->object_prototype();
          }
          else if (property->second && property->second->is(type_object))
          {
            return property->second.cast<object>();
          } else {
            return ref<object>();
          }
        }
        break;
    }

    return ref<object>(); // Just to make GCC happy.
  }

  bool operator==(const ref<value>& a, const ref<value>& b)
  {
    return a ? b && a->equals(b) : !b;
  }

  bool operator!=(const ref<value>& a, const ref<value>& b)
  {
    return a ? !b || !a->equals(b) : !!b;
  }

  std::ostream& operator<<(std::ostream& out, enum value::type type)
  {
    out << value::type_description(type);

    return out;
  }

  std::ostream& operator<<(std::ostream& os, const ref<class value>& value)
  {
    return os << value.get();
  }

  std::ostream& operator<<(std::ostream& os, const class value* value)
  {
    if (value)
    {
      os << value->to_string();
    } else {
      os << "<no value>";
    }

    return os;
  }
}

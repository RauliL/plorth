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
  std::u32string value::type_description() const
  {
    return type_description(type());
  }

  std::u32string value::type_description(enum type type)
  {
    switch (type)
    {
    case type::null:
      return U"null";

    case type::boolean:
      return U"boolean";

    case type::number:
      return U"number";

    case type::string:
      return U"string";

    case type::array:
      return U"array";

    case type::object:
      return U"object";

    case type::symbol:
      return U"symbol";

    case type::quote:
      return U"quote";

    case type::word:
      return U"word";

    case type::error:
      return U"error";
    }

    return U"unknown";
  }

  std::shared_ptr<object> value::prototype(
    const std::shared_ptr<class runtime>& runtime
  ) const
  {
    switch (type())
    {
    case type::null:
      return runtime->object_prototype();

    case type::boolean:
      return runtime->boolean_prototype();

    case type::number:
      return runtime->number_prototype();

    case type::string:
      return runtime->string_prototype();

    case type::array:
      return runtime->array_prototype();

    case type::symbol:
      return runtime->symbol_prototype();

    case type::quote:
      return runtime->quote_prototype();

    case type::word:
      return runtime->word_prototype();

    case type::error:
      return runtime->error_prototype();

    case type::object:
      {
        std::shared_ptr<value> slot;

        if (static_cast<const object*>(this)->own_property(U"__proto__", slot))
        {
          if (is(slot, type::object))
          {
            return std::static_pointer_cast<object>(slot);
          } else {
            return std::shared_ptr<object>();
          }
        }

        return runtime->object_prototype();
      }
      break;
    }

    return std::shared_ptr<object>(); // Just to make GCC happy.
  }

  bool operator==(const std::shared_ptr<value>& a,
                  const std::shared_ptr<value>& b)
  {
    return a ? b && a->equals(b) : !b;
  }

  bool operator!=(const std::shared_ptr<value>& a,
                  const std::shared_ptr<value>& b)
  {
    return a ? !b || !a->equals(b) : !!b;
  }

  std::ostream& operator<<(std::ostream& out, enum value::type type)
  {
    out << utf8_encode(value::type_description(type));

    return out;
  }

  std::ostream& operator<<(std::ostream& os, const class value* value)
  {
    if (value)
    {
      os << utf8_encode(value->to_string());
    } else {
      os << "<no value>";
    }

    return os;
  }
}

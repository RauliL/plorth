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
#include <plorth/value-string.hpp>

#include "./utils.hpp"

namespace plorth
{
  object::object() {}

  object::object(const std::vector<value_type>& properties)
  {
    for (const auto& property : properties)
    {
      m_properties[property.first] = property.second.get();
    }
  }

  std::vector<object::key_type> object::keys() const
  {
    std::vector<key_type> result;

    result.reserve(m_properties.size());
    for (const auto& property : m_properties)
    {
      result.push_back(property.first);
    }

    return result;
  }

  std::vector<object::mapped_type> object::values() const
  {
    std::vector<mapped_type> result;

    result.reserve(m_properties.size());
    for (const auto& property : m_properties)
    {
      result.push_back(mapped_type(property.second));
    }

    return result;
  }

  std::vector<object::value_type> object::entries() const
  {
    std::vector<value_type> result;

    result.reserve(m_properties.size());
    for (const auto& property : m_properties)
    {
      result.push_back(value_type(
        property.first,
        mapped_type(property.second)
      ));
    }

    return result;
  }

  bool object::property(const ref<class runtime>& runtime,
                        const unistring& name,
                        ref<value>& slot,
                        bool inherited) const
  {
    const auto property = m_properties.find(name);

    if (property != std::end(m_properties))
    {
      slot = mapped_type(property->second);

      return true;
    }
    if (inherited)
    {
      const auto proto = prototype(runtime);

      if (proto && this != proto.get())
      {
        return proto->property(runtime, name, slot, true);
      }
    }

    return false;
  }

  bool object::own_property(const unistring& name, ref<value>& slot) const
  {
    const auto property = m_properties.find(name);

    if (property == std::end(m_properties))
    {
      return false;
    }
    slot = mapped_type(property->second);

    return true;
  }

  bool object::equals(const ref<value>& that) const
  {
    ref<object> obj;

    if (!that || !that->is(type_object))
    {
      return false;
    }

    if (this == (obj = that.cast<object>()).get())
    {
      return true;
    }
    else if (m_properties.size() != obj->m_properties.size())
    {
      return false;
    }

    for (auto entry1 : m_properties)
    {
      const auto entry2 = obj->m_properties.find(entry1.first);

      if (entry2 != std::end(obj->m_properties))
      {
        const auto ref1 = mapped_type(entry1.second);
        const auto ref2 = mapped_type(entry2->second);

        if (ref1 != ref2)
        {
          return false;
        }
      } else {
        return false;
      }
    }

    return true;
  }

  unistring object::to_string() const
  {
    unistring result;
    bool first = true;

    for (const auto& property : m_properties)
    {
      if (first)
      {
        first = false;
      } else {
        result += ',';
        result += ' ';
      }
      result += property.first;
      result += '=';
      if (property.second)
      {
        result += property.second->to_string();
      }
    }

    return result;
  }

  unistring object::to_source() const
  {
    unistring result;
    bool first = true;

    result += '{';
    for (const auto& property : m_properties)
    {
      if (first)
      {
        first = false;
      } else {
        result += ',';
        result += ' ';
      }
      result += json_stringify(property.first);
      result += ':';
      result += ' ';
      if (property.second)
      {
        result += property.second->to_source();
      } else {
        result += U"null";
      }
    }
    result += '}';

    return result;
  }

  void object::mark()
  {
    value::mark();
    for (auto& property : m_properties)
    {
      if (property.second && !property.second->marked())
      {
        property.second->mark();
      }
    }
  }

  ref<object> runtime::object(
    const std::vector<object::value_type>& properties
  )
  {
    return ref<class object>(new (*m_memory_manager) class object(properties));
  }

  /**
   * Word: keys
   * Prototype: object
   *
   * Takes:
   * - object
   *
   * Gives:
   * - object
   * - array
   *
   * Retrieves all keys from the object and returns them in an array. Notice
   * that inherited properties are not included in the list.
   */
  static void w_keys(const ref<context>& ctx)
  {
    const auto runtime = ctx->runtime();
    ref<object> obj;
    std::vector<ref<value>> result;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    for (const auto& key : obj->keys())
    {
      result.push_back(runtime->string(key));
    }

    ctx->push(obj);
    ctx->push_array(result.data(), result.size());
  }

  /**
   * Word: values
   * Prototype: object
   *
   * Takes:
   * - object
   *
   * Gives:
   * - object
   * - array
   *
   * Retrieves all values from the object and returns them in an array. Notice
   * that inherited properties are not included in the list.
   */
  static void w_values(const ref<context>& ctx)
  {
    ref<object> obj;

    if (ctx->pop_object(obj))
    {
      const auto values = obj->values();

      ctx->push(obj);
      ctx->push_array(values.data(), values.size());
    }
  }

  /**
   * Word: entries
   * Prototype: object
   *
   * Takes:
   * - object
   *
   * Gives:
   * - object
   * - array
   *
   * Constructs an array of arrays where each non-inherited property in the
   * object is represented as an pair (i.e. array containing two elements, one
   * for the key and one for the value).
   */
  static void w_entries(const ref<context>& ctx)
  {
    const auto runtime = ctx->runtime();
    ref<object> obj;
    std::vector<ref<value>> result;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    for (const auto& property : obj->entries())
    {
      ref<value> pair[2];

      pair[0] = runtime->string(property.first);
      pair[1] = property.second;
      result.push_back(runtime->array(pair, 2));
    }

    ctx->push(obj);
    ctx->push_array(result.data(), result.size());
  }

  /**
   * Word: has?
   * Prototype: object
   *
   * Takes:
   * - string
   * - object
   *
   * Gives:
   * - object
   * - boolean
   *
   * Tests whether the object has property with given identifier. Notice that
   * inherited properties are also included in the search.
   */
  static void w_has(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      ref<value> slot;

      ctx->push(obj);
      ctx->push_boolean(!!obj->property(ctx->runtime(), id->to_string(), slot));
    }
  }

  /**
   * Word: has-own?
   * Prototype: object
   *
   * Takes:
   * - string
   * - object
   *
   * Gives:
   * - object
   * - boolean
   *
   * Tests whether the object has own property with given identifier. Inherited
   * properties are not included in the search.
   */
  static void w_has_own(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      ref<value> slot;

      ctx->push(obj);
      ctx->push_boolean(obj->own_property(id->to_string(), slot));
    }
  }

  /**
   * Word: new
   * Prototype: object
   *
   * Takes:
   * - any...
   *
   * Gives:
   * - object
   *
   * Constructs a new instance of the object and invokes its constructor if it
   * has one with the newly constructed object pushed on top of the stack.
   *
   * Type error will be thrown if the object has no "prototype" property.
   */
  static void w_new(const ref<context>& ctx)
  {
    const auto runtime = ctx->runtime();
    ref<object> obj;
    ref<value> prototype;
    ref<value> constructor;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    if (!obj->property(runtime, U"prototype", prototype, false)
        || !prototype->is(value::type_object))
    {
      ctx->error(error::code_type, U"Object has no prototype.");
      return;
    }

    ctx->push_object({ { U"__proto__", prototype } });

    if (prototype.cast<object>()->property(runtime,
                                           U"constructor",
                                           constructor)
        && constructor->is(value::type_quote))
    {
      constructor.cast<quote>()->call(ctx);
    }
  }

  /**
   * Word: @
   * Prototype: object
   *
   * Takes:
   * - string
   * - object
   *
   * Gives:
   * - object
   * - any
   *
   * Retrieves the value identified by given string from properties of the
   * object. If the object does not have such a property, range error will be
   * thrown. Notice that inherited properties are also included in the search.
   */
  static void w_get(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      ref<value> val;

      ctx->push(obj);
      if (obj->property(ctx->runtime(), id->to_string(), val))
      {
        ctx->push(val);
      } else {
        ctx->error(
          error::code_range,
          U"No such property: `" + id->to_string() + U"'"
        );
      }
    }
  }

  /**
   * Word: !
   * Prototype: object
   *
   * Takes:
   * - any
   * - string
   * - object
   *
   * Gives:
   * - object
   *
   * Constructs a copy of the object with new named property either introduced
   * or replaced.
   */
  static void w_set(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;
    ref<value> val;

    if (ctx->pop_object(obj) && ctx->pop_string(id) && ctx->pop(val))
    {
      auto properties = obj->entries();

      properties.push_back(std::make_pair(id->to_string(), val));
      ctx->push_object(properties);
    }
  }

  /**
   * Word: delete
   * Prototype: object
   *
   * Takes:
   * - string
   * - object
   *
   * Gives:
   * - object
   *
   * Constructs a copy of the object with the named property removed.
   */
  static void w_delete(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      const auto id_str = id->to_string();
      auto result = obj->entries();
      bool found = false;

      for (auto it = result.begin(); it != std::end(result); ++it)
      {
        if (it->first == id_str)
        {
          found = true;
          result.erase(it);
          break;
        }
      }
      if (!found)
      {
        ctx->error(
          error::code_range,
          U"No such property: `" + id->to_string() + U"'"
        );
      } else {
        ctx->push_object(result);
      }
    }
  }

  /**
   * Word: +
   * Prototype: object
   *
   * Takes:
   * - object
   * - object
   *
   * Gives:
   * - object
   *
   * Combines the contents of two objects together and returns the result. If
   * the two objects share keys the second object's values take precedence.
   */
  static void w_concat(const ref<context>& ctx)
  {
    ref<object> a;
    ref<object> b;

    if (ctx->pop_object(a) && ctx->pop_object(b))
    {
      auto result = b->entries();

      for (const auto& entry : a->entries())
      {
        result.push_back(entry);
      }

      ctx->push_object(result);
    }
  }

  namespace api
  {
    runtime::prototype_definition object_prototype()
    {
      return
      {
        { U"keys", w_keys },
        { U"values", w_values },
        { U"entries", w_entries },
        { U"has?", w_has },
        { U"has-own?", w_has_own },
        { U"new", w_new },
        { U"@", w_get },
        { U"!", w_set },
        { U"delete", w_delete },
        { U"+", w_concat }
      };
    }
  }
}

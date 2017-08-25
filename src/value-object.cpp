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
#include <plorth/value-string.hpp>

#include "./utils.hpp"

namespace plorth
{
  object::object(const container_type& properties)
    : m_properties(properties) {}

  bool object::property(const ref<class runtime>& runtime,
                        const unistring& name,
                        ref<value>& slot,
                        bool inherited) const
  {
    const auto property = m_properties.find(name);

    if (property != std::end(m_properties))
    {
      slot = property->second;

      return true;
    }
    if (inherited)
    {
      const ref<object> proto = prototype(runtime);

      if (proto && this != proto.get())
      {
        return proto->property(runtime, name, slot, true);
      }
    }

    return false;
  }

  bool object::equals(const ref<value>& that) const
  {
    const object* obj;

    if (!that || !that->is(type_object))
    {
      return false;
    }

    obj = that.cast<object>();

    if (m_properties.size() != obj->m_properties.size())
    {
      return false;
    }

    for (auto entry1 : m_properties)
    {
      const auto entry2 = obj->m_properties.find(entry1.first);

      if (entry2 == std::end(obj->m_properties) || entry1.second != entry2->second)
      {
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
    const ref<class runtime>& runtime = ctx->runtime();
    ref<object> obj;
    std::vector<ref<value>> result;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    for (const auto& property : obj->properties())
    {
      result.push_back(runtime->string(property.first));
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
    std::vector<ref<value>> result;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    for (const auto& property : obj->properties())
    {
      result.push_back(property.second);
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
      const auto& properties = obj->properties();

      ctx->push(obj);
      ctx->push_boolean(properties.find(id->to_string()) != std::end(properties));
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
    const ref<class runtime>& runtime = ctx->runtime();
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

    if (prototype.cast<object>()->property(runtime, U"constructor", constructor)
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
      object::container_type result = obj->properties();

      result[id->to_string()] = val;
      ctx->push_object(result);
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
      object::container_type result = obj->properties();

      if (result.erase(id->to_string()) < 1)
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
      object::container_type result = b->properties();

      for (const auto& entry : a->properties())
      {
        result[entry.first] = entry.second;
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

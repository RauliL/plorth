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
  namespace
  {
    class simple_object : public object
    {
    public:
      using container_type = std::unordered_map<key_type, mapped_type>;

      template<class InputIt>
      explicit simple_object(InputIt first, InputIt last)
        : m_container(first, last) {}

      bool has_own_property(const key_type& key) const
      {
        return m_container.find(key) != std::end(m_container);
      }

      bool own_property(const key_type& key, mapped_type& slot) const
      {
        const auto property = m_container.find(key);

        if (property != std::end(m_container))
        {
          slot = property->second;

          return true;
        }

        return false;
      }

      size_type size() const
      {
        return m_container.size();
      }

      std::vector<key_type> keys() const
      {
        std::vector<key_type> result;

        result.reserve(m_container.size());
        for (const auto& property : m_container)
        {
          result.push_back(property.first);
        }

        return result;
      }

      std::vector<mapped_type> values() const
      {
        std::vector<mapped_type> result;

        result.reserve(m_container.size());
        for (const auto& property : m_container)
        {
          result.push_back(property.second);
        }

        return result;
      }

      std::vector<value_type> entries() const
      {
        return std::vector<value_type>(
          std::begin(m_container),
          std::end(m_container)
        );
      }

    private:
      const container_type m_container;
    };

    class set_object : public object
    {
    public:
      explicit set_object(const std::shared_ptr<class object>& object,
                          const key_type& key,
                          const mapped_type& value)
        : m_object(object)
        , m_key(key)
        , m_value(value) {}

      bool has_own_property(const key_type& key) const
      {
        return key == m_key || m_object->has_own_property(key);
      }

      bool own_property(const key_type& key, mapped_type& slot) const
      {
        if (key == m_key)
        {
          slot = m_value;

          return true;
        }

        return m_object->own_property(key, slot);
      }

      size_type size() const
      {
        return m_object->size() + 1;
      }

      std::vector<key_type> keys() const
      {
        auto keys = m_object->keys();

        keys.push_back(m_key);

        return keys;
      }

      std::vector<mapped_type> values() const
      {
        auto values = m_object->values();

        values.push_back(m_value);

        return values;
      }

      std::vector<value_type> entries() const
      {
        auto entries = m_object->entries();

        entries.push_back({ m_key, m_value });

        return entries;
      }

    private:
      const std::shared_ptr<object> m_object;
      const key_type m_key;
      const mapped_type m_value;
    };

    class set_object_override : public object
    {
    public:
      explicit set_object_override(const std::shared_ptr<class object>& object,
                                   const key_type& key,
                                   const mapped_type& value)
        : m_object(object)
        , m_key(key)
        , m_value(value) {}

      bool has_own_property(const key_type& key) const
      {
        return key == m_key || m_object->has_own_property(key);
      }

      bool own_property(const key_type& key, mapped_type& slot) const
      {
        if (key == m_key)
        {
          slot = m_value;

          return true;
        }

        return m_object->own_property(key, slot);
      }

      size_type size() const
      {
        return m_object->size();
      }

      std::vector<key_type> keys() const
      {
        return m_object->keys();
      }

      std::vector<mapped_type> values() const
      {
        std::vector<mapped_type> result;

        result.reserve(m_object->size());
        for (const auto property : m_object->entries())
        {
          if (property.first == m_key)
          {
            result.push_back(m_value);
          } else {
            result.push_back(property.second);
          }
        }

        return result;
      }

      std::vector<value_type> entries() const
      {
        std::vector<value_type> result;

        result.reserve(m_object->size());
        for (const auto property : m_object->entries())
        {
          if (property.first == m_key)
          {
            result.push_back({ m_key, m_value });
          } else {
            result.push_back(property);
          }
        }

        return result;
      }

    private:
      const std::shared_ptr<object> m_object;
      const key_type m_key;
      const mapped_type m_value;
    };

    class delete_object : public object
    {
    public:
      explicit delete_object(const std::shared_ptr<class object>& object,
                             const key_type& removed_key)
        : m_object(object)
        , m_removed_key(removed_key) {}

      bool has_own_property(const key_type& key) const
      {
        return m_removed_key != key && m_object->has_own_property(key);
      }

      bool own_property(const key_type& key, mapped_type& slot) const
      {
        return key != m_removed_key && m_object->own_property(key, slot);
      }

      size_type size() const
      {
        return m_object->size() - 1;
      }

      std::vector<key_type> keys() const
      {
        std::vector<key_type> result;

        for (const auto& key : m_object->keys())
        {
          if (key != m_removed_key)
          {
            result.push_back(key);
          }
        }

        return result;
      }

      std::vector<mapped_type> values() const
      {
        std::vector<mapped_type> result;

        for (const auto& property : m_object->entries())
        {
          if (property.first != m_removed_key)
          {
            result.push_back(property.second);
          }
        }

        return result;
      }

      std::vector<value_type> entries() const
      {
        std::vector<value_type> result;

        for (const auto& property : m_object->entries())
        {
          if (property.first != m_removed_key)
          {
            result.push_back(property);
          }
        }

        return result;
      }

    private:
      const std::shared_ptr<object> m_object;
      const key_type m_removed_key;
    };
  }

  bool object::has_property(const std::shared_ptr<class runtime>& runtime,
                            const key_type& key) const
  {
    if (!has_own_property(key))
    {
      const auto proto = prototype(runtime);

      if (proto && this != proto.get())
      {
        return proto->has_property(runtime, key);
      }

      return false;
    }

    return true;
  }

  bool object::property(const std::shared_ptr<class runtime>& runtime,
                        const key_type& key,
                        mapped_type& slot) const
  {
    if (!own_property(key, slot))
    {
      const auto proto = prototype(runtime);

      if (proto && this != proto.get())
      {
        return proto->property(runtime, key, slot);
      }

      return false;
    }

    return true;
  }

  bool object::equals(const std::shared_ptr<value>& that) const
  {
    std::shared_ptr<object> obj;
    std::shared_ptr<value> slot;

    if (!is(that, type::object))
    {
      return false;
    }

    if (this == (obj = std::static_pointer_cast<object>(that)).get())
    {
      return true;
    }
    else if (size() != obj->size())
    {
      return false;
    }

    for (auto property : entries())
    {
      if (!obj->own_property(property.first, slot) || property.second != slot)
      {
        return false;
      }
    }

    return true;
  }

  std::u32string object::to_string() const
  {
    std::u32string result;
    bool first = true;

    for (const auto property : entries())
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

  std::u32string object::to_source() const
  {
    std::u32string result;
    bool first = true;

    result += '{';
    for (const auto property : entries())
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

  std::shared_ptr<object> runtime::object(
    const std::vector<object::value_type>& properties
  )
  {
    return std::shared_ptr<class object>(
      new (*m_memory_manager) simple_object(
        std::begin(properties),
        std::end(properties)
      )
    );
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
  static void w_keys(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<object> obj;
    std::vector<std::shared_ptr<value>> result;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    result.reserve(obj->size());
    for (const auto key : obj->keys())
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
  static void w_values(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> obj;

    if (ctx->pop_object(obj))
    {
      ctx->push(obj);
      ctx->push_array(obj->values());
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
  static void w_entries(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<object> obj;
    std::vector<std::shared_ptr<value>> result;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    for (const auto property : obj->entries())
    {
      std::shared_ptr<value> pair[2];

      pair[0] = runtime->string(property.first);
      pair[1] = property.second;
      result.push_back(runtime->array(pair, 2));
    }

    ctx->push(obj);
    ctx->push_array(result);
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
  static void w_has(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> obj;
    std::shared_ptr<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      ctx->push(obj);
      ctx->push_boolean(obj->has_property(ctx->runtime(), id->to_string()));
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
  static void w_has_own(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> obj;
    std::shared_ptr<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      ctx->push(obj);
      ctx->push_boolean(obj->has_own_property(id->to_string()));
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
  static void w_new(const std::shared_ptr<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    std::shared_ptr<object> obj;
    std::shared_ptr<value> prototype;
    std::shared_ptr<value> constructor;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    if (!obj->own_property(U"prototype", prototype)
        || !value::is(prototype, value::type::object))
    {
      ctx->error(error::code::type, U"Object has no prototype.");
      return;
    }

    ctx->push_object({ { U"__proto__", prototype } });

    if (std::static_pointer_cast<object>(prototype)->property(runtime,
                                                              U"constructor",
                                                              constructor)
        && value::is(constructor, value::type::quote))
    {
      std::static_pointer_cast<quote>(constructor)->call(ctx);
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
  static void w_get(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> obj;
    std::shared_ptr<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      std::shared_ptr<value> val;

      ctx->push(obj);
      if (obj->property(ctx->runtime(), id->to_string(), val))
      {
        ctx->push(val);
      } else {
        ctx->error(
          error::code::range,
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
  static void w_set(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> obj;
    std::shared_ptr<string> id;
    std::shared_ptr<value> val;

    if (ctx->pop_object(obj) && ctx->pop_string(id) && ctx->pop(val))
    {
      const auto key = id->to_string();
      std::shared_ptr<object> result;

      if (obj->has_own_property(key))
      {
        result = ctx->runtime()->value<set_object_override>(obj, key, val);
      } else {
        result = ctx->runtime()->value<set_object>(obj, key, val);
      }
      ctx->push(result);
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
  static void w_delete(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> obj;
    std::shared_ptr<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      const auto name = id->to_string();

      if (!obj->has_own_property(name))
      {
        ctx->error(
          error::code::range,
          U"No such property: `" + name + U"'"
        );
      }
      ctx->push(ctx->runtime()->value<delete_object>(obj, name));
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
  static void w_concat(const std::shared_ptr<context>& ctx)
  {
    std::shared_ptr<object> a;
    std::shared_ptr<object> b;

    if (ctx->pop_object(a) && ctx->pop_object(b))
    {
      const auto entries = b->entries();
      std::unordered_map<std::u32string, std::shared_ptr<value>> properties(
        std::begin(entries),
        std::end(entries)
      );

      for (const auto property : a->entries())
      {
        properties[property.first] = property.second;
      }
      ctx->push_object(std::vector<object::value_type>(
        std::begin(properties),
        std::end(properties)
      ));
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

#include <plorth/context.hpp>
#include <plorth/value-string.hpp>

#include "./utils.hpp"

namespace plorth
{
  object::object(const container_type& properties)
    : m_properties(properties) {}

  ref<value> object::property(const unistring& name) const
  {
    auto entry = m_properties.find(name);

    if (entry != std::end(m_properties))
    {
      return entry->second;
    }

    entry = m_properties.find(utf8_decode("__proto__"));
    if (entry != std::end(m_properties) && entry->second->is(type_object))
    {
      return entry->second.cast<object>()->property(name);
    }

    return ref<value>();
  }

  bool object::equals(const ref<value>& that) const
  {
    const object* obj;

    if (!that->is(type_object))
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

      if (entry2 == std::end(obj->m_properties)
          || !entry1.second->equals(entry2->second))
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
      result += property.second->to_string();
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
      result += property.second->to_source();
    }
    result += '}';

    return result;
  }

  /**
   * keys ( object -- object array )
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
      result.push_back(runtime->value<string>(property.first));
    }

    ctx->push(obj);
    ctx->push_array(result);
  }

  /**
   * values ( object -- object array )
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
    ctx->push_array(result);
  }

  /**
   * has? ( string object -- object boolean )
   *
   * Tests whether object has property with given identifier. Notice that
   * inherited properties are also included in the search.
   */
  static void w_has(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      ctx->push(obj);
      ctx->push_boolean(!!obj->property(id->value()));
    }
  }

  /**
   * has-own? ( string object -- object boolean )
   *
   * Tests whether object has own property with given identifier. Inherited
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
      ctx->push_boolean(properties.find(id->value()) != std::end(properties));
    }
  }

  /**
   * new ( any... -- object )
   *
   * Constructs new instance of the object and invokes it's constructor if it
   * has one with the newly constructed object pushed into top of the stack.
   *
   * Type error will be thrown if the object has no "prototype" property.
   */
  static void w_new(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<value> prototype;
    ref<value> constructor;

    if (!ctx->pop_object(obj))
    {
      return;
    }

    prototype = obj->property(utf8_decode("prototype"));
    if (!prototype || !prototype->is(value::type_object))
    {
      ctx->error(error::code_type, "Object has no prototype.");
      return;
    }

    ctx->push_object({ { utf8_decode("__proto__"), prototype } });

    constructor = prototype.cast<object>()->property(utf8_decode("constructor"));
    if (constructor && constructor->is(value::type_quote))
    {
      constructor.cast<quote>()->call(ctx);
    }
  }

  /**
   * @ ( string object -- object any )
   *
   * Retrieves value identified by given string from properties of the object.
   * If the object does not have such property, range error will be thrown.
   * Notice that inherited properties are also included in the search.
   */
  static void w_get(const ref<context>& ctx)
  {
    ref<object> obj;
    ref<string> id;

    if (ctx->pop_object(obj) && ctx->pop_string(id))
    {
      const ref<value> val = obj->property(id->value());

      ctx->push(obj);
      if (val)
      {
        ctx->push(val);
      } else {
        ctx->error(
          error::code_range,
          "No such property: `" + id->value() + "'"
        );
      }
    }
  }

  /**
   * ! ( any string object -- object )
   *
   * Constructs copy of the object with new named property either introduced
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

      result[id->value()] = val;
      ctx->push_object(result);
    }
  }

  /**
   * + ( object object -- object )
   *
   * Combines contents of two objects together and returns result.
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
        { "keys", w_keys },
        { "values", w_values },
        { "has?", w_has },
        { "has-own?", w_has_own },
        { "new", w_new },
        { "@", w_get },
        { "!", w_set },
        { "+", w_concat }
      };
    }
  }
}

#include "context.hpp"
#include "string.hpp"

namespace plorth
{
  /**
   * len ( obj -- obj num )
   *
   * Returns the number of key-value pairs from the object.
   */
  static void w_len(const Ref<Context>& context)
  {
    Ref<Object> object;

    if (context->PeekObject(object))
    {
      context->PushNumber(static_cast<std::int64_t>(object->GetOwnProperties().size()));
    }
  }

  /**
   * keys ( obj -- obj ary )
   *
   * Retrieves all keys from the object and returns them in an array.
   */
  static void w_keys(const Ref<Context>& context)
  {
    Ref<Object> object;
    std::vector<Ref<Value>> result;

    if (!context->PeekObject(object))
    {
      return;
    }
    for (const auto& property : object->GetOwnProperties())
    {
      result.push_back(context->GetRuntime()->NewString(property.first));
    }
    context->PushArray(result);
  }

  /**
   * values ( obj -- obj ary )
   *
   * Retrieves all values from the object and returns them in an array.
   */
  static void w_values(const Ref<Context>& context)
  {
    Ref<Object> object;
    std::vector<Ref<Value>> result;

    if (!context->PeekObject(object))
    {
      return;
    }
    for (const auto& property : object->GetOwnProperties())
    {
      result.push_back(property.second);
    }
    context->PushArray(result);
  }

  /**
   * has? ( str obj -- bool )
   *
   * Tests whether value identified by given string exists in the object.
   */
  static void w_has(const Ref<Context>& context)
  {
    Ref<Object> object;
    Ref<String> key;

    if (context->PopObject(object) && context->PopString(key))
    {
      const Ref<Value> value = object->GetOwnProperty(key->GetValue());

      context->PushBool(!!value);
    }
  }

  /**
   * @ ( str obj -- any )
   *
   * Retrieves value identified by given string from the object. If the object
   * does not have value for the identifier, null is returned instead.
   */
  static void w_get(const Ref<Context>& context)
  {
    Ref<Object> object;
    Ref<String> key;

    if (context->PopObject(object) && context->PopString(key))
    {
      const Ref<Value> value = object->GetOwnProperty(key->GetValue());

      if (value)
      {
        context->Push(value);
      } else {
        context->Push(context->GetRuntime()->GetNullValue());
      }
    }
  }

  /**
   * ! ( any str obj -- obj )
   *
   * Assigns named value to the object and returns result.
   */
  static void w_set(const Ref<Context>& context)
  {
    Ref<Object> object;
    Ref<String> key;
    Ref<Value> value;

    if (context->PopObject(object)
        && context->PopString(key)
        && context->Pop(value))
    {
      Object::Dictionary properties = object->GetOwnProperties();

      properties[key->GetValue()] = value;
      context->PushObject(properties);
    }
  }

  /**
   * + ( obj obj -- obj )
   *
   * Combines contents of two objects together and returns result.
   */
  static void w_plus(const Ref<Context>& context)
  {
    Ref<Object> obj_a;
    Ref<Object> obj_b;

    if (context->PopObject(obj_a) && context->PopObject(obj_b))
    {
      Object::Dictionary result = obj_b->GetOwnProperties();

      for (const auto& property : obj_a->GetOwnProperties())
      {
        result[property.first] = property.second;
      }
      context->PushObject(result);
    }
  }

  Ref<Object> make_object_prototype(Runtime* runtime)
  {
    return runtime->NewPrototype({
      { "len", w_len },
      { "keys", w_keys },
      { "values", w_values },

      { "has?", w_has },

      { "@", w_get },
      { "!", w_set },

      { "+", w_plus },
    });
  }
}

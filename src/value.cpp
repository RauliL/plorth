#include <plorth/runtime.hpp>

#include <sstream>

namespace plorth
{
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

      case type_quote:
        return runtime->quote_prototype();

      case type_error:
        return runtime->error_prototype();

      case type_object:
        {
          const auto property = static_cast<const object*>(this)->property(utf8_decode("__proto__"));

          if (property && property->is(type_object))
          {
            return property.cast<object>();
          }
        }
        break;
    }

    return runtime->object_prototype();
  }

  std::ostream& operator<<(std::ostream& os, enum value::type type)
  {
    switch (type)
    {
      case value::type_null:
        os << "null";
        break;

      case value::type_boolean:
        os << "boolean";
        break;

      case value::type_number:
        os << "number";
        break;

      case value::type_string:
        os << "string";
        break;

      case value::type_array:
        os << "array";
        break;

      case value::type_object:
        os << "object";
        break;

      case value::type_quote:
        os << "quote";
        break;

      case value::type_error:
        os << "error";
        break;

    }

    return os;
  }

  std::ostream& operator<<(std::ostream& os, const ref<class value>& value)
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

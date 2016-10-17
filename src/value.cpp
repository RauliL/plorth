#include "array.hpp"
#include "bool.hpp"
#include "null.hpp"
#include "object.hpp"
#include "string.hpp"

namespace plorth
{
  Value::Value() {}

  Ref<Object> Value::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return Ref<Object>();
  }

  std::string Value::ToSource() const
  {
    return ToString();
  }

  Null::Null() {}

  bool Null::Equals(const Ref<Value>& that) const
  {
    return that->GetType() == TYPE_NULL;
  }

  std::string Null::ToString() const
  {
    return std::string();
  }

  std::string Null::ToSource() const
  {
    return "null";
  }

  std::ostream& operator<<(std::ostream& os, const Ref<Value>& value)
  {
    if (value)
    {
      os << value->ToString();
    } else {
      os << "<no value>";
    }

    return os;
  }

  std::ostream& operator<<(std::ostream& os, Value::Type type)
  {
    switch (type)
    {
      case Value::TYPE_NULL:
        os << "null";
        break;

      case Value::TYPE_BOOL:
        os << "bool";
        break;

      case Value::TYPE_NUMBER:
        os << "num";
        break;

      case Value::TYPE_STRING:
        os << "str";
        break;

      case Value::TYPE_ARRAY:
        os << "ary";
        break;

      case Value::TYPE_OBJECT:
        os << "obj";
        break;

      case Value::TYPE_QUOTE:
        os << "quote";
        break;

      case Value::TYPE_ERROR:
        os << "error";
        break;
    }

    return os;
  }
}

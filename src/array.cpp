#include "array.hpp"
#include "context.hpp"

namespace plorth
{
  Array::Array(const std::vector<Ref<Value>>& elements)
    : m_elements(elements) {}

  Ref<Object> Array::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetArrayPrototype();
  }

  bool Array::Equals(const Ref<Value>& that) const
  {
    Ref<Array> other;

    if (that->GetType() != TYPE_ARRAY)
    {
      return false;
    }
    other = that.As<Array>();
    if (m_elements.size() != other->m_elements.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (!m_elements[i]->Equals(other->m_elements[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::string Array::ToString() const
  {
    std::string result;

    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += ", ";
      }
      result += m_elements[i]->ToString();
    }

    return result;
  }

  std::string Array::ToSource() const
  {
    std::string result;

    result += "[";
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += ", ";
      }
      result += m_elements[i]->ToSource();
    }
    result += "]";

    return result;
  }

  /**
   * ary? ( any -- any bool )
   *
   * Returns true if given value is array.
   */
  static void w_is_ary(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_ARRAY);
    }
  }

  void api_init_array(Runtime* runtime)
  {
    runtime->AddWord("ary?", w_is_ary);
  }
}

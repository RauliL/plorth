#ifndef PLORTH_ARRAY_HPP_GUARD
#define PLORTH_ARRAY_HPP_GUARD

#include <vector>

#include "value.hpp"

namespace plorth
{
  class Array : public Value
  {
  public:
    explicit Array(const std::vector<Ref<Value>>& elements);

    inline Type GetType() const
    {
      return TYPE_ARRAY;
    }

    /**
     * Returns true if the array is empty.
     */
    inline bool IsEmpty() const
    {
      return m_elements.empty();
    }

    /**
     * Returns the number of elements in the array.
     */
    inline std::vector<Ref<Value>>::size_type GetSize() const
    {
      return m_elements.size();
    }

    /**
     * Returns element from specified index.
     */
    inline const Ref<Value>& Get(std::size_t index) const
    {
      return m_elements[index];
    }

    inline const std::vector<Ref<Value>>& GetElements() const
    {
      return m_elements;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const std::vector<Ref<Value>> m_elements;
  };
}

#endif /* !PLORTH_ARRAY_HPP_GUARD */

#ifndef PLORTH_VALUE_ARRAY_HPP_GUARD
#define PLORTH_VALUE_ARRAY_HPP_GUARD

#include <plorth/value.hpp>

#include <vector>

namespace plorth
{
  /**
   * Array is an indexed sequence of other values. It is one of the basic data
   * types of Plorth.
   */
  class array : public value
  {
  public:
    using container_type = std::vector<ref<value>>;

    /**
     * Constructs new array from given elements.
     */
    explicit array(const container_type& elements);

    /**
     * Returns reference to the underlying container that contains the elements
     * of the array.
     */
    inline const container_type& elements() const
    {
      return m_elements;
    }

    inline enum type type() const
    {
      return type_array;
    }

    bool equals(const ref<value>& that) const;
    unistring to_string () const;
    unistring to_source() const;

  private:
    /** Container for elements of the array. */
    const container_type m_elements;
  };
}

#endif /* !PLORTH_VALUE_ARRAY_HPP_GUARD */

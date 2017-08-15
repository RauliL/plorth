#ifndef PLORTH_VALUE_OBJECT_HPP_GUARD
#define PLORTH_VALUE_OBJECT_HPP_GUARD

#include <plorth/value.hpp>

#include <unordered_map>

namespace plorth
{
  class object : public value
  {
  public:
    using container_type = std::unordered_map<unistring, ref<value>>;

    explicit object(const container_type& properties);

    inline const container_type& properties() const
    {
      return m_properties;
    }

    /**
     * Retrieves property with given name from the object itself and it's
     * prototypes.
     *
     * \param name Name of the property to retrieve.
     * \return     Reference to value of the property or null reference if
     *             property with given name does not exist.
     */
    ref<value> property(const unistring& name) const;

    inline enum type type() const
    {
      return type_object;
    }

    bool equals(const ref<value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

  private:
    const container_type m_properties;
  };
}

#endif /* !PLORTH_VALUE_OBJECT_HPP_GUARD */

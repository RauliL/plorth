#ifndef PLORTH_OBJECT_HPP_GUARD
#define PLORTH_OBJECT_HPP_GUARD

#include <unordered_map>

#include "value.hpp"

namespace plorth
{
  class Object : public Value
  {
  public:
    using Dictionary = std::unordered_map<std::string, Ref<Value>>;

    explicit Object();

    explicit Object(const Dictionary& properties);

    inline Type GetType() const
    {
      return TYPE_OBJECT;
    }

    inline const Dictionary& GetOwnProperties() const
    {
      return m_properties;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    Ref<Value> GetOwnProperty(const std::string& name) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const Dictionary m_properties;
  };
}

#endif /* !PLORTH_OBJECT_HPP_GUARD */

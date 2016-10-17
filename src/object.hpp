#ifndef PLORTH_OBJECT_HPP_GUARD
#define PLORTH_OBJECT_HPP_GUARD

#include <unordered_map>

#include "value.hpp"

namespace plorth
{
  class Object : public Value
  {
  public:
    explicit Object(const std::unordered_map<std::string, Ref<Value>>& entries);

    inline Type GetType() const
    {
      return TYPE_OBJECT;
    }

    inline std::size_t GetSize() const
    {
      return m_entries.size();
    }

    inline const std::unordered_map<std::string, Ref<Value>>& GetEntries() const
    {
      return m_entries;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    Ref<Value> Find(const std::string& name) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const std::unordered_map<std::string, Ref<Value>> m_entries;
  };
}

#endif /* !PLORTH_OBJECT_HPP_GUARD */

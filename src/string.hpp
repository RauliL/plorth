#ifndef PLORTH_STRING_HPP_GUARD
#define PLORTH_STRING_HPP_GUARD

#include "value.hpp"

namespace plorth
{
  class String : public Value
  {
  public:
    explicit String(const std::string& value);

    inline Type GetType() const
    {
      return TYPE_STRING;
    }

    /**
     * Returns true if the string is empty.
     */
    inline bool IsEmpty() const
    {
      return m_value.empty();
    }

    inline const std::string& GetValue() const
    {
      return m_value;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const std::string m_value;
  };
}

#endif /* !PLORTH_STRING_HPP_GUARD */

#ifndef PLORTH_BOOL_HPP_GUARD
#define PLORTH_BOOL_HPP_GUARD

#include "value.hpp"

namespace plorth
{
  class Bool : public Value
  {
  public:
    explicit Bool(bool value);

    inline Type GetType() const
    {
      return TYPE_BOOL;
    }

    inline bool GetValue() const
    {
      return m_value;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

  private:
    const bool m_value;
  };
}

#endif /* !PLORTH_BOOL_HPP_GUARD */

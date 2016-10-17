#ifndef PLORTH_NULL_HPP_GUARD
#define PLORTH_NULL_HPP_GUARD

#include "value.hpp"

namespace plorth
{
  class Null : public Value
  {
  public:
    explicit Null();

    inline Type GetType() const
    {
      return TYPE_NULL;
    }

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;
  };
}

#endif /* !PLORTH_NULL_HPP_GUARD */

#ifndef PLORTH_VALUE_HPP_GUARD
#define PLORTH_VALUE_HPP_GUARD

#include <iostream>

#include "memory.hpp"

namespace plorth
{
  /**
   * Abstract base class for anything which represents an value in Plorth.
   */
  class Value : public ManagedObject
  {
  public:
    enum Type
    {
      TYPE_NULL,
      TYPE_BOOL,
      TYPE_NUMBER,
      TYPE_STRING,
      TYPE_ARRAY,
      TYPE_OBJECT,
      TYPE_QUOTE,
      TYPE_ERROR
    };

    explicit Value();

    /**
     * Returns type of the value.
     */
    virtual Type GetType() const = 0;

    virtual Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    virtual bool Equals(const Ref<Value>& that) const = 0;

    virtual std::string ToString() const = 0;

    virtual std::string ToSource() const;
  };

  std::ostream& operator<<(std::ostream&, const Ref<Value>&);
  std::ostream& operator<<(std::ostream&, Value::Type);
}

#endif /* !PLORTH_VALUE_HPP_GUARD */

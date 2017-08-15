#ifndef PLORTH_VALUE_HPP_GUARD
#define PLORTH_VALUE_HPP_GUARD

#include <plorth/memory.hpp>
#include <plorth/unicode.hpp>

namespace plorth
{
  /**
   * Abstract base class for everything that acts as an value in Plorth.
   */
  class value : public memory::managed
  {
  public:
    /**
     * Enumeration of different supported value types.
     */
    enum type
    {
      /** Value for null. */
      type_null,
      /** Boolean values. */
      type_boolean,
      /** Number (floating point) values. */
      type_number,
      /** String (Unicode) values. */
      type_string,
      /** Array values. */
      type_array,
      /** Other type of objects. */
      type_object,
      /** Quotes. */
      type_quote,
      /** Errors. */
      type_error
    };

    /**
     * Returns type of the value.
     */
    virtual enum type type() const = 0;

    /**
     * Tests whether the value is of given type.
     */
    inline bool is(enum type t) const
    {
      return type() == t;
    };

    /**
     * Determines prototype object of the value, based on it's type. If the
     * value is an object, property called "__proto__" will be used instead,
     * with the runtime's object prototype acting as a fallback.
     *
     * \param runtime Script runtime to use for prototype retrieval.
     * \return        Prototype object of the value.
     */
    ref<object> prototype(const ref<class runtime>& runtime) const;

    /**
     * Tests whether two values are equal.
     *
     * \param that Other value to test this one against.
     */
    virtual bool equals(const ref<value>& that) const = 0;

    /**
     * Constructs string representation of the value.
     */
    virtual unistring to_string() const = 0;

    /**
     * Constructs a string that resembles as accurately as possible what this
     * value would look like in source code.
     */
    virtual unistring to_source() const = 0;
  };

  std::ostream& operator<<(std::ostream&, enum value::type);
  std::ostream& operator<<(std::ostream&, const ref<value>&);
}

#endif /* !PLORTH_VALUE_HPP_GUARD */

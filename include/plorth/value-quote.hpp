#ifndef PLORTH_VALUE_QUOTE_HPP_GUARD
#define PLORTH_VALUE_QUOTE_HPP_GUARD

#include <plorth/value.hpp>

#include <functional>

namespace plorth
{
  /**
   * Quote is a container for executable code, which can be either tokens
   * compiled from a script or callback to native C++ function.
   */
  class quote : public value
  {
  public:
    /** Signature of C++ function that can be used as quote. */
    using callback = std::function<void(const ref<context>&)>;

    /**
     * Enumeration for different supported quote types.
     */
    enum quote_type
    {
      quote_type_native,
      quote_type_compiled,
      quote_type_curried,
      quote_type_composed,
      quote_type_negated,
      quote_type_constant
    };

    /**
     * Invokes the quote.
     *
     * \param ctx Scripting context to execute the quote in.
     * \return    Boolean flag which tells whether execution of the quote was
     *            performed successfully without errors.
     */
    virtual bool call(const ref<context>& ctx) const = 0;

    /**
     * Returns type of the quote.
     */
    virtual enum quote_type quote_type() const = 0;

    /**
     * Tests whether the quote is of given type.
     */
    inline bool is(enum quote_type t) const
    {
      return quote_type() == t;
    }

    inline enum type type() const
    {
      return type_quote;
    }

    unistring to_source() const;
  };
}

#endif /* !PLORTH_VALUE_QUOTE_HPP_GUARD */

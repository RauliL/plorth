#ifndef PLORTH_QUOTE_HPP_GUARD
#define PLORTH_QUOTE_HPP_GUARD

#include <vector>

#include "value.hpp"

namespace plorth
{
  /**
   * Quote is a container for executable code, which can be either tokens
   * compiled from a script or callback to native C++ function.
   */
  class Quote : public Value
  {
  public:
    /**
     * Signature of C++ function that can be used as native quote.
     */
    using CallbackSignature = void(*)(const Ref<Context>&);

    static Ref<Quote> Compile(
      const Ref<Context>& context,
      const std::string& source
    );

    inline Type GetType() const
    {
      return TYPE_QUOTE;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    virtual bool Call(const Ref<Context>& context) const = 0;
  };
}

#endif /* !PLORTH_QUOTE_HPP_GUARD */

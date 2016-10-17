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
    static Ref<Quote> Compile(
      const Ref<State>& state,
      const std::string& source
    );

    inline Type GetType() const
    {
      return TYPE_QUOTE;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    virtual bool Call(const Ref<State>& state) const = 0;
  };

  class CompiledQuote : public Quote
  {
  public:
    explicit CompiledQuote(const std::vector<Token>& tokens);

    bool Call(const Ref<State>& state) const;

    std::string ToString() const;

  private:
    const std::vector<Token> m_tokens;
  };

  class NativeQuote : public Quote
  {
  public:
    typedef void (*CallbackSignature)(const Ref<State>&);

    explicit NativeQuote(CallbackSignature callback);

    bool Call(const Ref<State>& state) const;

    std::string ToString() const;

  private:
    const CallbackSignature m_callback;
  };
}

#endif /* !PLORTH_QUOTE_HPP_GUARD */

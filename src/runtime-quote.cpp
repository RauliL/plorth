#include "context.hpp"

namespace plorth
{
  /**
   * call ( quote -- )
   *
   * Executes quote taken from top of the stack.
   */
  static void w_call(const Ref<Context>& context)
  {
    Ref<Quote> quote;

    if (context->PopQuote(quote))
    {
      quote->Call(context);
    }
  }

  namespace
  {
    class NegatedQuote : public Quote
    {
    public:
      explicit NegatedQuote(const Ref<Quote>& delegate)
        : m_delegate(delegate) {}

      bool Call(const Ref<Context>& context) const
      {
        bool value;

        if (!m_delegate->Call(context) || !context->PopBool(value))
        {
          return false;
        }
        context->PushBool(!value);

        return true;
      }

      std::string ToString() const
      {
        std::string result;

        result += "{";
        result += m_delegate->ToString();
        result += " call not}";

        return result;
      }

    private:
      const Ref<Quote> m_delegate;
    };
  }

  /**
   * negate ( quote -- quote )
   *
   * Constructs new quote which negates boolean result returned by the original
   * quote.
   */
  static void w_negate(const Ref<Context>& context)
  {
    Ref<Quote> delegate;

    if (context->PopQuote(delegate))
    {
      context->Push(new (context->GetRuntime()) NegatedQuote(delegate));
    }
  }

  namespace
  {
    class JoinedQuote : public Quote
    {
    public:
      explicit JoinedQuote(const Ref<Quote>& first, const Ref<Quote>& second)
        : m_first(first)
        , m_second(second) {}

      bool Call(const Ref<Context>& context) const
      {
        return m_first->Call(context) && m_second->Call(context);
      }

      std::string ToString() const
      {
        std::string result;

        result += "{";
        result += m_first->ToString();
        result += " call ";
        result += m_second->ToString();
        result += " call}";

        return result;
      }

    private:
      const Ref<Quote> m_first;
      const Ref<Quote> m_second;
    };
  }

  /**
   * + ( quote quote -- quote )
   *
   * Constructs quote which calls two quotes in sequence.
   */
  static void w_plus(const Ref<Context>& context)
  {
    Ref<Quote> first;
    Ref<Quote> second;

    if (context->PopQuote(second) && context->PopQuote(first))
    {
      context->Push(new (context->GetRuntime()) JoinedQuote(first, second));
    }
  }

  Ref<Object> make_quote_prototype(Runtime* runtime)
  {
    return runtime->NewPrototype({
      { "call", w_call },
      { "negate", w_negate },
      { "+", w_plus },
    });
  }
}

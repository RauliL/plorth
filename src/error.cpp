#include <sstream>

#include "context.hpp"

namespace plorth
{
  Error::Error(ErrorCode code, const std::string& message)
    : m_code(code)
    , m_message(message) {}

  Ref<Object> Error::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetErrorPrototype();
  }

  bool Error::Equals(const Ref<Value>& that) const
  {
    Ref<Error> other;

    if (that->GetType() != TYPE_ERROR)
    {
      return false;
    }
    other = that.As<Error>();

    return m_code == other->m_code && m_message == other->m_message;
  }

  std::string Error::ToString() const
  {
    std::stringstream ss;

    ss << m_code;
    if (!m_message.empty())
    {
      ss << ": " << m_message;
    }

    return ss.str();
  }

  std::string Error::ToSource() const
  {
    return "<" + ToString() + ">";
  }

  std::ostream& operator<<(std::ostream& os, Error::ErrorCode code)
  {
    const char* description = "unknown error";

    switch (code)
    {
      case Error::ERROR_CODE_SYNTAX:
        description = "syntax error";
        break;

      case Error::ERROR_CODE_REFERENCE:
        description = "reference error";
        break;

      case Error::ERROR_CODE_TYPE:
        description = "type error";
        break;

      case Error::ERROR_CODE_RANGE:
        description = "range error";
        break;

      case Error::ERROR_CODE_UNKNOWN:
        description = "unknown error";
        break;
    }
    os << description;

    return os;
  }

  /**
   * error? ( any -- any bool )
   *
   * Returns true if given value is error.
   */
  static void w_is_error(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_ERROR);
    }
  }

  /**
   * try ( quote quote -- )
   *
   * Executes first quote and if it throws an error, calls second quote with
   * the error on top of the stack.
   */
  static void w_try(const Ref<Context>& context)
  {
    Ref<Quote> try_block;
    Ref<Quote> catch_block;

    if (!context->PopQuote(catch_block) || !context->PopQuote(try_block))
    {
      return;
    }
    if (!try_block->Call(context))
    {
      context->Push(context->GetError());
      context->ClearError();
      catch_block->Call(context);
    }
  }

  void api_init_error(Runtime* runtime)
  {
    runtime->AddWord("error?", w_is_error);
    runtime->AddWord("try", w_try);
  }
}

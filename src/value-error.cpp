#include <plorth/context.hpp>

#include <sstream>

namespace plorth
{
  error::error(enum code code, const unistring& message)
    : m_code(code)
    , m_message(message) {}

  bool error::equals(const ref<value>& that) const
  {
    const error* err;

    if (!that->is(type_error))
    {
      return false;
    }

    err = that.cast<error>();

    return m_code == err->m_code && !m_message.compare(err->m_message);
  }

  unistring error::to_string() const
  {
    std::stringstream ss;

    ss << m_code;
    if (!m_message.empty())
    {
      ss << ": " << m_message;
    }

    return utf8_decode(ss.str());
  }

  unistring error::to_source() const
  {
    return "<" + to_string() + ">";
  }

  std::ostream& operator<<(std::ostream& os, enum error::code code)
  {
    switch (code)
    {
      case error::code_syntax:
        os << "Syntax error";
        break;

      case error::code_reference:
        os << "Reference error";
        break;

      case error::code_type:
        os << "Type error";
        break;

      case error::code_range:
        os << "Range error";
        break;

      case error::code_import:
        os << "Import error";
        break;

      case error::code_unknown:
        os << "Unknown error";
        break;
    }

    return os;
  }

  /**
   * code ( error -- error number )
   *
   * Returns error code extracted from the error in numeric form.
   */
  static void w_code(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type_error))
    {
      ctx->push(err);
      ctx->push_number(err.cast<error>()->code());
    }
  }

  /**
   * message ( error -- error string|null )
   *
   * Returns error message extracted from the error, or null if the error does
   * not have any error message.
   */
  static void w_message(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type_error))
    {
      const unistring& message = err.cast<error>()->message();

      ctx->push(err);
      if (message.empty())
      {
        ctx->push_null();
      } else {
        ctx->push_string(message);
      }
    }
  }

  /**
   * throw ( error -- )
   *
   * Sets given error as current error of the execution context.
   */
  static void w_throw(const ref<context>& ctx)
  {
    ref<value> err;

    if (ctx->pop(err, value::type_error))
    {
      ctx->error(err.cast<error>());
    }
  }

  namespace api
  {
    runtime::prototype_definition error_prototype()
    {
      return
      {
        { "code", w_code },
        { "message", w_message },
        { "throw", w_throw },
      };
    }
  }
}

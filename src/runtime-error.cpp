#include <plorth/plorth-context.hpp>

namespace plorth
{
  /**
   * code ( error -- error num )
   *
   * Returns error code extracted from the error in numeric form.
   */
  static void w_code(const Ref<Context>& context)
  {
    Ref<Error> error;

    if (context->PeekError(error))
    {
      context->PushNumber(static_cast<std::int64_t>(error->GetCode()));
    }
  }

  /**
   * message ( error -- error str|null )
   *
   * Returns error message extracted from the error, or null if the error does
   * not have any error message.
   */
  static void w_message(const Ref<Context>& context)
  {
    Ref<Error> error;

    if (context->PeekError(error))
    {
      const std::string& message = error->GetMessage();

      if (message.empty())
      {
        context->PushNull();
      } else {
        context->PushString(message);
      }
    }
  }

  /**
   * throw ( error -- )
   *
   * Sets given error as current error of the execution context.
   */
  static void w_throw(const Ref<Context>& context)
  {
    Ref<Error> error;

    if (context->PopError(error))
    {
      context->SetError(error);
    }
  }

  Ref<Object> make_error_prototype(Runtime* runtime)
  {
    return runtime->NewPrototype({
      { "code", w_code },
      { "message", w_message },
      { "throw", w_throw },
    });
  }
}

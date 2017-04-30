#ifndef PLORTH_CONTEXT_HPP_GUARD
#define PLORTH_CONTEXT_HPP_GUARD

#include <plorth/plorth-runtime.hpp>

namespace plorth
{
  /**
   * Represents program execution state.
   */
  class Context : public ManagedObject
  {
  public:
    using Dictionary = Runtime::Dictionary;
    using Stack = std::vector<Ref<Value>>;

    /**
     * Constructs new context.
     *
     * \param runtime Runtime associated with this context.
     */
    explicit Context(const Ref<Runtime>& runtime);

    /**
     * Returns the runtime associated with this context.
     */
    inline const Ref<Runtime>& GetRuntime() const
    {
      return m_runtime;
    }

    /**
     * Returns true if the execution context has uncaught error.
     */
    inline bool HasError() const
    {
      return !!m_error;
    }

    /**
     * Returns reference to currently uncaught error in this context, or null
     * reference if the context does not have one.
     */
    inline const Ref<Error>& GetError() const
    {
      return m_error;
    }

    /**
     * Replaces currently uncaught error with given error instance.
     */
    inline void SetError(const Ref<Error>& error)
    {
      m_error = error;
    }

    /**
     * Constructs new error instance with given error code and error message
     * and replaces this execution state's currently uncaught error with it.
     */
    void SetError(Error::ErrorCode code, const std::string& message);

    /**
     * Removes currently uncaught error, if there is one.
     */
    void ClearError();

    /**
     * Returns the dictionary used by this context to store words.
     */
    inline const Dictionary& GetDictionary() const
    {
      return m_dictionary;
    }

    /**
     * Returns the data stack used by this context to store values.
     */
    inline const Stack& GetStack() const
    {
      return m_stack;
    }

    /**
     * Removes all values from the data stack.
     */
    void ClearStack()
    {
      m_stack.clear();
    }

    bool CallWord(const std::string& name);

    void AddWord(const std::string& name, const Ref<Value>& value);

    bool Peek(Ref<Value>& slot);

    bool Peek(Ref<Value>& slot, Value::Type type);

    bool PeekArray(Ref<Array>& slot);

    bool PeekBool(bool& slot);

    bool PeekError(Ref<Error>& slot);

    bool PeekNumber(Ref<Number>& slot);

    bool PeekObject(Ref<Object>& slot);

    bool PeekQuote(Ref<Quote>& slot);

    bool PeekString(Ref<String>& slot);

    /**
     * Pushes given value into the data stack.
     *
     * \param value Value to insert as top of the stack value.
     */
    void Push(const Ref<Value>& value);

    void PushArray(const std::vector<Ref<Value>>& elements);

    /**
     * Pushes given boolean value into the data stack.
     *
     * \param value Value to insert as top of the stack value.
     */
    void PushBool(bool value);

    /**
     * Pushes null value as top of the data stack.
     */
    void PushNull();

    void PushNumber(std::int64_t value);

    void PushNumber(double value);

    void PushNumber(const mpz_class& value);

    void PushNumber(const std::string& value);

    void PushObject();

    void PushObject(const Object::Dictionary& properties);

    void PushString(const std::string& value);

    bool Pop();

    bool Pop(Ref<Value>& slot);

    bool Pop(Ref<Value>& slot, Value::Type type);

    bool PopArray(Ref<Array>& slot);

    bool PopBool(bool& slot);

    bool PopError(Ref<Error>& slot);

    bool PopNumber(Ref<Number>& slot);

    bool PopObject(Ref<Object>& slot);

    bool PopQuote(Ref<Quote>& slot);

    bool PopString(Ref<String>& slot);

  private:
    /** Runtime associated with this context. */
    const Ref<Runtime> m_runtime;
    /** Currently uncaught error. */
    Ref<Error> m_error;
    /** Container for words associated with this context. */
    Dictionary m_dictionary;
    /** Data stack used for storing values in this context. */
    Stack m_stack;
  };
}

#endif /* !PLORTH_CONTEXT_HPP_GUARD */

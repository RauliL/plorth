#ifndef PLORTH_STATE_HPP_GUARD
#define PLORTH_STATE_HPP_GUARD

#include "runtime.hpp"

namespace plorth
{
  /**
   * Represents program execution state.
   */
  class State : public ManagedObject
  {
  public:
    typedef Runtime::Dictionary Dictionary;
    typedef std::vector<Ref<Value>> Stack;

    /**
     * Constructs new execution state.
     *
     * \param runtime Runtime associated with this state.
     */
    explicit State(const Ref<Runtime>& runtime);

    /**
     * Returns the runtime associated with this execution state.
     */
    inline const Ref<Runtime>& GetRuntime() const
    {
      return m_runtime;
    }

    /**
     * Returns reference to currently uncaught error, or null reference if
     * there isn't any.
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

    inline const Dictionary& GetDictionary() const
    {
      return m_dictionary;
    }

    Ref<Value> FindWord(const std::string& name) const;

    void AddWord(const std::string& name, const Ref<Value>& value);

    bool CallWord(const std::string& name);

    /**
     * Returns the data stack associated with this execution state.
     */
    inline Stack& GetDataStack()
    {
      return m_data;
    }

    /**
     * Returns the data stack associated with this execution state.
     */
    inline const Stack& GetDataStack() const
    {
      return m_data;
    }

    bool Peek(Ref<Value>& slot);

    bool Peek(Ref<Value>& slot, Value::Type type);

    bool PeekString(Ref<String>& slot);

    bool PeekArray(Ref<Array>& slot);

    bool PeekObject(Ref<Object>& slot);

    bool PeekError(Ref<Error>& slot);

    /**
     * Pushes given value into the data stack.
     *
     * \param value Value to insert as top of the stack value.
     */
    void Push(const Ref<Value>& value);

    /**
     * Pushes null value as top of the data stack.
     */
    void PushNull();

    /**
     * Pushes given boolean value into the data stack.
     *
     * \param value Value to insert as top of the stack value.
     */
    void PushBool(bool value);

    void PushNumber(std::int64_t value);

    void PushNumber(double value);

    void PushNumber(const mpz_class& value);

    void PushNumber(const std::string& value);

    void PushString(const std::string& value);

    void PushArray(const std::vector<Ref<Value>>& elements);

    void PushObject(const std::unordered_map<std::string, Ref<Value>>& entries);

    bool Pop();

    bool Pop(Ref<Value>& slot);

    bool Pop(Ref<Value>& slot, Value::Type type);

    bool PopBool(Ref<Bool>& slot);

    bool PopNumber(Ref<Number>& slot);

    bool PopString(Ref<String>& slot);

    bool PopArray(Ref<Array>& slot);

    bool PopObject(Ref<Object>& slot);

    bool PopQuote(Ref<Quote>& slot);

    bool PopError(Ref<Error>& slot);

  private:
    /** Runtime associated with this execution state. */
    const Ref<Runtime> m_runtime;
    /** Currently uncaught error. */
    Ref<Error> m_error;
    /** Container for words associated with this execution state. */
    Dictionary m_dictionary;
    /** Data stack associated with this execution state. */
    Stack m_data;
  };
}

#endif /* !PLORTH_STATE_HPP_GUARD */

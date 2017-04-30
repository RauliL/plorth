#ifndef PLORTH_RUNTIME_HPP_GUARD
#define PLORTH_RUNTIME_HPP_GUARD

#include <gmpxx.h>

#include "bool.hpp"
#include "error.hpp"
#include "null.hpp"
#include "number.hpp"
#include "object.hpp"
#include "quote.hpp"

namespace plorth
{
  class Runtime : public ManagedObject
  {
  public:
    typedef std::unordered_map<std::string, Ref<Value>> Dictionary;

    ~Runtime();

    /**
     * Returns the memory manager associated with this runtime.
     */
    inline MemoryManager* GetMemoryManager() const
    {
      return m_memory_manager;
    }

    inline const Ref<Null>& GetNullValue() const
    {
      return m_null_value;
    }

    inline const Ref<Bool>& GetTrueValue() const
    {
      return m_true_value;
    }

    inline Ref<Bool> GetFalseValue() const
    {
      return m_false_value;
    }

    inline const Ref<Object>& GetArrayPrototype() const
    {
      return m_array_prototype;
    }

    inline const Ref<Object>& GetBoolPrototype() const
    {
      return m_bool_prototype;
    }

    inline const Ref<Object>& GetErrorPrototype() const
    {
      return m_error_prototype;
    }

    inline const Ref<Object>& GetNumberPrototype() const
    {
      return m_number_prototype;
    }

    inline const Ref<Object>& GetObjectPrototype() const
    {
      return m_object_prototype;
    }

    inline const Ref<Object>& GetQuotePrototype() const
    {
      return m_quote_prototype;
    }

    inline const Ref<Object>& GetStringPrototype() const
    {
      return m_string_prototype;
    }

    Ref<Bool> NewBool(bool value) const;

    Ref<Number> NewNumber(std::int64_t value) const;

    Ref<Number> NewNumber(double value) const;

    Ref<Number> NewNumber(const mpz_class& value) const;

    Ref<Number> NewNumber(const std::string& value) const;

    Ref<String> NewString(const std::string& value) const;

    Ref<Array> NewArray(const std::vector<Ref<Value>>& elements) const;

    Ref<Object> NewObject() const;

    Ref<Object> NewObject(const Object::Dictionary& properties) const;

    Ref<Quote> NewNativeQuote(Quote::CallbackSignature callback) const;

    Ref<Error> NewError(Error::ErrorCode code, const std::string& message) const;

    Ref<Object> NewPrototype(
      const std::unordered_map<std::string, Quote::CallbackSignature>& properties
    );

    /**
     * Returns the dictionary used to store global words.
     */
    inline const Dictionary& GetDictionary() const
    {
      return m_dictionary;
    }

    bool FindWord(const std::string& name, Ref<Value>& slot) const;

    void AddWord(
      const std::string& name,
      Quote::CallbackSignature callback
    );

    void AddWord(const std::string& name, const Ref<Quote>& quote);

    void AddNamespace(
      const std::string& name,
      const std::unordered_map<std::string, Quote::CallbackSignature>& entries
    );

  private:
    Runtime(MemoryManager* memory_manager);
    friend class MemoryManager;

  private:
    /** Pointer to the memory manager associated with this runtime. */
    MemoryManager* m_memory_manager;
    /** Container for globally defined words. */
    Dictionary m_dictionary;
    const Ref<Null> m_null_value;
    const Ref<Bool> m_true_value;
    const Ref<Bool> m_false_value;
    Ref<Object> m_array_prototype;
    Ref<Object> m_bool_prototype;
    Ref<Object> m_error_prototype;
    Ref<Object> m_number_prototype;
    Ref<Object> m_object_prototype;
    Ref<Object> m_quote_prototype;
    Ref<Object> m_string_prototype;
  };

}

#endif /* !PLORTH_RUNTIME_HPP_GUARD */

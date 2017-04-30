#include "array.hpp"
#include "context.hpp"
#include "string.hpp"
#include "token.hpp"

namespace plorth
{
  void init_global_dictionary(Runtime*);
  Ref<Object> make_array_prototype(Runtime*);
  Ref<Object> make_bool_prototype(Runtime*);
  Ref<Object> make_error_prototype(Runtime*);
  Ref<Object> make_number_prototype(Runtime*);
  Ref<Object> make_object_prototype(Runtime*);
  Ref<Object> make_quote_prototype(Runtime*);
  Ref<Object> make_string_prototype(Runtime*);

  Runtime::Runtime(MemoryManager* memory_manager)
    : m_memory_manager(memory_manager)
    , m_null_value(new (memory_manager) Null())
    , m_true_value(new (memory_manager) Bool(true))
    , m_false_value(new (memory_manager) Bool(false))
  {
    init_global_dictionary(this);

    m_array_prototype = make_array_prototype(this);
    m_bool_prototype = make_bool_prototype(this);
    m_error_prototype = make_error_prototype(this);
    m_number_prototype = make_number_prototype(this);
    m_object_prototype = make_object_prototype(this);
    m_quote_prototype = make_quote_prototype(this);
    m_string_prototype = make_string_prototype(this);

    m_dictionary["ary"] = m_array_prototype;
    m_dictionary["bool"] = m_bool_prototype;
    m_dictionary["error"] = m_error_prototype;
    m_dictionary["num"] = m_number_prototype;
    m_dictionary["obj"] = m_object_prototype;
    m_dictionary["quote"] = m_quote_prototype;
    m_dictionary["str"] = m_string_prototype;
  }

  Runtime::~Runtime() {}

  Ref<Bool> Runtime::NewBool(bool value) const
  {
    return value ? m_true_value : m_false_value;
  }

  Ref<Number> Runtime::NewNumber(std::int64_t value) const
  {
    return new (m_memory_manager) IntNumber(value);
  }

  Ref<Number> Runtime::NewNumber(double value) const
  {
    return new (m_memory_manager) FloatNumber(value);
  }

  Ref<Number> Runtime::NewNumber(const mpz_class& value) const
  {
    return new (m_memory_manager) BigIntNumber(value);
  }

  Ref<Number> Runtime::NewNumber(const std::string& value) const
  {
    const std::string::size_type dot_index = value.find('.');

    if (dot_index == std::string::npos)
    {
      const std::int64_t i = std::strtoll(value.c_str(), nullptr, 10);

      if (errno == ERANGE)
      {
        return NewNumber(mpz_class(value));
      } else {
        return NewNumber(i);
      }
    } else {
      return NewNumber(std::strtod(value.c_str(), nullptr));
    }
  }

  Ref<String> Runtime::NewString(const std::string& value) const
  {
    return new (m_memory_manager) String(value);
  }

  Ref<Array> Runtime::NewArray(const std::vector<Ref<Value>>& elements) const
  {
    return new (m_memory_manager) Array(elements);
  }

  Ref<Object> Runtime::NewObject() const
  {
    return new (m_memory_manager) Object();
  }

  Ref<Object> Runtime::NewObject(const Object::Dictionary& properties) const
  {
    return new (m_memory_manager) Object(properties);
  }

  namespace
  {
    class NativeQuote : public Quote
    {
    public:
      explicit NativeQuote(CallbackSignature callback)
        : m_callback(callback) {}

      bool Call(const Ref<Context>& context) const
      {
        m_callback(context);

        return !context->HasError();
      }

      std::string ToString() const
      {
        return "<native quote>";
      }

    private:
      const CallbackSignature m_callback;
    };
  }

  Ref<Quote> Runtime::NewNativeQuote(Quote::CallbackSignature callback) const
  {
    return new (m_memory_manager) NativeQuote(callback);
  }

  Ref<Error> Runtime::NewError(Error::ErrorCode code,
                               const std::string& message) const
  {
    return new (m_memory_manager) Error(code, message);
  }

  Ref<Object> Runtime::NewPrototype(const std::unordered_map<std::string, Quote::CallbackSignature>& properties)
  {
    Object::Dictionary result;

    for (const auto& property : properties)
    {
      result[property.first] = NewNativeQuote(property.second);
    }

    return NewObject(result);
  }

  bool Runtime::FindWord(const std::string& name, Ref<Value>& slot) const
  {
    const auto entry = m_dictionary.find(name);

    if (entry != end(m_dictionary))
    {
      slot = entry->second;

      return true;
    }

    return false;
  }

  void Runtime::AddWord(const std::string& name,
                        NativeQuote::CallbackSignature callback)
  {
    m_dictionary[name] = new (m_memory_manager) NativeQuote(callback);
  }

  void Runtime::AddWord(const std::string& name, const Ref<Quote>& quote)
  {
    m_dictionary[name] = quote;
  }

  void Runtime::AddNamespace(const std::string& name,
                             const std::unordered_map<std::string, NativeQuote::CallbackSignature>& entries)
  {
    std::unordered_map<std::string, Ref<Value>> translated;

    for (auto& entry : entries)
    {
      translated[entry.first] = new (m_memory_manager) NativeQuote(entry.second);
    }
    m_dictionary[name] = new (m_memory_manager) Object(translated);
  }
}

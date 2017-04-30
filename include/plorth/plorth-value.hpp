#ifndef PLORTH_VALUE_HPP_GUARD
#define PLORTH_VALUE_HPP_GUARD

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <gmpxx.h>

#include <plorth/plorth-memory.hpp>

namespace plorth
{
  /**
   * Abstract base class for anything which represents an value in Plorth.
   */
  class Value : public ManagedObject
  {
  public:
    enum Type
    {
      TYPE_NULL,
      TYPE_BOOL,
      TYPE_NUMBER,
      TYPE_STRING,
      TYPE_ARRAY,
      TYPE_OBJECT,
      TYPE_QUOTE,
      TYPE_ERROR
    };

    explicit Value();

    /**
     * Returns type of the value.
     */
    virtual Type GetType() const = 0;

    virtual Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    virtual bool Equals(const Ref<Value>& that) const = 0;

    virtual std::string ToString() const = 0;

    virtual std::string ToSource() const;
  };

  std::ostream& operator<<(std::ostream&, const Ref<Value>&);
  std::ostream& operator<<(std::ostream&, Value::Type);

  class Array : public Value
  {
  public:
    explicit Array(const std::vector<Ref<Value>>& elements);

    inline Type GetType() const
    {
      return TYPE_ARRAY;
    }

    inline const std::vector<Ref<Value>>& GetElements() const
    {
      return m_elements;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const std::vector<Ref<Value>> m_elements;
  };

  class Bool : public Value
  {
  public:
    explicit Bool(bool value);

    inline Type GetType() const
    {
      return TYPE_BOOL;
    }

    inline bool GetValue() const
    {
      return m_value;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

  private:
    const bool m_value;
  };

  class Error : public Value
  {
  public:
    enum ErrorCode
    {
      /** Syntax error. */
      ERROR_CODE_SYNTAX = 1,
      /** Reference error. */
      ERROR_CODE_REFERENCE = 2,
      /** Type error. */
      ERROR_CODE_TYPE = 3,
      /** Range error. */
      ERROR_CODE_RANGE = 4,
      /** Import error. */
      ERROR_CODE_IMPORT = 5,
      /** Unknown error. */
      ERROR_CODE_UNKNOWN = 100
    };

    explicit Error(ErrorCode code, const std::string& message);

    inline Type GetType() const
    {
      return TYPE_ERROR;
    }

    inline ErrorCode GetCode() const
    {
      return m_code;
    }

    inline const std::string& GetMessage() const
    {
      return m_message;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const ErrorCode m_code;
    const std::string m_message;
  };

  std::ostream& operator<<(std::ostream&, Error::ErrorCode);

  class Number : public Value
  {
  public:
    enum NumberType
    {
      NUMBER_TYPE_INT,
      NUMBER_TYPE_FLOAT,
      NUMBER_TYPE_BIG_INT,
    };

    inline Type GetType() const
    {
      return TYPE_NUMBER;
    }

    virtual NumberType GetNumberType() const = 0;

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    virtual std::int64_t AsInt() const = 0;

    virtual double AsFloat() const = 0;

    virtual mpz_class AsBigInt() const = 0;
  };

  class Null : public Value
  {
  public:
    explicit Null();

    inline Type GetType() const
    {
      return TYPE_NULL;
    }

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;
  };

  class Object : public Value
  {
  public:
    using Dictionary = std::unordered_map<std::string, Ref<Value>>;

    explicit Object();

    explicit Object(const Dictionary& properties);

    inline Type GetType() const
    {
      return TYPE_OBJECT;
    }

    inline const Dictionary& GetOwnProperties() const
    {
      return m_properties;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    Ref<Value> GetOwnProperty(const std::string& name) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const Dictionary m_properties;
  };

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

  class String : public Value
  {
  public:
    explicit String(const std::string& value);

    inline Type GetType() const
    {
      return TYPE_STRING;
    }

    /**
     * Returns true if the string is empty.
     */
    inline bool IsEmpty() const
    {
      return m_value.empty();
    }

    inline const std::string& GetValue() const
    {
      return m_value;
    }

    Ref<Object> GetPrototype(const Ref<Runtime>& runtime) const;

    bool Equals(const Ref<Value>& that) const;

    std::string ToString() const;

    std::string ToSource() const;

  private:
    const std::string m_value;
  };
}

#endif /* !PLORTH_VALUE_HPP_GUARD */

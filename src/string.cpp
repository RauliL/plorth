#include "context.hpp"
#include "string.hpp"
#include "unicode.hpp"
#include "utils.hpp"

namespace plorth
{
  String::String(const std::string& value)
    : m_value(value) {}

  Ref<Object> String::GetPrototype(const Ref<Runtime>& runtime) const
  {
    Ref<Value> value;

    if (runtime->FindWord("str", value) && value->GetType() == TYPE_OBJECT)
    {
      return value.As<Object>();
    } else {
      return Ref<Object>();
    }
  }

  bool String::Equals(const Ref<Value>& that) const
  {
    if (that->GetType() != TYPE_STRING)
    {
      return false;
    }

    return that.As<String>()->m_value == m_value;
  }

  std::string String::ToString() const
  {
    return m_value;
  }

  std::string String::ToSource() const
  {
    return to_json_string(m_value);
  }

  /**
   * str? ( any -- any bool )
   *
   * Returns true if given value is string.
   */
  static void w_is_str(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Peek(value))
    {
      context->PushBool(value->GetType() == Value::TYPE_STRING);
    }
  }

  /**
   * >str ( any -- str )
   *
   * Converts value from top of the stack into string.
   */
  static void w_to_str(const Ref<Context>& context)
  {
    Ref<Value> value;

    if (context->Pop(value))
    {
      context->PushString(value->ToString());
    }
  }

  /**
   * len ( str -- str num )
   *
   * Returns length of the string.
   */
  static void w_len(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PeekString(string))
    {
      context->PushNumber(static_cast<std::int64_t>(utf8_strlen(string->GetValue().c_str())));
    }
  }

  /**
   * size ( str -- str num )
   *
   * Returns size of the string in bytes. Not necessarily the same as the
   * number of characters given by "len" words, because Plorth uses UTF-8
   * encoding.
   */
  static void w_size(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PeekString(string))
    {
      context->PushNumber(static_cast<std::int64_t>(string->GetValue().length()));
    }
  }

  /**
   * empty? ( str -- str bool )
   *
   * Returns true if the string is empty.
   */
  static void w_is_empty(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PeekString(string))
    {
      context->PushBool(string->IsEmpty());
    }
  }

  /**
   * blank? ( str -- str bool )
   *
   * Returns true if the string is empty or contains only whitespace
   * characters.
   */
  static void w_is_blank(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PeekString(string))
    {
      const std::string& s = string->GetValue();
      const std::size_t length = s.length();

      if (!length)
      {
        context->PushBool(true);
        return;
      }
      for (std::size_t i = 0; i < length; ++i)
      {
        if (!std::isspace(s[i]))
        {
          context->PushBool(false);
          return;
        }
      }
      context->PushBool(true);
    }
  }

  /**
   * chars ( str -- str ary )
   *
   * Extracts characters from the string and returns them in an array of
   * substrings.
   */
  static void w_chars(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PeekString(string))
    {
      const char* input = string->GetValue().c_str();
      char buffer[5];
      std::vector<Ref<Value>> result;

      while (*input)
      {
        unsigned int cp;

        if (!unicode_decode(input, cp))
        {
          break;
        }
        input += unicode_size(cp);
        if (unicode_encode(cp, buffer))
        {
          result.push_back(context->GetRuntime()->NewString(buffer));
        }
      }
      context->PushArray(result);
    }
  }

  /**
   * runes ( str -- str ary )
   *
   * Extracts Unicode code points from the string and returns them in an array
   * of numbers.
   */
  static void w_runes(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PeekString(string))
    {
      const char* input = string->GetValue().c_str();
      std::vector<Ref<Value>> result;

      while (*input)
      {
        unsigned int cp;

        if (!unicode_decode(input, cp))
        {
          break;
        }
        input += unicode_size(cp);
        result.push_back(context->GetRuntime()->NewNumber(static_cast<std::int64_t>(cp)));
      }
      context->PushArray(result);
    }
  }

  /**
   * lines ( str -- str ary )
   *
   * Extracts lines from the string and returns them in array.
   */
  static void w_lines(const Ref<Context>& context)
  {
    Ref<String> value;

    if (context->PeekString(value))
    {
      const std::string& str = value->GetValue();
      const std::size_t length = str.length();
      std::size_t begin = 0;
      std::size_t end = 0;
      std::vector<Ref<Value>> result;

      for (std::size_t i = 0; i < length; ++i)
      {
        if (i + 1 < length && str[i] == '\r' && str[i + 1] == '\n')
        {
          result.push_back(context->GetRuntime()->NewString(str.substr(begin, end - begin)));
          begin = end = ++i + 1;
        }
        else if (str[i] == '\n' || str[i] == '\r')
        {
          result.push_back(context->GetRuntime()->NewString(str.substr(begin, end - begin)));
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(context->GetRuntime()->NewString(str.substr(begin, end - begin)));
      }
      context->PushArray(result);
    }
  }

  /**
   * words ( str -- str ary )
   *
   * Extracts words from the string and returns them in array.
   */
  static void w_words(const Ref<Context>& context)
  {
    Ref<String> value;

    if (context->PeekString(value))
    {
      const std::string& str = value->GetValue();
      const std::size_t length = str.length();
      std::size_t begin = 0;
      std::size_t end = 0;
      std::vector<Ref<Value>> result;

      for (std::size_t i = 0; i < length; ++i)
      {
        if (std::isspace(str[i]))
        {
          if (end - begin > 0)
          {
            result.push_back(context->GetRuntime()->NewString(str.substr(begin, end - begin)));
          }
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(context->GetRuntime()->NewString(str.substr(begin, end - begin)));
      }
      context->PushArray(result);
    }
  }

  /**
   * lower ( str -- str )
   *
   * Converts string to lower case.
   */
  static void w_to_lower(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PopString(string))
    {
      const char* input = string->GetValue().c_str();
      char buffer[5];
      std::string result;

      while (*input)
      {
        unsigned int cp;

        if (!unicode_decode(input, cp))
        {
          break;
        }
        input += unicode_size(cp);
        cp = unicode_tolower(cp);
        if (unicode_encode(cp, buffer))
        {
          result += buffer;
        }
      }
      context->PushString(result);
    }
  }

  /**
   * upper ( str -- str )
   *
   * Converts string to upper case.
   */
  static void w_to_upper(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PopString(string))
    {
      const char* input = string->GetValue().c_str();
      char buffer[5];
      std::string result;

      while (*input)
      {
        unsigned int cp;

        if (!unicode_decode(input, cp))
        {
          break;
        }
        input += unicode_size(cp);
        cp = unicode_toupper(cp);
        if (unicode_encode(cp, buffer))
        {
          result += buffer;
        }
      }
      context->PushString(result);
    }
  }

  /**
   * capitalize ( str -- str )
   *
   * Converts first character of the string into upper case and remaining to
   * lower case.
   */
  static void w_capitalize(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PopString(string))
    {
      const char* input = string->GetValue().c_str();
      bool first = true;
      char buffer[5];
      std::string result;

      while (*input)
      {
        unsigned int cp;

        if (!unicode_decode(input, cp))
        {
          break;
        }
        input += unicode_size(cp);
        if (first)
        {
          cp = unicode_toupper(cp);
          first = false;
        } else {
          cp = unicode_tolower(cp);
        }
        if (unicode_encode(cp, buffer))
        {
          result += buffer;
        }
      }
      context->PushString(result);
    }
  }

  /**
   * reverse ( str -- str )
   *
   * Returns reversed copy of the string.
   */
  static void w_reverse(const Ref<Context>& context)
  {
    Ref<String> string;

    if (context->PopString(string))
    {
      const char* input = string->GetValue().c_str();
      std::string output;
      char buffer[5];

      while (input && *input)
      {
        unsigned int rune;
        std::size_t size;

        if (!unicode_decode(input, rune) || !unicode_encode(rune, buffer))
        {
          break;
        }
        size = unicode_size(rune);
        input += size;
        buffer[size] = 0;
        output.insert(0, buffer);
      }
      context->PushString(output);
    }
  }

  /**
   * = ( str str -- bool )
   *
   * Tests whether two strings are equal.
   */
  static void w_eq(const Ref<Context>& context)
  {
    Ref<String> a;
    Ref<String> b;

    if (context->PopString(a) && context->PopString(b))
    {
      context->PushBool(!a->GetValue().compare(b->GetValue()));
    }
  }

  /**
   * + ( str str -- str )
   *
   * Concatenates contents of two strings into one.
   */
  static void w_plus(const Ref<Context>& context)
  {
    Ref<String> a;
    Ref<String> b;

    if (context->PopString(a) && context->PopString(b))
    {
      context->Push(context->GetRuntime()->NewString(b->GetValue() + a->GetValue()));
    }
  }

  /**
   * * ( num str -- str )
   *
   * Repeats string given number of times.
   */
  static void w_times(const Ref<Context>& context)
  {
    Ref<String> string;
    Ref<Number> number;
    std::string result;

    if (!context->PopString(string) || !context->PopNumber(number))
    {
      return;
    }
    else if (number->GetNumberType() == Number::NUMBER_TYPE_INT)
    {
      std::int64_t times = number.As<IntNumber>()->GetValue();

      for (std::int64_t i = 0; i < times; ++i)
      {
        result += string->GetValue();
      }
    } else {
      mpz_class times = number->AsBigInt();

      for (mpz_class i = 0; i < times; ++i)
      {
        result += string->GetValue();
      }
    }
    context->PushString(result);
  }

  void api_init_string(Runtime* runtime)
  {
    runtime->AddWord("str?", w_is_str);
    runtime->AddWord(">str", w_to_str);

    runtime->AddNamespace("str", {
      { "len", w_len },
      { "size", w_size },

      { "empty?", w_is_empty },
      { "blank?", w_is_blank },

      { "chars", w_chars },
      { "runes", w_runes },
      { "lines", w_lines },
      { "words", w_words },

      { "lower", w_to_lower },
      { "upper", w_to_upper },
      { "capitalize", w_capitalize },
      { "reverse", w_reverse },

      { "=", w_eq },
      { "+", w_plus },
      { "*", w_times },
    });
  }
}

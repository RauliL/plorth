#include <sstream>

#include "array.hpp"
#include "state.hpp"
#include "string.hpp"
#include "token.hpp"
#include "unicode.hpp"
#include "utils.hpp"

namespace plorth
{
  static bool compile_string(const Ref<State>&,
                             const char,
                             std::string::const_iterator&,
                             const std::string::const_iterator&,
                             std::vector<Token>&,
                             std::string&);
  static Ref<Value> parse_value(const Ref<State>&,
                                const std::vector<Token>&,
                                std::size_t&,
                                const std::size_t);
  static bool parse_declaration(const Ref<State>&,
                                const std::vector<Token>&,
                                std::size_t&,
                                const std::size_t);

  Ref<Quote> Quote::Compile(const Ref<State>& state, const std::string& source)
  {
    std::vector<Token> tokens;
    std::string buffer;
    auto current = source.cbegin();
    const auto end = source.cend();

    while (current < end)
    {
      switch (*current)
      {
        // Skip line comments.
        case '#':
          while (++current < end)
          {
            if (*current == '\n' || *current == '\r')
            {
              break;
            }
          }
          break;

        // Skip whitespace.
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          ++current;
          break;

        // Separator characters.
        case '(': case ')':
        case '[': case ']':
        case '{': case '}':
        case ':': case ',':
        {
          const char c = *current++;

          tokens.push_back(Token(
            c == '(' ? Token::TYPE_LPAREN :
            c == ')' ? Token::TYPE_RPAREN :
            c == '[' ? Token::TYPE_LBRACK :
            c == ']' ? Token::TYPE_RBRACK :
            c == '{' ? Token::TYPE_LBRACE :
            c == '}' ? Token::TYPE_RBRACE :
            c == ':' ? Token::TYPE_COLON :
            Token::TYPE_COMMA
          ));
          break;
        }

        case '\'':
        case '"':
        {
          const char separator = *current++;

          if (!compile_string(state, separator, current, end, tokens, buffer))
          {
            return Ref<Quote>();
          }
          break;
        }

        default:
          buffer.assign(1, *current++);
          while (current < end && is_word_part(*current))
          {
            buffer.append(1, *current++);
          }
          tokens.push_back(Token(Token::TYPE_WORD, buffer));
      }
    }

    return state->GetRuntime()->NewQuote(tokens);
  }

  Ref<Object> Quote::GetPrototype(const Ref<Runtime>& runtime) const
  {
    Ref<Value> value;

    if (runtime->FindWord("quote", value) && value->GetType() == TYPE_OBJECT)
    {
      return value.As<Object>();
    } else {
      return Ref<Object>();
    }
  }

  bool Quote::Equals(const Ref<Value>& that) const
  {
    return false; // TODO: Implement equality testing for native quotes.
  }

  CompiledQuote::CompiledQuote(const std::vector<Token>& tokens)
    : m_tokens(tokens) {}

  bool CompiledQuote::Call(const Ref<State>& state) const
  {
    const std::size_t size = m_tokens.size();

    for (std::size_t i = 0; i < size;)
    {
      const Token& token = m_tokens[i];

      switch (token.GetType())
      {
        case Token::TYPE_STRING:
        case Token::TYPE_LBRACK:
        case Token::TYPE_LBRACE:
        {
          const Ref<Value> value = parse_value(state, m_tokens, i, size);

          if (!value)
          {
            return false;
          }
          state->Push(value);
          break;
        }

        case Token::TYPE_COLON:
          if (!parse_declaration(state, m_tokens, ++i, size))
          {
            return false;
          }
          break;

        case Token::TYPE_WORD:
          ++i;
          if (!state->CallWord(token.GetData()))
          {
            return false;
          }
          break;

        case Token::TYPE_LPAREN:
        case Token::TYPE_RPAREN:
        case Token::TYPE_RBRACK:
        case Token::TYPE_RBRACE:
        case Token::TYPE_COMMA:
        {
          std::stringstream ss;

          ss << "Unexpected " << token.GetType();
          state->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

          return false;
        }
      }
    }

    return true;
  }

  std::string CompiledQuote::ToString() const
  {
    std::string result;

    result += "{";
    for (std::size_t i = 0; i < m_tokens.size(); ++i)
    {
      if (i > 0)
      {
        result += " ";
      }
      result += m_tokens[i].ToSource();
    }
    result += "}";

    return result;
  }

  NativeQuote::NativeQuote(CallbackSignature callback)
    : m_callback(callback) {}

  bool NativeQuote::Call(const Ref<State>& state) const
  {
    m_callback(state);

    return !state->GetError();
  }

  std::string NativeQuote::ToString() const
  {
    return "<native quote>";
  }

  static bool compile_string(const Ref<State>& state,
                             const char separator,
                             std::string::const_iterator& current,
                             const std::string::const_iterator& end,
                             std::vector<Token>& tokens,
                             std::string& buffer)
  {
    buffer.clear();
    while (current < end && *current != separator)
    {
      if (*current == '\\')
      {
        char c;

        if (++current == end)
        {
          state->SetError(
            Error::ERROR_CODE_SYNTAX,
            "Unterminated string literal."
          );

          return false;
        }
        switch (c = *current++)
        {
          case 'b':
            buffer.append(1, 010);
            break;

          case 't':
            buffer.append(1, 011);
            break;

          case 'n':
            buffer.append(1, 012);
            break;

          case 'f':
            buffer.append(1, 014);
            break;

          case 'r':
            buffer.append(1, 015);
            break;

          case '"':
          case '\'':
          case '\\':
          case '/':
            buffer.append(1, c);
            break;

          case 'u':
          {
            unsigned int result = 0;

            for (int i = 0; i < 4; ++i)
            {
              int hex;

              if (current == end || !std::isxdigit(hex = *current++))
              {
                state->SetError(
                  Error::ERROR_CODE_SYNTAX,
                  "Illegal Unicode hex escape sequence."
                );

                return false;
              }
              else if (hex >= 'A' && hex <= 'F')
              {
                result = result * 16 + (hex - 'A' + 10);
              }
              else if (hex >= 'a' && hex <= 'f')
              {
                result = result * 16 + (hex - 'a' + 10);
              } else {
                result = result * 16 + (hex - '0');
              }
            }
            if (!unicode_encode(result, buffer))
            {
              state->SetError(
                Error::ERROR_CODE_SYNTAX,
                "Illegal Unicode hex escape sequence."
              );

              return false;
            }
            break;
          }

          default:
            state->SetError(
              Error::ERROR_CODE_SYNTAX,
              "Illegal escape sequence in string literal."
            );

            return false;
        }
      } else {
        buffer.append(1, *current++);
      }
    }
    if (current >= end)
    {
      state->SetError(
        Error::ERROR_CODE_SYNTAX,
        "Unterminated string literal."
      );

      return false;
    }
    ++current;
    tokens.push_back(Token(Token::TYPE_STRING, buffer));

    return true;
  }

  static Ref<Value> parse_array(const Ref<State>& state,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    std::vector<Ref<Value>> elements;

    for (;;)
    {
      if (index >= size)
      {
        state->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Unterminated array literal."
        );

        return Ref<Value>();
      }
      else if (tokens[index].GetType() == Token::TYPE_RBRACK)
      {
        ++index;
        break;
      } else {
        const Ref<Value> value = parse_value(state, tokens, index, size);

        if (!value)
        {
          return Ref<Value>();
        }
        elements.push_back(value);
        if (index >= size)
        {
          state->SetError(
            Error::ERROR_CODE_SYNTAX,
            "Unterminated array literal: Missing `]'"
          );

          return Ref<Value>();
        }
        else if (tokens[index].GetType() == Token::TYPE_COMMA)
        {
          ++index;
        }
        else if (tokens[index].GetType() != Token::TYPE_RBRACK)
        {
          std::stringstream ss;

          ss << "Unexpected " << tokens[index].GetType() << "; Missing `]'";
          state->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

          return Ref<Value>();
        }
      }
    }

    return state->GetRuntime()->NewArray(elements);
  }

  static Ref<Value> parse_object(const Ref<State>& state,
                                 const std::vector<Token>& tokens,
                                 std::size_t& index,
                                 const std::size_t size)
  {
    Object::Dictionary properties;

    for (;;)
    {
      std::string key;
      Ref<Value> value;

      if (index >= size)
      {
        state->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Missing key for object literal: Missing `}'"
        );

        return Ref<Value>();
      }
      else if (tokens[index].GetType() == Token::TYPE_RBRACE)
      {
        ++index;
        break;
      }
      else if (tokens[index].GetType() != Token::TYPE_STRING)
      {
        state->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Missing key for object literal."
        );

        return Ref<Value>();
      }
      key = tokens[index++].GetData();
      if (index >= size || tokens[index].GetType() != Token::TYPE_COLON)
      {
        state->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Missing `:' after key of an object."
        );

        return Ref<Value>();
      }
      else if (!(value = parse_value(state, tokens, ++index, size)))
      {
        return Ref<Value>();
      }
      properties[key] = value;
      if (index >= size)
      {
        state->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Unterminated object literal: Missing `}'"
        );

        return Ref<Value>();
      }
      else if (tokens[index].GetType() == Token::TYPE_COMMA)
      {
        ++index;
      }
      else if (tokens[index].GetType() != Token::TYPE_RBRACE)
      {
        std::stringstream ss;

        ss << "Unexpected " << tokens[index].GetType() << "; Missing `]'";
        state->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

        return Ref<Value>();
      }
    }

    return state->GetRuntime()->NewObject(properties);
  }

  static Ref<Quote> parse_quote(const Ref<State>& state,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    std::vector<Token> result;
    std::size_t counter = 1;

    while (index < size)
    {
      const Token& token = tokens[index++];

      if (token.GetType() == Token::TYPE_LBRACE)
      {
        ++counter;
      }
      else if (token.GetType() == Token::TYPE_RBRACE && !--counter)
      {
        break;
      }
      result.push_back(token);
    }
    if (counter > 0)
    {
      state->SetError(Error::ERROR_CODE_SYNTAX, "Unterminated quote.");

      return Ref<Quote>();
    }

    return state->GetRuntime()->NewQuote(result);
  }

  static Ref<Value> parse_value(const Ref<State>& state,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    const Token& token = tokens[index++];

    switch (token.GetType())
    {
      case Token::TYPE_STRING:
        return state->GetRuntime()->NewString(token.GetData());

      case Token::TYPE_LBRACK:
        return parse_array(state, tokens, index, size);

      case Token::TYPE_LBRACE:
        // Empty object literal?
        if (index < size
            && tokens[index].GetType() == Token::TYPE_RBRACE)
        {
          ++index;

          return state->GetRuntime()->NewObject(std::unordered_map<std::string, Ref<Value>>());
        }
        // Object literal?
        else if (index + 1 < size
                && tokens[index].GetType() == Token::TYPE_STRING
                && tokens[index + 1].GetType() == Token::TYPE_COLON)
        {
          return parse_object(state, tokens, index, size);
        } else {
          return parse_quote(state, tokens, index, size);
        }

      case Token::TYPE_WORD:
      {
        const std::string& text = token.GetData();

        if (!text.compare("null"))
        {
          return state->GetRuntime()->GetNullValue();
        }
        else if (!text.compare("true"))
        {
          return state->GetRuntime()->GetTrueValue();
        }
        else if (!text.compare("false"))
        {
          return state->GetRuntime()->GetFalseValue();
        }
        else if (str_is_number(text))
        {
          return state->GetRuntime()->NewNumber(text);
        }
      }

      default:
      {
        std::stringstream ss;

        ss << "Unexpected " << token.GetType() << "; Missing value.";
        state->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

        return Ref<Value>();
      }
    }
  }

  static bool parse_declaration(const Ref<State>& state,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    std::string name;
    Ref<Value> value;

    if (index >= size || tokens[index].GetType() != Token::TYPE_WORD)
    {
      state->SetError(
        Error::ERROR_CODE_SYNTAX,
        "Missing name after word declaration."
      );

      return false;
    }
    name = tokens[index++].GetData();
    if (!(value = parse_value(state, tokens, index, size)))
    {
      return false;
    }
    state->AddWord(name, value);

    return true;
  }

  /**
   * quote? ( any -- any bool )
   *
   * Returns true if given value is quote.
   */
  static void w_is_quote(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Peek(value))
    {
      state->PushBool(value->GetType() == Value::TYPE_QUOTE);
    }
  }

  /**
   * compile ( str -- quote )
   *
   * Compiles given string into quote.
   */
  static void w_compile(const Ref<State>& state)
  {
    Ref<String> source;

    if (state->PopString(source))
    {
      const Ref<Quote> quote = Quote::Compile(state, source->GetValue());

      if (quote)
      {
        state->Push(quote);
      }
    }
  }

  /**
   * globals ( -- obj )
   *
   * Returns global dictionary as object.
   */
  static void w_globals(const Ref<State>& state)
  {
    state->PushObject(state->GetRuntime()->GetDictionary());
  }

  /**
   * locals ( -- obj )
   *
   * Returns local dictionary of current program execution state as object.
   */
  static void w_locals(const Ref<State>& state)
  {
    state->PushObject(state->GetDictionary());
  }

  /**
   * call ( quote -- )
   *
   * Executes quote taken from top of the stack.
   */
  static void w_call(const Ref<State>& state)
  {
    Ref<Quote> quote;

    if (state->PopQuote(quote))
    {
      quote->Call(state);
    }
  }

  namespace
  {
    class NegatedQuote : public Quote
    {
    public:
      explicit NegatedQuote(const Ref<Quote>& delegate)
        : m_delegate(delegate) {}

      bool Call(const Ref<State>& state) const
      {
        Ref<Bool> value;

        if (!m_delegate->Call(state) || !state->PopBool(value))
        {
          return false;
        }
        state->PushBool(!value->GetValue());

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
  static void w_negate(const Ref<State>& state)
  {
    Ref<Quote> delegate;

    if (state->PopQuote(delegate))
    {
      state->Push(new (state->GetRuntime()) NegatedQuote(delegate));
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

      bool Call(const Ref<State>& state) const
      {
        return m_first->Call(state) && m_second->Call(state);
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
  static void w_plus(const Ref<State>& state)
  {
    Ref<Quote> first;
    Ref<Quote> second;

    if (state->PopQuote(second) && state->PopQuote(first))
    {
      state->Push(new (state->GetRuntime()) JoinedQuote(first, second));
    }
  }

  void api_init_quote(Runtime* runtime)
  {
    runtime->AddWord("quote?", w_is_quote);
    runtime->AddWord("compile", w_compile);
    runtime->AddWord("globals", w_globals);
    runtime->AddWord("locals", w_locals);

    runtime->AddNamespace("quote", {
      { "call", w_call },
      { "negate", w_negate },
      { "+", w_plus },
    });
  }
}

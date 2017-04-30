#include <sstream>

#include "array.hpp"
#include "context.hpp"
#include "string.hpp"
#include "token.hpp"
#include "unicode.hpp"
#include "utils.hpp"

namespace plorth
{
  static bool compile_string(const Ref<Context>&,
                             const char,
                             std::string::const_iterator&,
                             const std::string::const_iterator&,
                             std::vector<Token>&,
                             std::string&);
  static Ref<Value> parse_value(const Ref<Context>&,
                                const std::vector<Token>&,
                                std::size_t&,
                                const std::size_t);
  static bool parse_declaration(const Ref<Context>&,
                                const std::vector<Token>&,
                                std::size_t&,
                                const std::size_t);

  namespace
  {
    class CompiledQuote : public Quote
    {
    public:
      explicit CompiledQuote(const std::vector<Token>& tokens)
        : m_tokens(tokens) {}

      bool Call(const Ref<Context>& context) const
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
              const Ref<Value> value = parse_value(context, m_tokens, i, size);

              if (!value)
              {
                return false;
              }
              context->Push(value);
              break;
            }

            case Token::TYPE_COLON:
              if (!parse_declaration(context, m_tokens, ++i, size))
              {
                return false;
              }
              break;

            case Token::TYPE_WORD:
              ++i;
              if (!context->CallWord(token.GetData()))
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
              context->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

              return false;
            }
          }
        }

        return true;
      }

      std::string ToString() const
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

    private:
      const std::vector<Token> m_tokens;
    };
  }

  Ref<Quote> Quote::Compile(const Ref<Context>& context, const std::string& source)
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

          if (!compile_string(context, separator, current, end, tokens, buffer))
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

    return new (context->GetRuntime()) CompiledQuote(tokens);
  }

  Ref<Object> Quote::GetPrototype(const Ref<Runtime>& runtime) const
  {
    return runtime->GetQuotePrototype();
  }

  bool Quote::Equals(const Ref<Value>& that) const
  {
    return false; // TODO: Implement equality testing for native quotes.
  }

  static bool compile_string(const Ref<Context>& context,
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
          context->SetError(
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
                context->SetError(
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
              context->SetError(
                Error::ERROR_CODE_SYNTAX,
                "Illegal Unicode hex escape sequence."
              );

              return false;
            }
            break;
          }

          default:
            context->SetError(
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
      context->SetError(
        Error::ERROR_CODE_SYNTAX,
        "Unterminated string literal."
      );

      return false;
    }
    ++current;
    tokens.push_back(Token(Token::TYPE_STRING, buffer));

    return true;
  }

  static Ref<Value> parse_array(const Ref<Context>& context,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    std::vector<Ref<Value>> elements;

    for (;;)
    {
      if (index >= size)
      {
        context->SetError(
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
        const Ref<Value> value = parse_value(context, tokens, index, size);

        if (!value)
        {
          return Ref<Value>();
        }
        elements.push_back(value);
        if (index >= size)
        {
          context->SetError(
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
          context->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

          return Ref<Value>();
        }
      }
    }

    return context->GetRuntime()->NewArray(elements);
  }

  static Ref<Value> parse_object(const Ref<Context>& context,
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
        context->SetError(
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
        context->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Missing key for object literal."
        );

        return Ref<Value>();
      }
      key = tokens[index++].GetData();
      if (index >= size || tokens[index].GetType() != Token::TYPE_COLON)
      {
        context->SetError(
          Error::ERROR_CODE_SYNTAX,
          "Missing `:' after key of an object."
        );

        return Ref<Value>();
      }
      else if (!(value = parse_value(context, tokens, ++index, size)))
      {
        return Ref<Value>();
      }
      properties[key] = value;
      if (index >= size)
      {
        context->SetError(
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
        context->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

        return Ref<Value>();
      }
    }

    return context->GetRuntime()->NewObject(properties);
  }

  static Ref<Quote> parse_quote(const Ref<Context>& context,
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
      context->SetError(Error::ERROR_CODE_SYNTAX, "Unterminated quote.");

      return Ref<Quote>();
    }

    return new (context->GetRuntime()) CompiledQuote(result);
  }

  static Ref<Value> parse_value(const Ref<Context>& context,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    const Token& token = tokens[index++];

    switch (token.GetType())
    {
      case Token::TYPE_STRING:
        return context->GetRuntime()->NewString(token.GetData());

      case Token::TYPE_LBRACK:
        return parse_array(context, tokens, index, size);

      case Token::TYPE_LBRACE:
        // Empty object literal?
        if (index < size
            && tokens[index].GetType() == Token::TYPE_RBRACE)
        {
          ++index;

          return context->GetRuntime()->NewObject();
        }
        // Object literal?
        else if (index + 1 < size
                && tokens[index].GetType() == Token::TYPE_STRING
                && tokens[index + 1].GetType() == Token::TYPE_COLON)
        {
          return parse_object(context, tokens, index, size);
        } else {
          return parse_quote(context, tokens, index, size);
        }

      case Token::TYPE_WORD:
      {
        const std::string& text = token.GetData();

        if (!text.compare("null"))
        {
          return context->GetRuntime()->GetNullValue();
        }
        else if (!text.compare("true"))
        {
          return context->GetRuntime()->GetTrueValue();
        }
        else if (!text.compare("false"))
        {
          return context->GetRuntime()->GetFalseValue();
        }
        else if (str_is_number(text))
        {
          return context->GetRuntime()->NewNumber(text);
        }
      }

      default:
      {
        std::stringstream ss;

        ss << "Unexpected " << token.GetType() << "; Missing value.";
        context->SetError(Error::ERROR_CODE_SYNTAX, ss.str());

        return Ref<Value>();
      }
    }
  }

  static bool parse_declaration(const Ref<Context>& context,
                                const std::vector<Token>& tokens,
                                std::size_t& index,
                                const std::size_t size)
  {
    std::string name;
    Ref<Value> value;

    if (index >= size || tokens[index].GetType() != Token::TYPE_WORD)
    {
      context->SetError(
        Error::ERROR_CODE_SYNTAX,
        "Missing name after word declaration."
      );

      return false;
    }
    name = tokens[index++].GetData();
    if (!(value = parse_value(context, tokens, index, size)))
    {
      return false;
    }
    context->AddWord(name, value);

    return true;
  }
}

#include <plorth/plorth-token.hpp>

#include "./utils.hpp"

namespace plorth
{
  Token::Token(Type type, const std::string& data)
    : m_type(type)
    , m_data(data) {}

  Token::Token(const Token& that)
    : m_type(that.m_type)
    , m_data(that.m_data) {}

  std::string Token::ToSource() const
  {
    switch (m_type)
    {
      case Token::TYPE_LPAREN:
        return "(";

      case Token::TYPE_RPAREN:
        return ")";

      case Token::TYPE_LBRACK:
        return "[";

      case Token::TYPE_RBRACK:
        return "]";

      case Token::TYPE_LBRACE:
        return "{";

      case Token::TYPE_RBRACE:
        return "}";

      case Token::TYPE_COLON:
        return ":";

      case Token::TYPE_COMMA:
        return ",";

      case Token::TYPE_WORD:
        return m_data;

      case Token::TYPE_STRING:
        return to_json_string(m_data);
    }

    return "<unknown token>";
  }

  std::ostream& operator<<(std::ostream& os, const Token& token)
  {
    os << token.ToSource();

    return os;
  }

  std::ostream& operator<<(std::ostream& os, Token::Type type)
  {
    switch (type)
    {
      case Token::TYPE_LPAREN:
        os << "`('";
        break;

      case Token::TYPE_RPAREN:
        os << "`)'";
        break;

      case Token::TYPE_LBRACK:
        os << "`['";
        break;

      case Token::TYPE_RBRACK:
        os << "`]'";
        break;

      case Token::TYPE_LBRACE:
        os << "`{'";
        break;

      case Token::TYPE_RBRACE:
        os << "`}'";
        break;

      case Token::TYPE_COLON:
        os << "`:'";
        break;

      case Token::TYPE_COMMA:
        os << "`,'";
        break;

      case Token::TYPE_WORD:
        os << "word";
        break;

      case Token::TYPE_STRING:
        os << "string literal";
        break;
    }

    return os;
  }
}

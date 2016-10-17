#ifndef PLORTH_TOKEN_HPP_GUARD
#define PLORTH_TOKEN_HPP_GUARD

#include <iostream>

#include "plorth.hpp"

namespace plorth
{
  class Token
  {
  public:
    enum Type
    {
      TYPE_LPAREN = 1,
      TYPE_RPAREN = 2,
      TYPE_LBRACK = 3,
      TYPE_RBRACK = 4,
      TYPE_LBRACE = 5,
      TYPE_RBRACE = 6,
      TYPE_COLON  = 7,
      TYPE_COMMA  = 8,
      TYPE_WORD   = 9,
      TYPE_STRING = 10
    };

    /**
     * Constructs new token.
     *
     * \param type Type of the token.
     * \param data Textual data associated with the token.
     */
    explicit Token(Type type = TYPE_WORD, const std::string& data = std::string());

    /**
     * Constructs copy of existing token.
     */
    Token(const Token& that);

    /**
     * Assignment operator.
     */
    Token& operator=(const Token& that);

    /**
     * Returns type of the token.
     */
    inline Type GetType() const
    {
      return m_type;
    }

    /**
     * Returns textual data associated with the token, or empty string if there
     * is no text associated with this token.
     */
    inline const std::string& GetData() const
    {
      return m_data;
    }

    std::string ToSource() const;

  private:
    /** Type of the token. */
    Type m_type;
    /** Textual data associated with the token. */
    std::string m_data;
  };

  std::ostream& operator<<(std::ostream&, const Token&);
  std::ostream& operator<<(std::ostream&, Token::Type);
}

#endif /* !PLORTH_TOKEN_HPP_GUARD */

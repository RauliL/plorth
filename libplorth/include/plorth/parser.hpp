/*
 * Copyright (c) 2017-2018, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PLORTH_PARSER_HPP_GUARD
#define PLORTH_PARSER_HPP_GUARD

#include <plorth/token.hpp>

namespace plorth
{
  /**
   * Parser for the Plorth programming language.
   */
  class parser
  {
  public:
    /**
     * Constructs new parser.
     *
     * \param source   Source code to parse.
     * \param filename Optional file name information from which the source
     *                 code was read from.
     * \param line     Initial line number of the source code.
     * \param column   Initial column number of the source code.
     */
    explicit parser(
      const std::u32string& source,
      const std::u32string& filename = U"<eval>",
      int line = 1,
      int column = 0
    );

    /**
     * Returns the current position in source code.
     */
    inline const struct position& position() const
    {
      return m_position;
    }

    /**
     * Returns the error message encountered during parsing or empty string if
     * no syntax error has been encountered.
     */
    inline const std::u32string& error() const
    {
      return m_error;
    }

    /**
     * Attempts to parse top level script.
     *
     * \param container Where parsed tokens will be inserted into.
     * \return          A boolean flag describing whether the parsing was
     *                  successful or not.
     */
    bool parse(std::vector<std::shared_ptr<token>>& container);

    parser(const parser&) = delete;
    parser(parser&&) = delete;
    void operator=(const parser&) = delete;
    void operator=(parser&&) = delete;

  private:
    std::shared_ptr<token> parse_value();
    std::shared_ptr<token::array> parse_array();
    std::shared_ptr<token::object> parse_object();
    std::shared_ptr<token::quote> parse_quote();
    std::shared_ptr<token::string> parse_string();
    std::shared_ptr<token::symbol> parse_symbol();
    std::shared_ptr<token::word> parse_word();

    /**
     * Returns true if there are no more characters to be read from the source
     * code.
     */
    inline bool eof() const
    {
      return m_pos >= m_end;
    }

    /**
     * Advances to the next character in source code, discarding the current
     * one.
     */
    inline void advance()
    {
      read();
    }

    /**
     * Advances to the next character in source code and returns the current
     * one.
     */
    char32_t read();

    /**
     * Returns the next character to be read from the source code without
     * advancing any further.
     */
    inline char32_t peek() const
    {
      return *m_pos;
    }

    /**
     * Returns true if the next character to be read from the source code
     * matches with one given as argument.
     */
    inline bool peek(char32_t expected) const
    {
      return !eof() && peek() == expected;
    }

    /**
     * Returns true and advances to next character in the source code if next
     * character to be read from the source code matches with the one given as
     * argument.
     */
    bool peek_read(char32_t expected);

    /**
     * Skips whitespace and comments from the source code.
     *
     * \return True if end of input has been reached, false otherwise.
     */
    bool skip_whitespace();

    bool parse_escape_sequence(std::u32string& buffer);

  private:
    /** Current position in the source code. */
    std::u32string::const_iterator m_pos;
    /** End of the source code. */
    const std::u32string::const_iterator m_end;
    /** Line and column number tracking. */
    struct position m_position;
    /** Container for error messages. */
    std::u32string m_error;
  };
}

#endif /* !PLORTH_PARSER_HPP_GUARD */

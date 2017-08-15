#ifndef PLORTH_UNICODE_HPP_GUARD
#define PLORTH_UNICODE_HPP_GUARD

#include <cstdint>

#include <iostream>
#include <string>

namespace plorth
{
  using unichar = std::uint32_t;
  using unistring = std::basic_string<unichar>;

  unistring operator+(const char*, const unistring&);
  unistring operator+(const unistring&, const char*);
  std::ostream& operator<<(std::ostream&, const unistring&);

  /**
   * Decodes UTF-8 encoded byte string into Unicode string. Encountered encoding
   * errors are ignored.
   */
  unistring utf8_decode(const std::string&);

  /**
   * Encodes Unicode string into UTF-8 encoded byte string.
   */
  std::string utf8_encode(const unistring&);

  /**
   * Decodes single Unicode code point from given byte string iterator.
   *
   * \param it Current position of the byte string iterator.
   * \param end Ending point of the byte string iterator.
   * \param output Where the decoded Unicode code point will be stored into.
   * \return       Boolean flag which tells whether the decoding was successfull
   *               or not.
   */
  bool utf8_advance(std::string::const_iterator&,
                    const std::string::const_iterator&,
                    unichar&);

  /**
   * Determines whether given character is valid Unicode code point.
   */
  bool unichar_validate(unichar);

  /**
   * Determines whether a character is a control character.
   */
  bool unichar_iscntrl(unichar);

  /**
   * Determines whether a character is printable and not a space.
   */
  bool unichar_isgraph(unichar);

  /**
   * Determines whether a character is whitespace.
   */
  bool unichar_isspace(unichar);

  /**
   * Determines whether a character is upper case.
   */
  bool unichar_isupper(unichar);

  /**
   * Determines whether a character is lower case.
   */
  bool unichar_islower(unichar);

  /**
   * Determines whether a character can be part of Plorth word.
   */
  bool unichar_isword(unichar);

  /**
   * Converts given Unicode character into upper case.
   */
  unichar unichar_toupper(unichar);

  /**
   * Converts given Unicode character into lower case.
   */
  unichar unichar_tolower(unichar);
}

namespace std
{
  /**
   * Custom C++11 hashing function for Unicode strings. The hashing algorithm
   * is stol^H^H^H^Hborrowed from Java.
   */
  template<>
  struct hash<plorth::unistring>
  {
    using argument_type = plorth::unistring;
    using result_type = std::int32_t;

    result_type operator()(const plorth::unistring& s) const
    {
      const auto length = s.length();
      result_type result = 0;

      for (plorth::unistring::size_type i = 0; i < length; ++i)
      {
        result = 31 * result + s[i];
      }

      return result;
    }
  };
}

#endif /* !PLORTH_UNICODE_HPP_GUARD */

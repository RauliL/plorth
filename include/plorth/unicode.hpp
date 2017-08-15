/*
 * Copyright (c) 2017, Rauli Laine
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

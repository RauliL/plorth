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
  using unichar = char32_t;
  using unistring = std::u32string;
  using uniostream = std::basic_ostream<unichar>;

  /**
   * Decodes Unicode string into UTF-8 and outputs it into given byte stream.
   */
  std::ostream& operator<<(std::ostream&, const unistring&);

  /**
   * Decodes UTF-8 encoded byte string into Unicode string. Encountered encoding
   * errors are ignored.
   */
  unistring utf8_decode(const std::string&);

  /**
   * Decodes UTF-8 encoded byte string into Unicode string with validation.
   */
  bool utf8_decode_test(const std::string&, unistring&);

  /**
   * Encodes Unicode string into UTF-8 encoded byte string.
   */
  std::string utf8_encode(const unistring&);

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

#endif /* !PLORTH_UNICODE_HPP_GUARD */

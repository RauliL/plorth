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
#ifndef PLORTH_UNICODE_HPP_GUARD
#define PLORTH_UNICODE_HPP_GUARD

#include <plorth/config.hpp>

#include <cstdint>

#include <iostream>
#include <string>

namespace plorth
{
  /**
   * Decodes UTF-8 encoded byte string into Unicode string. Encountered encoding
   * errors are ignored.
   */
  std::u32string utf8_decode(const std::string&);

  /**
   * Decodes UTF-8 encoded byte string into Unicode string with validation.
   */
  bool utf8_decode_test(const std::string&, std::u32string&);

  /**
   * Encodes given Unicode characters into UTF-8 encoded byte string.
   *
   * \param ptr Pointer to array of characters to encode.
   * \param len Size of the array.
   * \return    Given Unicode characters encoded into UTF-8 byte string.
   */
  std::string utf8_encode(const char32_t*, std::size_t);

  /**
   * Encodes Unicode string into UTF-8 encoded byte string.
   */
  inline std::string utf8_encode(const std::u32string& input)
  {
    return utf8_encode(input.c_str(), input.length());
  }

#if defined(__EMSCRIPTEN__)
  /**
   * Decodes UTF-32LE encoded wide character string into Unicode string.
   * Encountered encoding errors are ignored.
   */
  std::u32string utf32le_decode(const std::wstring&);

  /**
   * Encodes Unicode string into UTF-32LE encoded wide character string.
   */
  std::wstring utf32le_encode(const std::u32string&);
#endif

  /**
   * Determines whether given character is valid Unicode code point.
   */
  bool unicode_validate(char32_t);

  /**
   * Determines whether a character is a control character.
   */
  bool unicode_iscntrl(char32_t);

  /**
   * Determines whether a character is printable and not a space.
   */
  bool unicode_isgraph(char32_t);

  /**
   * Determines whether a character is whitespace.
   */
  bool unicode_isspace(char32_t);

  /**
   * Determines whether a character is upper case.
   */
  bool unicode_isupper(char32_t);

  /**
   * Determines whether a character is lower case.
   */
  bool unicode_islower(char32_t);

  /**
   * Determines whether a character can be part of Plorth word.
   */
  bool unicode_isword(char32_t);

  /**
   * Converts given Unicode character into upper case.
   */
  char32_t unicode_toupper(char32_t);

  /**
   * Converts given Unicode character into lower case.
   */
  char32_t unicode_tolower(char32_t);

  /**
   * Attempts to determine length (in bytes) of UTF-8 sequence which begins
   * with the given byte. If the length cannot be determined (i.e. beginning of
   * sequence is invalid according to UTF-8 specification), 0 will be returned
   * instead.
   */
  std::size_t utf8_sequence_length(unsigned char);
}

#endif /* !PLORTH_UNICODE_HPP_GUARD */

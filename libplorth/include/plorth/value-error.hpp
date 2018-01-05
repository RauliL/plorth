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
#ifndef PLORTH_VALUE_ERROR_HPP_GUARD
#define PLORTH_VALUE_ERROR_HPP_GUARD

#include <plorth/position.hpp>
#include <plorth/value.hpp>

namespace plorth
{
  class error : public value
  {
  public:
    enum code
    {
      /** Syntax error. */
      code_syntax = 1,
      /** Reference error. */
      code_reference = 2,
      /** Type error. */
      code_type = 3,
      /** Value error. */
      code_value = 4,
      /** Range error. */
      code_range = 5,
      /** Import error. */
      code_import = 6,
      /** Unknown error. */
      code_unknown = 100
    };

    /**
     * Constructs new error instance.
     *
     * \param code     Error code
     * \param message  Textual description of the error
     * \param position Optional position in source code where the error
     *                 occurred.
     */
    explicit error(enum code code,
                   const unistring& message,
                   const struct position* position = nullptr);

    ~error();

    inline enum code code() const
    {
      return m_code;
    }

    /**
     * Returns textual description of the error code.
     */
    unistring code_description() const;

    /**
     * Returns textual description of given error code.
     */
    static unistring code_description(enum code code);

    inline const unistring& message() const
    {
      return m_message;
    }

    /**
     * Returns position in the source code where the error occurred or null
     * pointer if no such information is available.
     */
    inline const struct position* position() const
    {
      return m_position;
    }

    inline enum type type() const
    {
      return type_error;
    }

    bool equals(const std::shared_ptr<value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

  private:
    /** Error code. */
    const enum code m_code;
    /** Textual description of the error. */
    const unistring m_message;
    /** Optional position in source code. */
    struct position* m_position;
  };

  std::ostream& operator<<(std::ostream&, enum error::code);
}

#endif /* !PLORTH_VALUE_ERROR_HPP_GUARD */

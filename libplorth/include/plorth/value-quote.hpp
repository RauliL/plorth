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
#ifndef PLORTH_VALUE_QUOTE_HPP_GUARD
#define PLORTH_VALUE_QUOTE_HPP_GUARD

#include <plorth/value.hpp>

#include <functional>

namespace plorth
{
  /**
   * Quote is a container for executable code, which can be either tokens
   * compiled from a script or callback to native C++ function.
   */
  class quote : public value
  {
  public:
    /** Signature of C++ function that can be used as quote. */
    using callback = std::function<void(const std::shared_ptr<context>&)>;

    /**
     * Enumeration for different supported quote types.
     */
    enum class quote_type
    {
      native = 0,
      compiled = 1
    };

    /**
     * Invokes the quote.
     *
     * \param ctx Scripting context to execute the quote in.
     * \return    Boolean flag which tells whether execution of the quote was
     *            performed successfully without errors.
     */
    virtual bool call(const std::shared_ptr<context>& ctx) const = 0;

    /**
     * Returns type of the quote.
     */
    virtual enum quote_type quote_type() const = 0;

    /**
     * Tests whether the quote is of given type.
     */
    inline bool is(enum quote_type t) const
    {
      return quote_type() == t;
    }

    inline enum type type() const
    {
      return type::quote;
    }

    std::u32string to_source() const;
  };
}

#endif /* !PLORTH_VALUE_QUOTE_HPP_GUARD */

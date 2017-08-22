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
#ifndef PLORTH_VALUE_HPP_GUARD
#define PLORTH_VALUE_HPP_GUARD

#include <plorth/memory.hpp>
#include <plorth/unicode.hpp>

namespace plorth
{
  /**
   * Abstract base class for everything that acts as an value in Plorth.
   */
  class value : public memory::managed
  {
  public:
    /**
     * Enumeration of different supported value types.
     */
    enum type
    {
      /** Value for null. */
      type_null,
      /** Boolean values. */
      type_boolean,
      /** Number (floating point) values. */
      type_number,
      /** String (Unicode) values. */
      type_string,
      /** Array values. */
      type_array,
      /** Other type of objects. */
      type_object,
      /** Quotes. */
      type_quote,
      /** Errors. */
      type_error
    };

    /**
     * Returns type of the value.
     */
    virtual enum type type() const = 0;

    /**
     * Tests whether the value is of given type.
     */
    inline bool is(enum type t) const
    {
      return type() == t;
    };

    /**
     * Determines prototype object of the value, based on it's type. If the
     * value is an object, property called "__proto__" will be used instead,
     * with the runtime's object prototype acting as a fallback.
     *
     * \param runtime Script runtime to use for prototype retrieval.
     * \return        Prototype object of the value.
     */
    ref<object> prototype(const ref<class runtime>& runtime) const;

    /**
     * Tests whether two values are equal.
     *
     * \param that Other value to test this one against.
     */
    virtual bool equals(const ref<value>& that) const = 0;

    /**
     * Constructs string representation of the value.
     */
    virtual unistring to_string() const = 0;

    /**
     * Constructs a string that resembles as accurately as possible what this
     * value would look like in source code.
     */
    virtual unistring to_source() const = 0;
  };

  bool operator==(const ref<value>&, const ref<value>&);
  bool operator!=(const ref<value>&, const ref<value>&);

  std::ostream& operator<<(std::ostream&, enum value::type);
  std::ostream& operator<<(std::ostream&, const ref<value>&);

  uniostream& operator<<(uniostream&, enum value::type);
  uniostream& operator<<(uniostream&, const ref<value>&);
}

#endif /* !PLORTH_VALUE_HPP_GUARD */

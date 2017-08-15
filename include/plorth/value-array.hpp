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
#ifndef PLORTH_VALUE_ARRAY_HPP_GUARD
#define PLORTH_VALUE_ARRAY_HPP_GUARD

#include <plorth/value.hpp>

#include <vector>

namespace plorth
{
  /**
   * Array is an indexed sequence of other values. It is one of the basic data
   * types of Plorth.
   */
  class array : public value
  {
  public:
    using container_type = std::vector<ref<value>>;

    /**
     * Constructs new array from given elements.
     */
    explicit array(const container_type& elements);

    /**
     * Returns reference to the underlying container that contains the elements
     * of the array.
     */
    inline const container_type& elements() const
    {
      return m_elements;
    }

    inline enum type type() const
    {
      return type_array;
    }

    bool equals(const ref<value>& that) const;
    unistring to_string () const;
    unistring to_source() const;

  private:
    /** Container for elements of the array. */
    const container_type m_elements;
  };
}

#endif /* !PLORTH_VALUE_ARRAY_HPP_GUARD */

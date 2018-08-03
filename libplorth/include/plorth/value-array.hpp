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
#ifndef PLORTH_VALUE_ARRAY_HPP_GUARD
#define PLORTH_VALUE_ARRAY_HPP_GUARD

#include <iterator>

#include <plorth/value.hpp>

namespace plorth
{
  /**
   * Array is an indexed sequence of other values. It is one of the basic data
   * types of Plorth.
   */
  class array : public value
  {
  public:
    using size_type = std::size_t;
    using value_type = std::shared_ptr<value>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    class iterator;

    /**
     * Returns the number of elements in the array.
     */
    virtual size_type size() const = 0;

    /**
     * Returns element of the array from given index.
     */
    virtual const_reference at(size_type offset) const = 0;

    inline enum type type() const
    {
      return type::array;
    }

    bool equals(const std::shared_ptr<value>& that) const;
    std::u32string to_string() const;
    std::u32string to_source() const;
  };

  /**
   * Iterator implementation for Plorth array.
   */
  class array::iterator
  {
  public:
    using difference_type = int;
    using value_type = const std::shared_ptr<value>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    iterator(const std::shared_ptr<array>& ary, array::size_type index = 0);
    iterator(const iterator& that);
    iterator& operator=(const iterator& that);

    iterator& operator++();
    iterator operator++(int);
    reference operator*();
    reference operator->();

    bool operator==(const iterator& that) const;
    bool operator!=(const iterator& that) const;

  private:
    /** Reference to array which is being iterated. */
    std::shared_ptr<array> m_array;
    /** Current offset in the iterated array. */
    array::size_type m_index;
  };

  inline array::iterator begin(const std::shared_ptr<array>& ary)
  {
    return array::iterator(ary);
  }

  inline array::iterator end(const std::shared_ptr<array>& ary)
  {
    return array::iterator(ary, ary->size());
  }
}

#endif /* !PLORTH_VALUE_ARRAY_HPP_GUARD */

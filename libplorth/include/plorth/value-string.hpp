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
#ifndef PLORTH_VALUE_STRING_HPP_GUARD
#define PLORTH_VALUE_STRING_HPP_GUARD

#include <iterator>

#include <plorth/value.hpp>

namespace plorth
{
  class string : public value
  {
  public:
    using size_type = std::size_t;
    using value_type = unichar;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    /**
     * Tests whether the string is empty.
     */
    inline bool empty() const
    {
      return length() == 0;
    }

    /**
     * Returns length of the string.
     */
    virtual size_type length() const = 0;

    /**
     * Returns Unicode code point from specified offset of the string.
     */
    virtual value_type at(size_type offset) const = 0;

    enum type type() const
    {
      return type_string;
    }

    bool equals(const ref<class value>& that) const;
    unistring to_string() const;
    unistring to_source() const;

    /**
     * Iterator implementation for Plorth string.
     */
    class iterator
    {
    public:
      using difference_type = int;
      using value_type = unichar;
      using pointer = value_type*;
      using reference = value_type&;
      using iterator_category = std::forward_iterator_tag;

      iterator(const ref<string>& str, string::size_type index = 0);
      iterator(const iterator& that);
      iterator& operator=(const iterator& that);

      iterator& operator++();
      iterator operator++(int);
      value_type operator*();

      bool operator==(const iterator& that) const;
      bool operator!=(const iterator& that) const;

    private:
      /** Reference to string which is being iterated. */
      ref<string> m_string;
      /** Current offset in the iterated string. */
      array::size_type m_index;
    };

    inline iterator begin()
    {
      return iterator(this);
    }

    inline iterator end()
    {
      return iterator(this, length());
    }
  };

  inline string::iterator begin(const ref<string>& str)
  {
    return str->begin();
  }

  inline string::iterator end(const ref<string>& str)
  {
    return str->end();
  }
}

#endif /* !PLORTH_VALUE_STRING_HPP_GUARD */

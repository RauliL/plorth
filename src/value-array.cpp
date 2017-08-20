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
#include <plorth/context.hpp>
#include <plorth/value-number.hpp>
#include <plorth/value-string.hpp>

namespace plorth
{
  namespace
  {
    /**
     * Implementation of simple array, which only acts as a wrapper for C type
     * array.
     */
    class simple_array : public array
    {
    public:
      simple_array(size_type size, const_pointer elements)
        : m_size(size)
        , m_elements(size > 0 ? new value_type[size] : nullptr)
      {
        for (size_type i = 0; i < m_size; ++i)
        {
          m_elements[i] = elements[i];
        }
      }

      ~simple_array()
      {
        if (m_size > 0)
        {
          delete[] m_elements;
        }
      }

      size_type size() const
      {
        return m_size;
      }

      const_reference at(size_type i) const
      {
        return m_elements[i];
      }

    private:
      const size_type m_size;
      pointer m_elements;
    };

    /**
     * Implementation of array where two arrays have been concatenated into one.
     */
    class concat_array : public array
    {
    public:
      concat_array(const ref<array>& left, const ref<array>& right)
        : m_left(left)
        , m_right(right) {}

      size_type size() const
      {
        return m_left->size() + m_right->size();
      }

      const_reference at(size_type offset) const
      {
        const size_type left_size = m_left->size();

        if (offset < left_size)
        {
          return m_left->at(offset);
        } else {
          return m_right->at(offset - left_size);
        }
      }

    private:
      const ref<array> m_left;
      const ref<array> m_right;
    };
  }

  bool array::equals(const ref<value>& that) const
  {
    ref<array> ary;

    if (!that || !that->is(type_array))
    {
      return false;
    }

    ary = that.cast<array>();

    if (size() != ary->size())
    {
      return false;
    }

    for (size_type i = 0; i < size(); ++i)
    {
      if (at(i) != ary->at(i))
      {
        return false;
      }
    }

    return true;
  }

  unistring array::to_string() const
  {
    const size_type s = size();
    unistring result;

    for (size_type i = 0; i < s; ++i)
    {
      const_reference element = at(i);

      if (i > 0)
      {
        result += ',';
        result += ' ';
      }
      if (element)
      {
        result += element->to_string();
      }
    }

    return result;
  }

  unistring array::to_source() const
  {
    const size_type s = size();
    unistring result;

    result += '[';
    for (size_type i = 0; i < s; ++i)
    {
      const_reference element = at(i);

      if (i > 0)
      {
        result += ',';
        result += ' ';
      }
      if (element)
      {
        result += element->to_source();
      } else {
        result += utf8_decode("null");
      }
    }
    result += ']';

    return result;
  }

  ref<class array> runtime::array(array::const_pointer elements, array::size_type size)
  {
    return new (*m_memory_manager) simple_array(size, elements);
  }

  /**
   * Word: length
   * Prototype: array
   *
   * Takes:
   * - array
   *
   * Gives:
   * - array
   * - number
   *
   * Returns number of elements in the array, while keeping the array in the
   * stack.
   */
  static void w_length(const ref<context>& ctx)
  {
    ref<array> ary;

    if (ctx->pop_array(ary))
    {
      ctx->push(ary);
      ctx->push_int(ary->size());
    }
  }

  /**
   * Word: includes?
   * Prototype: array
   *
   * Takes:
   * - any
   * - array
   *
   * Gives:
   * - array
   * - boolean
   *
   * Searches for given value from the array and returns true if it's included
   * in the array and false if it's not.
   */
  static void w_includes(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<value> val;

    if (ctx->pop_array(ary) && ctx->pop(val))
    {
      ctx->push(ary);
      for (array::size_type i = 0; i < ary->size(); ++i)
      {
        array::const_reference element = ary->at(i);

        if (val == element)
        {
          ctx->push_boolean(true);
          return;
        }
      }
      ctx->push_boolean(false);
    }
  }

  /**
   * Word: index-of
   * Prototype: array
   *
   * Takes:
   * - any
   * - array
   *
   * Gives:
   * - array
   * - number|null
   *
   * Searches for given value from the array and returns it's index in the array
   * if it's included in the array and null when it's not.
   */
  static void w_index_of(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<value> val;

    if (ctx->pop_array(ary) && ctx->pop(val))
    {
      const auto size = ary->size();

      ctx->push(ary);
      for (array::size_type i = 0; i < size; ++i)
      {
        if (val == ary->at(i))
        {
          ctx->push_int(i);
          return;
        }
      }
      ctx->push_null();
    }
  }

  /**
   * Word: find
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - array
   * -any|null
   *
   * Returns first element from the array that satisfies the provided testing
   * quote. Otherwise null is returned.
   */
  static void w_find(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<quote> quo;

    if (ctx->pop_array(ary) && ctx->pop_quote(quo))
    {
      ctx->push(ary);
      for (array::size_type i = 0; i < ary->size(); ++i)
      {
        const auto& element = ary->at(i);
        bool result;

        ctx->push(element);
        if (!quo->call(ctx) || !ctx->pop_boolean(result))
        {
          return;
        }
        else if (result)
        {
          ctx->push(element);
          return;
        }
      }
      ctx->push_null();
    }
  }

  /**
   * Word: find-index
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - array
   * - number|null
   *
   * Returns index of first element from the array that satisfies the provided
   * testing quote. Otherwise null is returned.
   */
  static void w_find_index(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<quote> quo;

    if (ctx->pop_array(ary) && ctx->pop_quote(quo))
    {
      const auto size = ary->size();

      ctx->push(ary);
      for (array::size_type i = 0; i < size; ++i)
      {
        bool result;

        ctx->push(ary->at(i));
        if (!quo->call(ctx) || !ctx->pop_boolean(result))
        {
          return;
        }
        else if (result)
        {
          ctx->push_int(i);
          return;
        }
      }
      ctx->push_null();
    }
  }

  /**
   * Word: every?
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - array
   * -boolean
   *
   * Tests whether all elements in the array passes the test implemented by the
   * provided quote.
   */
  static void w_every(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<quote> quo;

    if (ctx->pop_array(ary) && ctx->pop_quote(quo))
    {
      ctx->push(ary);
      for (array::size_type i = 0; i < ary->size(); ++i)
      {
        const auto& element = ary->at(i);
        bool result;

        ctx->push(element);
        if (!quo->call(ctx) || !ctx->pop_boolean(result))
        {
          return;
        }
        else if (!result)
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * Word: some?
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - array
   * - boolean
   *
   * Tests whether any element in the array passes the test implemented by the
   * provided quote.
   */
  static void w_some(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<quote> quo;

    if (ctx->pop_array(ary) && ctx->pop_quote(quo))
    {
      ctx->push(ary);
      for (array::size_type i = 0; i < ary->size(); ++i)
      {
        const auto& element = ary->at(i);
        bool result;

        ctx->push(element);
        if (!quo->call(ctx) || !ctx->pop_boolean(result))
        {
          return;
        }
        else if (result)
        {
          ctx->push_boolean(true);
          return;
        }
      }
      ctx->push_boolean(false);
    }
  }

  /**
   * Word: reverse
   * Prototype: array
   *
   * Takes:
   * - array
   *
   * Gives:
   * - array
   *
   * Returns reversed copy of the array where the first array element becomes
   * the last, and the last array element becomes first.
   */
  static void w_reverse(const ref<context>& ctx)
  {
    ref<array> ary;

    if (ctx->pop_array(ary))
    {
      const auto size = ary->size();
      ref<value> result[size];

      for (array::size_type i = ary->size(); i > 0; --i)
      {
        result[size - i] = ary->at(i - 1);
      }
      ctx->push_array(result, size);
    }
  }

  /**
   * Word: uniq
   * Prototype: array
   *
   * Takes:
   * - array
   *
   * Gives:
   * - array
   *
   * Creates a duplicate free version of the array.
   */
  static void w_uniq(const ref<context>& ctx)
  {
    ref<array> ary;

    if (ctx->pop_array(ary))
    {
      std::vector<ref<value>> result;

      for (array::size_type i = 0; i < ary->size(); ++i)
      {
        const auto& value1 = ary->at(i);
        bool found = false;

        for (const auto& value2 : result)
        {
          if (value1 == value2)
          {
            found = true;
            break;
          }
        }

        if (!found)
        {
          result.push_back(value1);
        }
      }

      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: extract
   * Prototype: array
   *
   * Takes:
   * - array
   *
   * Gives:
   * - any...
   *
   * Extracts all values from the array and places them into the stack.
   */
  static void w_extract(const ref<context>& ctx)
  {
    ref<array> ary;

    if (!ctx->pop_array(ary))
    {
      return;
    }
    for (array::size_type i = ary->size(); i > 0; --i)
    {
      ctx->push(ary->at(i - 1));
    }
  }

  /**
   * Word: join
   * Prototype: array
   *
   * Takes:
   * - string
   * - array
   *
   * Gives:
   * - string
   *
   * Concatenates all elements from the array into single string, delimited by
   * the given separator string.
   */
  static void w_join(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<string> separator;
    unistring result;

    if (!ctx->pop_array(ary) || !ctx->pop_string(separator))
    {
      return;
    }

    for (array::size_type i = 0; i < ary->size(); ++i)
    {
      const auto& element = ary->at(i);

      if (i > 0)
      {
        result += separator->to_string();
      }
      if (element)
      {
        result += element->to_string();
      } else {
        result += utf8_decode("null");
      }
    }

    ctx->push_string(result);
  }

  /**
   * Word: for-each
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Runs quote once for every element in the array.
   */
  static void w_for_each(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<quote> quo;

    if (!ctx->pop_array(ary) || !ctx->pop_quote(quo))
    {
      return;
    }

    for (array::size_type i = 0; i < ary->size(); ++i)
    {
      ctx->push(ary->at(i));
      if (!quo->call(ctx))
      {
        return;
      }
    }
  }

  /**
   * Word: map
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - array
   *
   * Applies quote once for each element in the array and constructs new array
   * from values returned by the quote.
   */
  static void w_map(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<quote> quo;

    if (ctx->pop_array(ary) && ctx->pop_quote(quo))
    {
      const auto size = ary->size();
      ref<value> result[size];

      for (array::size_type i = 0; i < size; ++i)
      {
        ref<value> quote_result;

        ctx->push(ary->at(i));
        if (!quo->call(ctx) || !ctx->pop(quote_result))
        {
          return;
        }
        result[i] = quote_result;
      }
      ctx->push_array(result, size);
    }
  }

  /**
   * Word: filter
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - array
   *
   * Applies quote once for each element in the array and constructs new array
   * from ones which passed the test.
   */
  static void w_filter(const ref<context>& ctx)
  {
    ref<class array> array;
    ref<class quote> quote;
    std::vector<ref<value>> result;

    if (!ctx->pop_array(array) || !ctx->pop_quote(quote))
    {
      return;
    }

    for (array::size_type i = 0; i < array->size(); ++i)
    {
      array::const_reference element = array->at(i);
      bool quote_result;

      ctx->push(element);
      if (!quote->call(ctx) || !ctx->pop_boolean(quote_result))
      {
        return;
      }
      else if (quote_result)
      {
        result.push_back(element);
      }
    }

    ctx->push_array(result.data(), result.size());
  }

  /**
   * Word: reduce
   * Prototype: array
   *
   * Takes:
   * - quote
   * - array
   *
   * Gives:
   * - any
   *
   * Applies given quote against an acculumator and each element in array to
   * reduce it into single value.
   */
  static void w_reduce(const ref<context>& ctx)
  {
    ref<class array> array;
    ref<class quote> quote;
    ref<value> result;
    array::size_type size;

    if (!ctx->pop_array(array) || !ctx->pop_quote(quote))
    {
      return;
    }

    size = array->size();

    if (size == 0)
    {
      ctx->error(error::code_range, "Cannot reduce empty array.");
      return;
    }

    result = array->at(0);

    for (array::size_type i = 1; i < size; ++i)
    {
      ctx->push(result);
      ctx->push(array->at(i));
      if (!quote->call(ctx) || !ctx->pop(result))
      {
        return;
      }
    }

    ctx->push(result);
  }

  /**
   * Word: +
   * Prototype: array
   *
   * Takes:
   * - array
   * - array
   *
   * Gives:
   * - array
   *
   * Concatenates contents of two arrays and returns result.
   */
  static void w_concat(const ref<context>& ctx)
  {
    ref<array> a;
    ref<array> b;

    if (ctx->pop_array(a) && ctx->pop_array(b))
    {
      ctx->push(ctx->runtime()->value<concat_array>(b, a));
    }
  }

  /**
   * Word: *
   * Prototype: array
   *
   * Takes:
   * - number
   * - array
   *
   * Gives:
   * - array
   *
   * Repeats array given number of times.
   */
  static void w_repeat(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<number> num;
    double count;

    if (ctx->pop_array(ary) && ctx->pop_number(num))
    {
      const auto size = ary->size();
      const std::int64_t count = num->as_int();

      if (count >= 0)
      {
        ref<value> result[size * count];

        for (std::int64_t i = 0; i < count; ++i)
        {
          for (array::size_type j = 0; j < size; ++j)
          {
            result[(i * size) + j] = ary->at(j);
          }
        }
        ctx->push_array(result, size * count);
      } else {
        ctx->error(error::code_range, "Invalid repeat count.");
      }
    }
  }

  /**
   * Word: &
   * Prototype: array
   *
   * Takes:
   * - array
   * - array
   *
   * Gives:
   * - array
   *
   * Set intersection: Returns new array containing unique elements common to
   * the two arrays.
   */
  static void w_intersect(const ref<context>& ctx)
  {
    ref<array> a;
    ref<array> b;

    if (ctx->pop_array(a) && ctx->pop_array(b))
    {
      std::vector<ref<value>> result;

      for (array::size_type i = 0; i < b->size(); ++i)
      {
        const auto& value1 = b->at(i);
        bool found = false;

        for (array::size_type j = 0; j < a->size(); ++j)
        {
          const auto& value2 = a->at(j);

          if (value1 == value2)
          {
            found = true;
            break;
          }
        }

        if (!found)
        {
          continue;
        }

        for (const auto& value2 : result)
        {
          if (value1 == value2)
          {
            found = false;
            break;
          }
        }

        if (found)
        {
          result.push_back(value1);
        }
      }

      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: |
   * Prototype: array
   *
   * Takes:
   * - array
   * - array
   *
   * Gives:
   * - array
   *
   * Set union: Returns new array by joining the two given arrays, excluding any
   * duplicates and preserving the order from the given arrays.
   */
  static void w_union(const ref<context>& ctx)
  {
    ref<array> a;
    ref<array> b;

    if (ctx->pop_array(a) && ctx->pop_array(b))
    {
      std::vector<ref<value>> result;

      for (array::size_type i = 0; i < b->size(); ++i)
      {
        const auto& value1 = b->at(i);
        bool found = false;

        for (const auto& value2 : result)
        {
          if (value1 == value2)
          {
            found = true;
            break;
          }
        }

        if (!found)
        {
          result.push_back(value1);
        }
      }

      for (array::size_type i = 0; i < a->size(); ++i)
      {
        const auto& value1 = a->at(i);
        bool found = false;

        for (const auto& value2 : result)
        {
          if (value1 == value2)
          {
            found = true;
            break;
          }
        }

        if (!found)
        {
          result.push_back(value1);
        }
      }

      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: @
   * Prototype: array
   *
   * Takes:
   * - number
   * - array
   *
   * Gives:
   * - array
   * - any
   *
   * Retrieves value from array at given numerical index. Negative indexes
   * count backwards. If given index is out of bounds, range error will be
   * thrown.
   */
  static void w_get(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<number> num;

    if (ctx->pop_array(ary) && ctx->pop_number(num))
    {
      std::int64_t index = num->as_int();

      if (index < 0)
      {
        index += ary->size();
      }

      ctx->push(ary);

      if (index < 0 || index > ary->size())
      {
        ctx->error(error::code_range, "Array index out of bounds.");
        return;
      }

      ctx->push(ary->at(index));
    }
  }

  /**
   * Word: !
   * Prototype: array
   *
   * Takes:
   * - any
   * - number
   * - array
   *
   * Gives:
   * - array
   *
   * Sets value in the array at given index. Negative indexes count backwrds.
   * If index is larger than number of elements in the array, value will be
   * appended as the last element of the array.
   */
  static void w_set(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<number> num;
    ref<value> val;

    if (ctx->pop_array(ary) && ctx->pop_number(num) && ctx->pop(val))
    {
      const auto size = ary->size();
      std::int64_t index = num->as_int();
      std::vector<ref<value>> result;

      if (index < 0)
      {
        index += size;
      }

      for (array::size_type i = 0;  i < size; ++i)
      {
        result.push_back(ary->at(i));
      }

      if (index < 0 || index > size)
      {
        result.push_back(val);
      } else {
        result[index] = val;
      }

      ctx->push_array(result.data(), result.size());
    }
  }

  namespace api
  {
    runtime::prototype_definition array_prototype()
    {
      return
      {
        { "length", w_length },

        // Search methods.
        { "includes?", w_includes },
        { "index-of", w_index_of },
        { "find", w_find },
        { "find-index", w_find_index },
        { "every?", w_every },
        { "some?", w_some },

        // Conversions.
        { "reverse", w_reverse },
        { "uniq", w_uniq },
        { "extract", w_extract },
        { "join", w_join },

        { "for-each", w_for_each },
        { "map", w_map },
        { "filter", w_filter },
        { "reduce", w_reduce },

        { "+", w_concat },
        { "*", w_repeat },
        { "&", w_intersect },
        { "|", w_union },
        { "@", w_get },
        { "!", w_set }
      };
    }
  }
}

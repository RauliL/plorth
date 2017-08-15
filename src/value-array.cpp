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
#include <plorth/value-string.hpp>

namespace plorth
{
  array::array(const container_type& elements)
    : m_elements(elements) {}

  bool array::equals(const ref<value>& that) const
  {
    ref<array> ary;

    if (!that->is(type_array))
    {
      return false;
    }

    ary = that.cast<array>();

    if (m_elements.size() != ary->m_elements.size())
    {
      return false;
    }

    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (!m_elements[i]->equals(ary->m_elements[i]))
      {
        return false;
      }
    }

    return true;
  }

  unistring array::to_string() const
  {
    const std::size_t size = m_elements.size();
    unistring result;

    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result += ',';
        result += ' ';
      }
      result += m_elements[i]->to_string();
    }

    return result;
  }

  unistring array::to_source() const
  {
    const std::size_t size = m_elements.size();
    unistring result;

    result += '[';
    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result += ',';
        result += ' ';
      }
      result += m_elements[i]->to_source();
    }
    result += ']';

    return result;
  }

  /**
   * length ( array -- array number )
   *
   * Returns number of elements in the array, while keeping the array in the
   * stack.
   */
  static void w_length(const ref<context>& ctx)
  {
    ref<class array> array;

    if (ctx->pop_array(array))
    {
      ctx->push(array);
      ctx->push_number(array->elements().size());
    }
  }

  /**
   * includes? ( any array -- array boolean )
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
      for (const auto& element : ary->elements())
      {
        if (val->equals(element))
        {
          ctx->push_boolean(true);
          return;
        }
      }
      ctx->push_boolean(false);
    }
  }

  /**
   * index-of ( any array -- array number|null )
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
      const auto& elements = ary->elements();
      const auto size = elements.size();

      ctx->push(ary);
      for (std::size_t i = 0; i < size; ++i)
      {
        if (val->equals(elements[i]))
        {
          ctx->push_number(i);
          return;
        }
      }
      ctx->push_null();
    }
  }

  /**
   * find ( quote array -- array any|null )
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
      for (const auto& element : ary->elements())
      {
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
   * find-index ( quote array -- array number|null )
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
      const auto& elements = ary->elements();
      const auto size = elements.size();

      ctx->push(ary);
      for (std::size_t i = 0; i < size; ++i)
      {
        bool result;

        ctx->push(elements[i]);
        if (!quo->call(ctx) || !ctx->pop_boolean(result))
        {
          return;
        }
        else if (result)
        {
          ctx->push_number(i);
          return;
        }
      }
      ctx->push_null();
    }
  }

  /**
   * every? ( quote array -- array boolean )
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
      for (const auto& element : ary->elements())
      {
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
   * some? ( quote array -- array boolean )
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
      for (const auto& element : ary->elements())
      {
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
   * reverse ( array -- array )
   *
   * Returns reversed copy of the array where the first array element becomes
   * the last, and the last array element becomes first.
   */
  static void w_reverse(const ref<context>& ctx)
  {
    ref<class array> array;

    if (ctx->pop_array(array))
    {
      const array::container_type& elements = array->elements();

      ctx->push_array(array::container_type(elements.rbegin(), elements.rend()));
    }
  }

  /**
   * uniq ( array -- array )
   *
   * Creates a duplicate free version of the array.
   */
  static void w_uniq(const ref<context>& ctx)
  {
    ref<array> ary;

    if (ctx->pop_array(ary))
    {
      array::container_type result;

      for (const auto& value1 : ary->elements())
      {
        bool found = false;

        for (const auto& value2 : result)
        {
          if (value1->equals(value2))
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

      ctx->push_array(result);
    }
  }

  /**
   * extract ( array -- any... )
   *
   * Extracts all values from the array and places them into the stack.
   */
  static void w_extract(const ref<context>& ctx)
  {
    ref<class array> array;

    if (ctx->pop_array(array))
    {
      const auto& elements = array->elements();
      auto it = elements.rbegin();
      const auto end = elements.rend();

      while (it != end)
      {
        ctx->push(*it++);
      }
    }
  }

  /**
   * join ( string array -- string )
   *
   * Concatenates all elements from the array into single string, delimited by
   * the given separator string.
   */
  static void w_join(const ref<context>& ctx)
  {
    ref<array> ary;
    ref<string> separator;
    unistring result;
    bool first = true;

    if (!ctx->pop_array(ary) || !ctx->pop_string(separator))
    {
      return;
    }

    for (const auto& element : ary->elements())
    {
      if (first)
      {
        first = false;
      } else {
        result += separator->value();
      }
      result += element->to_string();
    }

    ctx->push_string(result);
  }

  /**
   * for-each ( quote array -- )
   *
   * Runs quote once for every element in the array.
   */
  static void w_for_each(const ref<context>& ctx)
  {
    ref<class array> array;
    ref<class quote> quote;

    if (!ctx->pop_array(array) || !ctx->pop_quote(quote))
    {
      return;
    }

    for (const auto& element : array->elements())
    {
      ctx->push(element);
      if (!quote->call(ctx))
      {
        return;
      }
    }
  }

  /**
   * map ( quote array -- array )
   *
   * Applies quote once for each element in the array and constructs new array
   * from values returned by the quote.
   */
  static void w_map(const ref<context>& ctx)
  {
    ref<class array> array;
    ref<class quote> quote;
    std::vector<ref<value>> result;

    if (!ctx->pop_array(array) || !ctx->pop_quote(quote))
    {
      return;
    }

    result.reserve(array->elements().size());

    for (const auto& element : array->elements())
    {
      ref<value> quote_result;

      ctx->push(element);
      if (!quote->call(ctx) || !ctx->pop(quote_result))
      {
        return;
      }
      result.push_back(quote_result);
    }

    ctx->push_array(result);
  }

  /**
   * filter ( quote array -- array )
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

    for (const auto& element : array->elements())
    {
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

    ctx->push_array(result);
  }

  /**
   * reduce ( quote array -- any )
   *
   * Applies given quote against an acculumator and each element in array to
   * reduce it into single value.
   */
  static void w_reduce(const ref<context>& ctx)
  {
    ref<class array> array;
    ref<class quote> quote;
    ref<value> result;
    std::size_t size;

    if (!ctx->pop_array(array) || !ctx->pop_quote(quote))
    {
      return;
    }

    size = array->elements().size();

    if (size == 0)
    {
      ctx->error(error::code_range, "Cannot reduce empty array.");
      return;
    }

    result = array->elements().front();

    for (std::size_t i = 1; i < size; ++i)
    {
      ctx->push(result);
      ctx->push(array->elements()[i]);
      if (!quote->call(ctx) || !ctx->pop(result))
      {
        return;
      }
    }

    ctx->push(result);
  }

  /**
   * + ( array array -- array )
   *
   * Concatenates contents of two arrays and returns result.
   */
  static void w_concat(const ref<context>& ctx)
  {
    ref<array> a;
    ref<array> b;

    if (ctx->pop_array(a) && ctx->pop_array(b))
    {
      const array::container_type& x = a->elements();
      const array::container_type& y = b->elements();
      array::container_type result;

      result.reserve(x.size() + y.size());
      result.insert(result.begin(), std::begin(x), std::end(x));
      result.insert(result.begin(), std::begin(y), std::end(y));
      ctx->push_array(result);
    }
  }

  /**
   * * ( number array -- array )
   *
   * Repeats array given number of times.
   */
  static void w_repeat(const ref<context>& ctx)
  {
    ref<array> ary;
    double count;

    if (ctx->pop_array(ary) && ctx->pop_number(count))
    {
      const auto& elements = ary->elements();
      array::container_type result;

      if (count < 0.0)
      {
        count = -count;
      }

      result.reserve(elements.size() * count);

      while (count >= 1.0)
      {
        count -= 1.0;
        result.insert(result.end(), std::begin(elements), std::end(elements));
      }

      ctx->push_array(result);
    }
  }

  /**
   * & ( array array -- array )
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
      array::container_type result;

      for (const auto& value1 : b->elements())
      {
        bool found = false;

        for (const auto& value2 : a->elements())
        {
          if (value1->equals(value2))
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
          if (value1->equals(value2))
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

      ctx->push_array(result);
    }
  }

  /**
   * | ( array array -- array )
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
      array::container_type result;

      for (const auto& value1 : b->elements())
      {
        bool found = false;

        for (const auto& value2 : result)
        {
          if (value1->equals(value2))
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

      for (const auto& value1 : a->elements())
      {
        bool found = false;

        for (const auto& value2 : result)
        {
          if (value1->equals(value2))
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

      ctx->push_array(result);
    }
  }

  /**
   * @ ( number array -- array any )
   *
   * Retrieves value from array at given numerical index. Negative indexes
   * count backwards. If given index is out of bounds, range error will be
   * thrown.
   */
  static void w_get(const ref<context>& ctx)
  {
    ref<array> ary;
    double index;

    if (ctx->pop_array(ary) && ctx->pop_number(index))
    {
      const array::container_type& elements = ary->elements();

      if (index < 0.0)
      {
        index += elements.size();
      }

      ctx->push(ary);

      if (index < 0.0 || index > elements.size())
      {
        ctx->error(error::code_range, "Array index out of bounds.");
        return;
      }

      ctx->push(elements[index]);
    }
  }

  /**
   * ! ( any number array -- array )
   *
   * Sets value in the array at given index. Negative indexes count backwrds.
   * If index is larger than number of elements in the array, value will be
   * appended as the last element of the array.
   */
  static void w_set(const ref<context>& ctx)
  {
    ref<array> ary;
    double index;
    ref<value> val;

    if (ctx->pop_array(ary) && ctx->pop_number(index) && ctx->pop(val))
    {
      array::container_type result = ary->elements();

      if (index < 0.0)
      {
        index += result.size();
      }

      if (index < 0.0 || index > result.size())
      {
        result.push_back(val);
      } else {
        result[index] = val;
      }

      ctx->push_array(result);
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

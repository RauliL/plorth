#include "array.hpp"
#include "state.hpp"
#include "string.hpp"

namespace plorth
{
  Array::Array(const std::vector<Ref<Value>>& elements)
    : m_elements(elements) {}

  Ref<Object> Array::GetPrototype(const Ref<Runtime>& runtime) const
  {
    Ref<Value> value;

    if (runtime->FindWord("ary", value) && value->GetType() == TYPE_OBJECT)
    {
      return value.As<Object>();
    } else {
      return Ref<Object>();
    }
  }

  bool Array::Equals(const Ref<Value>& that) const
  {
    Ref<Array> other;

    if (that->GetType() != TYPE_ARRAY)
    {
      return false;
    }
    other = that.As<Array>();
    if (m_elements.size() != other->m_elements.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (!m_elements[i]->Equals(other->m_elements[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::string Array::ToString() const
  {
    std::string result;

    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += ", ";
      }
      result += m_elements[i]->ToString();
    }

    return result;
  }

  std::string Array::ToSource() const
  {
    std::string result;

    result += "[";
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += ", ";
      }
      result += m_elements[i]->ToSource();
    }
    result += "]";

    return result;
  }

  /**
   * ary? ( any -- any bool )
   *
   * Returns true if given value is array.
   */
  static void w_is_ary(const Ref<State>& state)
  {
    Ref<Value> value;

    if (state->Peek(value))
    {
      state->PushBool(value->GetType() == Value::TYPE_ARRAY);
    }
  }

  /**
   * len ( ary -- ary num )
   *
   * Returns number of elements in the array.
   */
  static void w_len(const Ref<State>& state)
  {
    Ref<Array> array;

    if (state->PeekArray(array))
    {
      state->PushNumber(static_cast<std::int64_t>(array->GetSize()));
    }
  }

  /**
   * empty? ( ary -- ary bool )
   *
   * Returns true if the array is empty.
   */
  static void w_is_empty(const Ref<State>& state)
  {
    Ref<Array> array;

    if (state->PeekArray(array))
    {
      state->PushBool(array->IsEmpty());
    }
  }

  /**
   * every? ( quote ary -- bool )
   *
   * Tests whether all elements in the array passes the test implemented by the
   * provided quote.
   */
  static void w_every(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Quote> quote;

    if (!state->PopArray(array) || !state->PopQuote(quote))
    {
      return;
    }
    for (std::size_t i = 0; i < array->GetSize(); ++i)
    {
      Ref<Bool> result;

      state->Push(array->Get(i));
      if (!quote->Call(state) || !state->PopBool(result))
      {
        return;
      }
      else if (!result->GetValue())
      {
        state->PushBool(false);
        return;
      }
    }
    state->PushBool(true);
  }

  /**
   * some? ( quote ary -- bool )
   *
   * Tests whether any element in the array passes the test implemented by the
   * provided quote.
   */
  static void w_some(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Quote> quote;

    if (!state->PopArray(array) || !state->PopQuote(quote))
    {
      return;
    }
    for (std::size_t i = 0; i < array->GetSize(); ++i)
    {
      Ref<Bool> result;

      state->Push(array->Get(i));
      if (!quote->Call(state) || !state->PopBool(result))
      {
        return;
      }
      else if (result->GetValue())
      {
        state->PushBool(true);
        return;
      }
    }
    state->PushBool(false);
  }

  /**
   * index-of ( any ary -- num|null )
   *
   * Attempts to find given value from the array and returns index of it, if it
   * exists in the array, otherwise null.
   */
  static void w_index_of(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Value> value;

    if (!state->PopArray(array) || !state->Pop(value))
    {
      return;
    }
    for (std::size_t i = 0; i < array->GetSize(); ++i)
    {
      if (array->Get(i)->Equals(value))
      {
        state->PushNumber(static_cast<std::int64_t>(i));
        return;
      }
    }
    state->PushNull();
  }

  /**
   * join ( str ary -- str )
   *
   * Concatenates all elements from the array into single string, delimited by
   * the given separator string.
   */
  static void w_join(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<String> separator;

    if (state->PopArray(array) && state->PopString(separator))
    {
      std::string result;
      bool first = true;

      for (auto& element : array->GetElements())
      {
        if (first)
        {
          first = false;
        } else {
          result += separator->GetValue();
        }
        result += element->ToString();
      }
      state->PushString(result);
    }
  }

  /**
   * for-each ( quote ary -- )
   *
   * Runs quote once for every element in the array.
   */
  static void w_for_each(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Quote> quote;

    if (state->PopArray(array) && state->PopQuote(quote))
    {
      const std::size_t size = array->GetSize();

      for (std::size_t i = 0; i < size; ++i)
      {
        state->Push(array->Get(i));
        if (!quote->Call(state))
        {
          return;
        }
      }
    }
  }

  /**
   * filter ( quote ary -- ary )
   *
   * Applies quote once for each element in the array and constructs new array
   * from ones which passed the test.
   */
  static void w_filter(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Quote> quote;
    std::vector<Ref<Value>> result;

    if (!state->PopArray(array) || !state->PopQuote(quote))
    {
      return;
    }
    for (std::size_t i = 0; i < array->GetSize(); ++i)
    {
      Ref<Bool> quote_result;

      state->Push(array->Get(i));
      if (!quote->Call(state) || !state->PopBool(quote_result))
      {
        return;
      }
      else if (quote_result->GetValue())
      {
        result.push_back(array->Get(i));
      }
    }
    state->PushArray(result);
  }

  /**
   * map ( quote ary -- ary )
   *
   * Applies quote once for each element in the array and constructs new array
   * from values returned by the quote.
   */
  static void w_map(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Quote> quote;
    std::vector<Ref<Value>> result;

    if (!state->PopArray(array) || !state->PopQuote(quote))
    {
      return;
    }
    for (std::size_t i = 0; i < array->GetSize(); ++i)
    {
      Ref<Value> quote_result;

      state->Push(array->Get(i));
      if (!quote->Call(state) || !state->Pop(quote_result))
      {
        return;
      }
      result.push_back(quote_result);
    }
    state->PushArray(result);
  }

  /**
   * find ( quote ary -- num|null )
   *
   * Returns index of the element in the array which passes test implemented by
   * the given quote, or null if no such element exists in the array.
   */
  static void w_find(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Quote> quote;

    if (!state->PopArray(array) || !state->PopQuote(quote))
    {
      return;
    }
    for (std::size_t i = 0; i < array->GetSize(); ++i)
    {
      Ref<Bool> result;

      state->Push(array->Get(i));
      if (!quote->Call(state) || !state->PopBool(result))
      {
        return;
      }
      else if (result->GetValue())
      {
        state->PushNumber(static_cast<std::int64_t>(i));
        return;
      }
    }
    state->PushNull();
  }

  /**
   * reverse ( ary -- ary )
   *
   * Returns copy of the array in reversed order.
   */
  static void w_reverse(const Ref<State>& state)
  {
    Ref<Array> array;

    if (state->PopArray(array))
    {
      const std::vector<Ref<Value>>& elements = array->GetElements();

      state->PushArray(
        std::vector<Ref<Value>>(
          elements.rbegin(),
          elements.rend()
        )
      );
    }
  }

  /**
   * extract ( ary -- any... )
   *
   * Extracts all values from the array and pushes them to the stack.
   */
  static void w_extract(const Ref<State>& state)
  {
    Ref<Array> array;

    if (state->PopArray(array))
    {
      const std::vector<Ref<Value>>& elements = array->GetElements();

      for (std::size_t i = 0; i < elements.size(); ++i)
      {
        state->Push(elements[i]);
      }
    }
  }

  /**
   * @ ( num ary -- any )
   *
   * Retrieves value from array at given index. Negative indexes count
   * backwards.
   */
  static void w_get(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Number> index;

    if (state->PopArray(array) && state->PopNumber(index))
    {
      const std::vector<Ref<Value>>& elements = array->GetElements();
      std::int64_t i = index->AsInt();

      if (i < 0)
      {
        i += elements.size();
      }
      if (i < 0 || static_cast<std::size_t>(i) >= elements.size())
      {
        state->SetError(Error::ERROR_CODE_RANGE, "Array index out of bounds.");
        return;
      }
      state->Push(elements[i]);
    }
  }

  /**
   * ! ( any num ary -- any )
   *
   * Sets value in the array at given index. Negative indexes count backwrds.
   * If index is larger than number of elements in the array, value will be
   * appended as the last element of the array.
   */
  static void w_set(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Number> index;
    Ref<Value> value;

    if (state->PopArray(array) && state->PopNumber(index) && state->Pop(value))
    {
      std::vector<Ref<Value>> elements = array->GetElements();
      std::int64_t i = index->AsInt();

      if (i < 0)
      {
        i += elements.size();
      }
      if (i < 0 || static_cast<std::size_t>(i) >= elements.size())
      {
        elements.push_back(value);
      } else {
        elements[i] = value;
      }
      state->PushArray(elements);
    }
  }

  /**
   * + ( ary ary -- ary )
   *
   * Combines contents of two arrays together.
   */
  static void w_plus(const Ref<State>& state)
  {
    Ref<Array> array_a;
    Ref<Array> array_b;

    if (state->PopArray(array_a) && state->PopArray(array_b))
    {
      const std::vector<Ref<Value>>& a = array_a->GetElements();
      const std::vector<Ref<Value>>& b = array_b->GetElements();
      std::vector<Ref<Value>> result;

      result.reserve(a.size() + b.size());
      result.insert(end(result), begin(b), end(b));
      result.insert(end(result), begin(a), end(a));
      state->PushArray(result);
    }
  }

  /**
   * * ( num ary -- ary )
   *
   * Repeats array given number of times.
   */
  static void w_times(const Ref<State>& state)
  {
    Ref<Array> array;
    Ref<Number> number;
    std::vector<Ref<Value>> result;

    if (!state->PopArray(array) || !state->PopNumber(number))
    {
      return;
    }
    else if (number->GetNumberType() == Number::NUMBER_TYPE_INT)
    {
      std::int64_t times = number.As<IntNumber>()->GetValue();

      result.reserve(array->GetElements().size() * times);
      for (std::int64_t i = 0; i < times; ++i)
      {
        result.insert(
          end(result),
          begin(array->GetElements()),
          end(array->GetElements())
        );
      }
    } else {
      const mpz_class times = number->AsBigInt();

      for (mpz_class i = 0; i < times; ++i)
      {
        result.insert(
          end(result),
          begin(array->GetElements()),
          end(array->GetElements())
        );
      }
    }
    state->PushArray(result);
  }

  void api_init_array(Runtime* runtime)
  {
    runtime->AddWord("ary?", w_is_ary);

    runtime->AddNamespace("ary", {
      { "len", w_len },

      { "empty?", w_is_empty },
      { "every?", w_every },
      { "some?", w_some },

      { "index-of", w_index_of },
      { "join", w_join },

      { "for-each", w_for_each },
      { "filter", w_filter },
      { "map", w_map },
      { "find", w_find },

      { "reverse", w_reverse },
      { "extract", w_extract },

      { "@", w_get },
      { "!", w_set },

      { "+", w_plus },
      { "*", w_times },
    });
  }
}

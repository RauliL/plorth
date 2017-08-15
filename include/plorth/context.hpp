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
#ifndef PLORTH_CONTEXT_HPP_GUARD
#define PLORTH_CONTEXT_HPP_GUARD

#include <plorth/runtime.hpp>
#include <plorth/value-array.hpp>
#include <plorth/value-error.hpp>

#include <deque>

namespace plorth
{
  /**
   * Represents program execution state.
   */
  class context : public memory::managed
  {
  public:
    using container_type = std::deque<ref<value>>;

    /**
     * Constructs new context.
     *
     * \param runtime Runtime associated with this context.
     */
    explicit context(const ref<class runtime>& runtime);

    /**
     * Returns the runtime associated with this context.
     */
    inline const ref<class runtime>& runtime() const
    {
      return m_runtime;
    }

    /**
     * Returns the currently uncaught error in this context or null reference
     * if this context has no error.
     */
    inline const ref<class error>& error() const
    {
      return m_error;
    }

    /**
     * Sets the current uncaught error to one given as argument.
     *
     * \param error Error instance to set as the currently uncaught error.
     */
    inline void error(const ref<class error>& error)
    {
      m_error = error;
    }

    /**
     * Constructs new error instance with given error code and error message
     * and replaces this execution state's currently uncaught error with it.
     */
    void error(enum error::code code, const unistring& message);

    /**
     * Constructs new error instance with given error code and error message
     * and replaces this execution state's currently uncaught error with it.
     *
     * The error message is expected to be encoded in UTF-8 character encoding.
     */
    inline void error(enum error::code code, const char* message)
    {
      error(code, utf8_decode(message));
    }

    /**
     * Removes currently uncaught error in the context.
     */
    inline void clear_error()
    {
      m_error.release();
    }

    /**
     * Returns the dictionary used by this context to store words.
     */
    inline const object::container_type& dictionary() const
    {
      return m_dictionary;
    }

    /**
     * Executes a word within this context.
     *
     * \param word Name of the word to execute.
     * \return     A boolean flag which tells whether the execution was
     *             successfull or whether an error was encountered.
     */
    bool call(const unistring& word);

    /**
     * Inserts given quote into the context dictionary with given word as it's
     * name.
     *
     * \param word  Name of the word to define.
     * \param quote Quote which is executed when the word is called.
     */
    void declare(const unistring& word, const ref<class quote>& quote);

    /**
     * Compiles given source code into a quote.
     *
     * \param source Source code to compile into quote.
     * \return       Reference the quote that was compiled from given source,
     *               or null reference if syntax error was encountered.
     */
    ref<quote> compile(const std::string& source);

    /**
     * Provides direct access to the data stack.
     */
    inline container_type& data()
    {
      return m_data;
    }

    /**
     * Provides direct access to the data stack.
     */
    inline const container_type& data() const
    {
      return m_data;
    }

    /**
     * Returns true if the data stack is empty.
     */
    inline bool empty() const
    {
      return m_data.empty();
    }

    /**
     * Returns the number of values currently in the data stack.
     */
    inline std::size_t size() const
    {
      return m_data.size();
    }

    /**
     * Removes all values from the data stack.
     */
    inline void clear()
    {
      m_data.clear();
    }

    /**
     * Pushes given value into the data stack.
     */
    inline void push(const ref<class value>& value)
    {
      m_data.push_back(value);
    }

    /**
     * Pushes null value into the data stack.
     */
    void push_null();

    /**
     * Pushes boolean value into the data stack.
     */
    void push_boolean(bool value);

    /**
     * Pushes number value into the data stack.
     */
    void push_number(double value);

    /**
     * Pushes string value into the data stack.
     */
    void push_string(const unistring& value);

    /**
     * Constructs array from given sequence of values and pushes it into the
     * data stack.
     */
    void push_array(const array::container_type& elements);

    /**
     * Constructs object from given properties and pushes it into the data
     * stack.
     */
    void push_object(const object::container_type& properties);

    /**
     * Pops value from the data stack and discards it. If the stack is empty,
     * range error will be set.
     *
     * \return Boolean flag that tells whether the operation was successfull or
     *         not.
     */
    bool pop();

    /**
     * Pops value of certain type from the data stack and discards it. If the
     * stack is empty, range error will be set. If the top value of the stack
     * is different type than expected, type error will be set.
     *
     * \param type Value type to be expected to be at the top of the stack.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop(enum value::type type);

    /**
     * Pops value from the data stack and places it into given reference slot.
     * If the stack is empty, range error will be set.
     *
     * \param slot Reference where the value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop(ref<value>& slot);

    /**
     * Pops value of certain type from the data stack and places it into given
     * reference slot. If the stack is empty, range error will be set. If the
     * top value of the stack is different type than expected, type error will
     * be set.
     *
     * \param slot Reference where the value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop(ref<value>& slot, enum value::type type);

    /**
     * Pops boolean value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * boolean is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the boolean value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_boolean(bool& slot);

    /**
     * Pops number value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * number is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the number value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_number(double& slot);

    /**
     * Pops string value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * string is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the string value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_string(ref<string>& slot);

    /**
     * Pops array value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * array is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the array value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_array(ref<array>& slot);

    /**
     * Pops object from the data stack and places it into given slot. If the
     * stack is empty, range error will be set. If something else than object
     * is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the object will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_object(ref<object>& slot);

    /**
     * Pops quote from the data stack and places it into given slot. If the
     * stack is empty, range error will be set. If something else than quote
     * is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the quote will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_quote(ref<quote>& slot);

  private:
    /** Runtime associated with this context. */
    const ref<class runtime> m_runtime;
    /** Currently uncaught error in this context. */
    ref<class error> m_error;
    /** Data stack used for storing values in this context. */
    container_type m_data;
    /** Container for words associated with this context. */
    object::container_type m_dictionary;
  };
}

#endif /* !PLORTH_CONTEXT_HPP_GUARD */

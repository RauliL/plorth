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
#ifndef PLORTH_RUNTIME_HPP_GUARD
#define PLORTH_RUNTIME_HPP_GUARD

#include <plorth/dictionary.hpp>
#include <plorth/io-input.hpp>
#include <plorth/io-output.hpp>
#include <plorth/value-array.hpp>
#include <plorth/value-boolean.hpp>
#include <plorth/value-number.hpp>
#include <plorth/value-object.hpp>
#include <plorth/value-string.hpp>

namespace plorth
{
  class runtime : public memory::managed
  {
  public:
    using prototype_definition = std::vector<
      std::pair<const char32_t*, quote::callback>
    >;
#if PLORTH_ENABLE_SYMBOL_CACHE
    using symbol_cache = std::unordered_map<
      unistring,
      std::shared_ptr<class symbol>
    >;
#endif

    /**
     * Constructs new runtime.
     *
     * \param memory_manager Memory manager to use for allocating memory.
     * \param input          Input used by the runtime. If omitted, standard
     *                       input stream of the process will be used.
     * \param output         Output used by the runtime. If omitted, standard
     *                       output stream of the process will be used.
     * \return               Reference to the created runtime.
     */
    static std::shared_ptr<runtime> make(
      memory::manager& memory_manager,
      const std::shared_ptr<io::input>& input = std::shared_ptr<io::input>(),
      const std::shared_ptr<io::output>& output = std::shared_ptr<io::output>()
    );

    /**
     * Returns the memory manager used by this scripting runtime.
     */
    inline memory::manager& memory_manager() const
    {
      return *m_memory_manager;
    }

    /**
     * Returns the input used by the runtime.
     */
    inline std::shared_ptr<io::input>& input()
    {
      return m_input;
    }

    /**
     * Returns the input used by the runtime.
     */
    inline const std::shared_ptr<io::input>& input() const
    {
      return m_input;
    }

    /**
     * Returns the output used by the runtime.
     */
    inline std::shared_ptr<io::output>& output()
    {
      return m_output;
    }

    /**
     * Returns the output used by the runtime.
     */
    inline const std::shared_ptr<io::output>& output() const
    {
      return m_output;
    }

    /**
     * Returns the global dictionary that contains core word set available to
     * all contexts.
     *
     * This non-constant version of the method can be used to define new words
     * into the global dictionary, or remove existing ones.
     */
    inline class dictionary& dictionary()
    {
      return m_dictionary;
    }

    /**
     * Returns the global dictionary that contains core word set available to
     * all contexts.
     */
    inline const class dictionary& dictionary() const
    {
      return m_dictionary;
    }

    /**
     * Returns container for command line arguments provided for the
     * interpreter.
     */
    inline std::vector<unistring>& arguments()
    {
      return m_arguments;
    }

    /**
     * Returns container for command line arguments provided for the
     * interpreter.
     */
    inline const std::vector<unistring>& arguments() const
    {
      return m_arguments;
    }

    /**
     * Returns container for file system paths where modules are searched from.
     */
    inline std::vector<unistring>& module_paths()
    {
      return m_module_paths;
    }

    /**
     * Returns container for file system paths where modules are searched from.
     */
    inline const std::vector<unistring>& module_paths() const
    {
      return m_module_paths;
    }

    /**
     * Returns the container which the runtime uses to cache imported modules.
     */
    inline object::container_type& imported_modules()
    {
      return m_imported_modules;
    }

    /**
     * Returns the container which the runtime uses to cache imported modules.
     */
    inline const object::container_type& imported_modules() const
    {
      return m_imported_modules;
    }

    /**
     * Reads Unicode code points from the input of the interpreter and places
     * them in the string given as argument.
     *
     * \param size   Number of Unicode characters to be read. If zero is given,
     *               input will be consumed until there is no more input to be
     *               read.
     * \param output Where the read Unicode characters will be placed into.
     * \param read   Where the amount of read Unicode characters will be placed
     *               into.
     */
    io::input::result read(
      io::input::size_type size,
      unistring& output,
      io::input::size_type& read
    );

    /**
     * Outputs given Unicode string into the output of the interpreter.
     */
    void print(const unistring& str) const;

    /**
     * Outputs system specific new line into the output of the interpreter.
     */
    void println() const;

    /**
     * Outputs given Unicode string and system specific new line into the
     * output of the interpreter.
     */
    void println(const unistring& str) const;

    /**
     * Constructs integer number from given value.
     *
     * \param value Value of the number.
     * \return      Reference to the created number value.
     */
    std::shared_ptr<class number> number(number::int_type value);

    /**
     * Constructs real number from given value.
     *
     * \param value Value of the number.
     * \return      Reference to the created number value.
     */
    std::shared_ptr<class number> number(number::real_type value);

    /**
     * Parses given text input into number (either real or integer) and
     * constructs number value from it.
     *
     * \param value Value of the number as text.
     * \return      Reference to the created number value.
     */
    std::shared_ptr<class number> number(const unistring& value);

    /**
     * Constructs array value from given elements.
     *
     * \param elements Array of elements to construct array from.
     * \param size     Number of elements in the array.
     * \return         Reference to the created array value.
     */
    std::shared_ptr<class array> array(array::const_pointer elements,
                                       array::size_type size);

    /**
     * Constructs string value from given Unicode string.
     *
     * \param input Unicode string to construct string value from.
     * \return      Reference to the created string value.
     */
    std::shared_ptr<class string> string(const unistring& input);

    /**
     * Constructs string value from given pointer of Unicode code points.
     *
     * \param chars  Array of Unicode characters to construct the string from.
     * \param length Number of characters in the string.
     * \return       Reference to the created string value.
     */
    std::shared_ptr<class string> string(string::const_pointer chars,
                                         string::size_type length);

    /**
     * Constructs symbol from given identifier string.
     *
     * \param id       String which acts as identifier for the symbol.
     * \param position Optional position in source code where the symbol was
     *                 encountered.
     * \return         Reference to the created symbol.
     */
    std::shared_ptr<class symbol> symbol(
      const unistring& id,
      const struct position* position = nullptr
    );

    /**
     * Constructs compiled quote from given sequence of values.
     */
    std::shared_ptr<quote> compiled_quote(
      const std::vector<std::shared_ptr<value>>& values
    );

    /**
     * Constructs native quote from given C++ callback.
     */
    std::shared_ptr<quote> native_quote(quote::callback callback);

    /**
     * Constructs word from given string and quote.
     */
    std::shared_ptr<class word> word(
      const unistring& id,
      const std::shared_ptr<class quote>& quote
    );

    /**
     * Constructs word from given symbol and quote.
     */
    std::shared_ptr<class word> word(
      const std::shared_ptr<class symbol>& symbol,
      const std::shared_ptr<class quote>& quote
    );

    /**
     * Helper method for constructing managed objects (such as values) using
     * the memory manager associated with this runtime instance.
     */
    template< typename T, typename... Args >
    inline std::shared_ptr<T> value(Args... args)
    {
      return std::shared_ptr<T>(new (*m_memory_manager) T(args...));
    }

    /**
     * Returns shared instance of true boolean value.
     */
    inline const std::shared_ptr<class boolean>& true_value() const
    {
      return m_true_value;
    }

    /**
     * Returns shared instance of false boolean value.
     */
    inline const std::shared_ptr<class boolean>& false_value() const
    {
      return m_false_value;
    }

    /**
     * Helper method for converting C++ boolean value into Plorth boolean
     * value.
     */
    inline const std::shared_ptr<class boolean>& boolean(bool b) const
    {
      return b ? m_true_value : m_false_value;
    }

    /**
     * Returns prototype for array values.
     */
    inline const std::shared_ptr<object>& array_prototype() const
    {
      return m_array_prototype;
    }

    /**
     * Returns prototype for boolean values.
     */
    inline const std::shared_ptr<object>& boolean_prototype() const
    {
      return m_boolean_prototype;
    }

    /**
     * Returns prototype for error values.
     */
    inline const std::shared_ptr<object>& error_prototype() const
    {
      return m_error_prototype;
    }

    /**
     * Returns prototype for number values.
     */
    inline const std::shared_ptr<object>& number_prototype() const
    {
      return m_number_prototype;
    }

    /**
     * Returns prototype for objects.
     */
    inline const std::shared_ptr<object>& object_prototype() const
    {
      return m_object_prototype;
    }

    /**
     * Returns prototype for quotes.
     */
    inline const std::shared_ptr<object>& quote_prototype() const
    {
      return m_quote_prototype;
    }

    /**
     * Returns prototype for string values.
     */
    inline const std::shared_ptr<object>& string_prototype() const
    {
      return m_string_prototype;
    }

    /**
     * Returns prototype for symbols.
     */
    inline const std::shared_ptr<object>& symbol_prototype() const
    {
      return m_symbol_prototype;
    }

    /**
     * Returns prototype for words.
     */
    inline const std::shared_ptr<object>& word_prototype() const
    {
      return m_word_prototype;
    }

  protected:
    /**
     * Constructs new runtime.
     *
     * \param memory_manager Pointer to the memory manager to use for
     *                       allocating memory.
     */
    explicit runtime(memory::manager* memory_manager);

  private:
    /** Memory manager associated with this runtime. */
    memory::manager* m_memory_manager;
    /** Input which the runtime uses. */
    std::shared_ptr<io::input> m_input;
    /** Output which the runtime uses. */
    std::shared_ptr<io::output> m_output;
    /** Global dictionary available to all contexts. */
    class dictionary m_dictionary;
    /** Shared instance of true boolean value. */
    std::shared_ptr<class boolean> m_true_value;
    /** Shared instance of false boolean value. */
    std::shared_ptr<class boolean> m_false_value;
    /** Prototype for array values. */
    std::shared_ptr<object> m_array_prototype;
    /** Prototype for boolean values. */
    std::shared_ptr<object> m_boolean_prototype;
    /** Prototype for error values. */
    std::shared_ptr<object> m_error_prototype;
    /** Prototype for number values. */
    std::shared_ptr<object> m_number_prototype;
    /** Prototype for objects. */
    std::shared_ptr<object> m_object_prototype;
    /** Prototype for quotes. */
    std::shared_ptr<object> m_quote_prototype;
    /** Prototype for string values. */
    std::shared_ptr<object> m_string_prototype;
    /** Prototype for symbol values. */
    std::shared_ptr<object> m_symbol_prototype;
    /** Prototype for words. */
    std::shared_ptr<object> m_word_prototype;
    /** List of command line arguments given for the interpreter. */
    std::vector<unistring> m_arguments;
    /** List of file system paths where to look modules from. */
    std::vector<unistring> m_module_paths;
    /** Container for already imported modules. */
    object::container_type m_imported_modules;
#if PLORTH_ENABLE_SYMBOL_CACHE
    /** Cache for symbols used by the runtime. */
    symbol_cache m_symbol_cache;
#endif
#if PLORTH_ENABLE_INTEGER_CACHE
    /** Cache for commonly used integer numbers. */
    std::shared_ptr<class number> m_integer_cache[256];
#endif
  };
}

#endif /* !PLORTH_RUNTIME_HPP_GUARD */

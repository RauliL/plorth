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
#include <plorth/context.hpp>
#include <plorth/value-error.hpp>
#include <plorth/value-quote.hpp>

#include <cassert>

namespace plorth
{
  namespace api
  {
    runtime::prototype_definition global_dictionary();
    runtime::prototype_definition array_prototype();
    runtime::prototype_definition boolean_prototype();
    runtime::prototype_definition error_prototype();
    runtime::prototype_definition number_prototype();
    runtime::prototype_definition object_prototype();
    runtime::prototype_definition quote_prototype();
    runtime::prototype_definition string_prototype();
    runtime::prototype_definition symbol_prototype();
    runtime::prototype_definition word_prototype();
  }

  static inline std::shared_ptr<object> make_prototype(
    runtime*,
    const char32_t*,
    const std::shared_ptr<object>&,
    const runtime::prototype_definition&
  );

  std::shared_ptr<runtime> runtime::make(
    memory::manager& memory_manager,
    const std::shared_ptr<io::input>& input,
    const std::shared_ptr<io::output>& output
  )
  {
    const auto runtime = std::shared_ptr<class runtime>(
      new (memory_manager) class runtime(&memory_manager)
    );

    runtime->m_input = input ? input : io::input::standard(memory_manager);
    runtime->m_output = output ? output : io::output::standard(memory_manager);

    return runtime;
  }

  runtime::runtime(memory::manager* memory_manager)
    : m_memory_manager(memory_manager)
  {
    assert(memory_manager);

    m_true_value = value<class boolean>(true);
    m_false_value = value<class boolean>(false);

    for (auto& entry : api::global_dictionary())
    {
      m_dictionary.insert(word(
        symbol(entry.first),
        native_quote(entry.second)
      ));
    }

    m_object_prototype = make_prototype(
      this,
      U"object",
      std::shared_ptr<object>(),
      api::object_prototype()
    );
    m_array_prototype = make_prototype(
      this,
      U"array",
      std::shared_ptr<object>(),
      api::array_prototype()
    );
    m_boolean_prototype = make_prototype(
      this,
      U"boolean",
      std::shared_ptr<object>(),
      api::boolean_prototype()
    );
    m_error_prototype = make_prototype(
      this,
      U"error",
      std::shared_ptr<object>(),
      api::error_prototype()
    );
    m_number_prototype = make_prototype(
      this,
      U"number",
      std::shared_ptr<object>(),
      api::number_prototype()
    );
    m_quote_prototype = make_prototype(
      this,
      U"quote",
      std::shared_ptr<object>(),
      api::quote_prototype()
    );
    m_string_prototype = make_prototype(
      this,
      U"string",
      std::shared_ptr<object>(),
      api::string_prototype()
    );
    m_symbol_prototype = make_prototype(
      this,
      U"symbol",
      std::shared_ptr<object>(),
      api::symbol_prototype()
    );
    m_word_prototype = make_prototype(
      this,
      U"word",
      std::shared_ptr<object>(),
      api::word_prototype()
    );
  }

  io::input::result runtime::read(io::input::size_type size,
                                  unistring& output,
                                  io::input::size_type& read)
  {
    if (m_input)
    {
      return m_input->read(size, output, read);
    }
    read = 0;

    return io::input::result_eof;
  }

  void runtime::print(const unistring& str) const
  {
    if (m_output)
    {
      m_output->write(str);
    }
  }

  void runtime::println() const
  {
#if defined(_WIN32)
    static const unistring newline = {'\r', '\n'};
#else
    static const unistring newline = {'\n'};
#endif

    print(newline);
  }

  void runtime::println(const unistring& str) const
  {
    print(str);
    println();
  }

  void runtime::mark()
  {
    managed::mark();
    if (m_input && !m_input->marked())
    {
      m_input->mark();
    }
    if (m_output && !m_output->marked())
    {
      m_output->mark();
    }
    m_dictionary.mark();
    if (m_true_value && !m_true_value->marked())
    {
      m_true_value->mark();
    }
    if (m_false_value && !m_false_value->marked())
    {
      m_false_value->mark();
    }
    if (m_array_prototype && !m_array_prototype->marked())
    {
      m_array_prototype->mark();
    }
    if (m_boolean_prototype && !m_boolean_prototype->marked())
    {
      m_boolean_prototype->mark();
    }
    if (m_error_prototype && !m_error_prototype->marked())
    {
      m_error_prototype->mark();
    }
    if (m_number_prototype && !m_number_prototype->marked())
    {
      m_number_prototype->mark();
    }
    if (m_object_prototype && !m_object_prototype->marked())
    {
      m_object_prototype->mark();
    }
    if (m_quote_prototype && !m_quote_prototype->marked())
    {
      m_quote_prototype->mark();
    }
    if (m_string_prototype && !m_string_prototype->marked())
    {
      m_string_prototype->mark();
    }
    if (m_symbol_prototype && !m_symbol_prototype->marked())
    {
      m_symbol_prototype->mark();
    }
    if (m_word_prototype && !m_word_prototype->marked())
    {
      m_word_prototype->mark();
    }
#if PLORTH_ENABLE_MODULES
    for (auto& entry : m_imported_modules)
    {
      auto& value = entry.second;

      if (value && !value->marked())
      {
        value->mark();
      }
    }
#endif
#if PLORTH_ENABLE_SYMBOL_CACHE
    for (auto& entry : m_symbol_cache)
    {
      auto& symbol = entry.second;

      if (symbol && !symbol->marked())
      {
        symbol->mark();
      }
    }
#endif
#if PLORTH_ENABLE_INTEGER_CACHE
    for (int i = 0; i < 256; ++i)
    {
      auto& number = m_integer_cache[i];

      if (number && !number->marked())
      {
        number->mark();
      }
    }
#endif
  }

  static inline std::shared_ptr<object> make_prototype(
    class runtime* runtime,
    const char32_t* name,
    const std::shared_ptr<object>& parent_prototype,
    const runtime::prototype_definition& definition
  )
  {
    object::container_type properties;
    std::shared_ptr<object> prototype;

    for (auto& entry : definition)
    {
      properties[entry.first] = runtime->native_quote(entry.second);
    }
    properties[U"__proto__"] = parent_prototype;
    prototype = runtime->value<object>(properties);

    // Define prototype into global dictionary as constant if name has been
    // given.
    if (name)
    {
      runtime->dictionary().insert(runtime->word(
        runtime->symbol(name),
        runtime->compiled_quote({
          runtime->value<object>(object::container_type({
            { U"__proto__", runtime->object_prototype() },
            { U"prototype", prototype }
          }))
        })
      ));
    }

    return prototype;
  }
}

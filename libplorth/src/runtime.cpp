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
    const runtime::prototype_definition&
  );

  std::shared_ptr<runtime> runtime::make(
    memory::manager& memory_manager,
    const std::shared_ptr<io::input>& input,
    const std::shared_ptr<io::output>& output,
    const std::shared_ptr<module::manager>& module_manager
  )
  {
    const auto runtime = std::shared_ptr<class runtime>(
      new (memory_manager) class runtime(&memory_manager)
    );

    runtime->m_input = input ? input : io::input::standard(memory_manager);
    runtime->m_output = output ? output : io::output::standard(memory_manager);
    runtime->m_module_manager =
      module_manager ?
      module_manager :
      module::manager::file_system(memory_manager);

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
      api::object_prototype()
    );
    m_array_prototype = make_prototype(
      this,
      U"array",
      api::array_prototype()
    );
    m_boolean_prototype = make_prototype(
      this,
      U"boolean",
      api::boolean_prototype()
    );
    m_error_prototype = make_prototype(
      this,
      U"error",
      api::error_prototype()
    );
    m_number_prototype = make_prototype(
      this,
      U"number",
      api::number_prototype()
    );
    m_quote_prototype = make_prototype(
      this,
      U"quote",
      api::quote_prototype()
    );
    m_string_prototype = make_prototype(
      this,
      U"string",
      api::string_prototype()
    );
    m_symbol_prototype = make_prototype(
      this,
      U"symbol",
      api::symbol_prototype()
    );
    m_word_prototype = make_prototype(
      this,
      U"word",
      api::word_prototype()
    );
  }

  io::input::result runtime::read(io::input::size_type size,
                                  std::u32string& output,
                                  io::input::size_type& read)
  {
    if (m_input)
    {
      return m_input->read(size, output, read);
    }
    read = 0;

    return io::input::result::eof;
  }

  void runtime::print(const std::u32string& str) const
  {
    if (m_output)
    {
      m_output->write(str);
    }
  }

  void runtime::println() const
  {
#if defined(_WIN32)
    static const std::u32string newline = {'\r', '\n'};
#else
    static const std::u32string newline = {'\n'};
#endif

    print(newline);
  }

  void runtime::println(const std::u32string& str) const
  {
    print(str);
    println();
  }

  static inline std::shared_ptr<object> make_prototype(
    class runtime* runtime,
    const char32_t* name,
    const runtime::prototype_definition& definition
  )
  {
    std::vector<object::value_type> properties;
    std::shared_ptr<object> prototype;

    for (auto& entry : definition)
    {
      properties.push_back({
        entry.first,
        runtime->native_quote(entry.second)
      });
    }
    properties.push_back({ U"__proto__", std::shared_ptr<value>() });
    prototype = runtime->object(properties);

    // Define prototype into global dictionary as constant if name has been
    // given.
    if (name)
    {
      runtime->dictionary().insert(runtime->word(
        runtime->symbol(name),
        runtime->compiled_quote({
          runtime->object({
            { U"__proto__", runtime->object_prototype() },
            { U"prototype", prototype }
          })
        })
      ));
    }

    return prototype;
  }
}

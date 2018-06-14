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
#include <plorth/gui/context.hpp>
#include "./utils.hpp"

namespace plorth
{
  namespace gui
  {
#if PLORTH_ENABLE_MODULES
    static void scan_module_path(const ref<runtime>&);
#endif

    namespace
    {
      class custom_output : public io::output
      {
      public:
        explicit custom_output(Context::text_written_signal signal)
          : m_signal(signal) {}

        void write(const unistring& text)
        {
          m_signal.emit(utils::string_convert<Glib::ustring, unistring>(text));
        }

      private:
        const Context::text_written_signal m_signal;
      };
    }

    Context::Context()
      : m_runtime(runtime::make(m_memory_manager))
      , m_context(context::make(m_runtime))
    {
      const auto output = m_runtime->value<custom_output>(
        m_signal_text_written
      );

      m_runtime->output() = output;
      m_runtime->input() = io::input::dummy(m_memory_manager);

#if PLORTH_ENABLE_MODULES
      scan_module_path(m_runtime);
#endif
    }

    void Context::execute(const Glib::ustring& source_code,
                          const Glib::ustring& file,
                          int line)
    {
      auto script = m_context->compile(
        utils::string_convert<unistring, Glib::ustring>(source_code),
        utils::string_convert<unistring, Glib::ustring>(file),
        line
      );

      if (!script || !script->call(m_context))
      {
        auto error = m_context->error();

        m_context->clear_error();
        if (error)
        {
          m_signal_error_thrown.emit(error);
        }
      }
    }

#if PLORTH_ENABLE_MODULES
    static void scan_module_path(const ref<runtime>& runtime)
    {
#if defined(_WIN32)
      static const unichar path_separator = ';';
#else
      static const unichar path_separator = ':';
#endif
      auto& module_paths = runtime->module_paths();
      const char* begin = std::getenv("PLORTHPATH");
      const char* end = begin;

      if (end)
      {
        for (; *end; ++end)
        {
          if (*end != path_separator)
          {
            continue;
          }

          if (end - begin > 0)
          {
            module_paths.push_back(utf8_decode(std::string(begin, end - begin)));
          }
          begin = end + 1;
        }

        if (end - begin > 0)
        {
          module_paths.push_back(utf8_decode(std::string(begin, end - begin)));
        }
      }

#if defined(PLORTH_RUNTIME_LIBRARY_PATH)
      if (module_paths.empty())
      {
        module_paths.push_back(PLORTH_RUNTIME_LIBRARY_PATH);
      }
#endif
    }
#endif
  }
}

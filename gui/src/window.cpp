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
#include <plorth/gui/window.hpp>
#include "./utils.hpp"

namespace plorth
{
  namespace gui
  {
    const int Window::DEFAULT_WIDTH = 450;
    const int Window::DEFAULT_HEIGHT = 250;

    static void count_open_braces(const Glib::ustring&, std::stack<unichar>&);
#if PLORTH_ENABLE_MODULES
    static void scan_module_path(const std::shared_ptr<runtime>&);
#endif

    namespace
    {
      class custom_output : public io::output
      {
      public:
        explicit custom_output(Window::text_written_signal signal)
          : m_signal(signal) {}

        void write(const unistring& text)
        {
          m_signal.emit(utils::string_convert<Glib::ustring, unistring>(text));
        }

      private:
        const Window::text_written_signal m_signal;
      };
    }

    Window::Window()
      : m_runtime(runtime::make(m_memory_manager))
      , m_context(context::make(m_runtime))
      , m_box(Gtk::ORIENTATION_VERTICAL)
    {
      const auto output = m_runtime->value<custom_output>(
        m_text_written_signal
      );

      set_title("Plorth");
      set_border_width(5);
      set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);

      m_runtime->output() = output;
      m_runtime->input() = io::input::dummy(m_memory_manager);

#if PLORTH_ENABLE_MODULES
      scan_module_path(m_runtime);
#endif

      m_paned.pack1(m_line_display, true, false);
      m_paned.pack2(m_stack_display, false, false);

      m_box.pack_start(m_paned);
      m_box.pack_start(m_line_editor, Gtk::PACK_SHRINK);

      add(m_box);
      show_all_children();

      m_line_editor.signal_line_received().connect(sigc::mem_fun(
        this,
        &Window::on_line_received
      ));
      m_text_written_signal.connect(sigc::mem_fun(
        this,
        &Window::on_text_written
      ));
    }

    void Window::on_show()
    {
      Gtk::Window::on_show();
      m_line_editor.grab_focus();
    }

    void Window::on_line_received(const Glib::ustring& line)
    {
      if (line.empty())
      {
        return;
      }
      m_line_display.add_line(line + '\n', LineDisplay::LINE_TYPE_INPUT);
      m_source.append(utils::string_convert<unistring, Glib::ustring>(line));
      m_source.append(1, '\n');
      count_open_braces(line, m_open_braces);
      if (m_open_braces.empty())
      {
        auto script = m_context->compile(
          m_source,
          U"<eval>",
          m_line_editor.get_line_count() - 1
        );

        m_source.clear();
        if (!script || !script->call(m_context))
        {
          const auto error = m_context->error();

          m_context->clear_error();
          if (error)
          {
            m_line_display.add_line(
              utils::string_convert<Glib::ustring, unistring>(
                error->to_string()
              ) + '\n',
              LineDisplay::LINE_TYPE_ERROR
            );
          }
        }
        m_line_editor.set_stack_depth_count(m_context->size());
        m_stack_display.update(m_context->data());
      }
    }

    void Window::on_text_written(const Glib::ustring& text)
    {
      m_line_display.add_line(text, LineDisplay::LINE_TYPE_OUTPUT);
    }

    static void count_open_braces(const Glib::ustring& input,
                                  std::stack<unichar>& open_braces)
    {
      const auto length = input.length();

      for (Glib::ustring::size_type i = 0; i < length; ++i)
      {
        switch (input[i])
        {
          case '#':
            return;

          case '(':
            open_braces.push(')');
            break;

          case '[':
            open_braces.push(']');
            break;

          case '{':
            open_braces.push('}');
            break;

          case ')':
          case ']':
          case '}':
            if (!open_braces.empty() && open_braces.top() == input[i])
            {
              open_braces.pop();
            }
            break;

          case '"':
            while (i < length)
            {
              if (input[i] == '"')
              {
                break;
              }
              else if (input[i] == '\\' && i + 1 < length)
              {
                i += 2;
              } else {
                ++i;
              }
            }
        }
      }
    }

#if PLORTH_ENABLE_MODULES
    static void scan_module_path(const std::shared_ptr<runtime>& runtime)
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

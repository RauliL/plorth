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
#include <plorth/gui/line-editor.hpp>

namespace plorth
{
  namespace gui
  {
    LineEditor::LineEditor()
      : m_line_count(1)
      , m_stack_depth_count(0)
      , m_box(Gtk::ORIENTATION_HORIZONTAL)
    {
      Pango::FontDescription font;

      font.set_family("monospace");

      update_prompt();
      m_label.override_font(font);

      m_entry.set_has_frame(false);
      m_entry.override_font(font);

      m_box.pack_start(m_label, Gtk::PACK_SHRINK);
      m_box.pack_start(m_entry);
      add(m_box);

      m_entry.signal_activate().connect(sigc::mem_fun(
        this,
        &LineEditor::on_activate
      ));
    }

    void LineEditor::grab_focus()
    {
      m_entry.grab_focus_without_selecting();
    }

    void LineEditor::set_line_count(int line_count)
    {
      if (m_line_count != line_count)
      {
        m_line_count = line_count;
        update_prompt();
      }
    }

    void LineEditor::set_stack_depth_count(int stack_depth_count)
    {
      if (m_stack_depth_count != stack_depth_count)
      {
        m_stack_depth_count = stack_depth_count;
        update_prompt();
      }
    }

    void LineEditor::on_activate()
    {
      set_line_count(m_line_count + 1);
      m_signal_line_received.emit(m_entry.get_text());
      m_entry.set_text(Glib::ustring());
    }

    void LineEditor::update_prompt()
    {
      m_label.set_text(Glib::ustring::format(
        "plorth:",
        m_line_count,
        ':',
        m_stack_depth_count,
        '>'
      ));
    }
  }
}

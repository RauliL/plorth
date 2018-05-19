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
#ifndef PLORTH_GUI_LINE_EDITOR_HPP_GUARD
#define PLORTH_GUI_LINE_EDITOR_HPP_GUARD

#include <gtkmm.h>

namespace plorth
{
  namespace gui
  {
    /**
     * Custom GTK widget which renders a prompt and a single line text entry
     * field.
     */
    class LineEditor : public Gtk::Bin
    {
    public:
      explicit LineEditor();

      void grab_focus();

      inline int get_line_count() const
      {
        return m_line_count;
      }

      void set_line_count(int line_count);

      inline int get_stack_depth_count() const
      {
        return m_stack_depth_count;
      }

      void set_stack_depth_count(int stack_depth_count);

      inline sigc::signal<void, Glib::ustring>& signal_line_received()
      {
        return m_signal_line_received;
      }

    protected:
      /**
       * Signal callback which is used when enter has been pressed to the text
       * entry.
       */
      void on_activate();

      void update_prompt();

    private:
      /** Line counter for the prompt. */
      int m_line_count;
      /** Stack depth counter for the prompt. */
      int m_stack_depth_count;
      /** Container for the editor widgets. */
      Gtk::Box m_box;
      /** Label for the prompt. */
      Gtk::Label m_label;
      /** Text entry widget. */
      Gtk::Entry m_entry;
      /** Signal when a line has been entered into the editor. */
      sigc::signal<void, Glib::ustring> m_signal_line_received;
    };
  }
}

#endif /* !PLORTH_GUI_LINE_EDITOR_HPP_GUARD */

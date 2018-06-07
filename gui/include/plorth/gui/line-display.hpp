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
#ifndef PLORTH_GUI_LINE_DISPLAY_HPP_GUARD
#define PLORTH_GUI_LINE_DISPLAY_HPP_GUARD

#include <gtkmm.h>

namespace plorth
{
  namespace gui
  {
    /**
     * Custom GTK widget for displaying lines of text.
     */
    class LineDisplay : public Gtk::Bin
    {
    public:
      enum LineType
      {
        LINE_TYPE_INPUT,
        LINE_TYPE_OUTPUT,
        LINE_TYPE_ERROR
      };

      explicit LineDisplay();

      void add_line(const Glib::ustring& line, LineType type = LINE_TYPE_OUTPUT);
      void scroll_to_bottom();

    private:
      Gtk::ScrolledWindow m_scrolled_window;
      Gtk::TextView m_text_view;
      Glib::RefPtr<Gtk::TextBuffer> m_text_buffer;
      Glib::RefPtr<Gtk::TextTag> m_input_tag;
      Glib::RefPtr<Gtk::TextTag> m_output_tag;
      Glib::RefPtr<Gtk::TextTag> m_error_tag;
    };
  }
}

#endif /* !PLORTH_GUI_LINE_DISPLAY_HPP_GUARD */

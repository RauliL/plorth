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
#include <plorth/gui/stack-display.hpp>
#include "./utils.hpp"

namespace plorth
{
  namespace gui
  {
    StackDisplayColumns::StackDisplayColumns()
    {
      add(m_index_column);
      add(m_value_column);
    }

    StackDisplay::StackDisplay()
      : m_tree_model(Gtk::ListStore::create(m_columns))
    {
      m_tree_view.set_model(m_tree_model);
      m_tree_view.append_column("#", m_columns.index_column());
      m_tree_view.append_column("Value", m_columns.value_column());

      m_scrolled_window.set_policy(
        Gtk::POLICY_AUTOMATIC,
        Gtk::POLICY_AUTOMATIC
      );
      m_scrolled_window.add(m_tree_view);

      add(m_scrolled_window);
    }

    void StackDisplay::update(const context::container_type& stack)
    {
      auto it = stack.rbegin();
      const auto end = stack.rend();
      const auto index_column = m_columns.index_column();
      const auto value_column = m_columns.value_column();
      int index = 0;

      m_tree_model->clear();

      for (; it != end; ++it)
      {
        const auto& value = *it;
        auto row = *(m_tree_model->append());

        row[index_column] = ++index;
        row[value_column] = utils::string_convert<Glib::ustring, unistring>(
          value ? value->to_source() : U"null"
        );
      }
    }
  }
}

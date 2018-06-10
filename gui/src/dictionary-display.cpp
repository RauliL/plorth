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
#include <plorth/gui/dictionary-display.hpp>
#include "./utils.hpp"

namespace plorth
{
  namespace gui
  {
    DictionaryDisplayColumns::DictionaryDisplayColumns()
    {
      add(m_symbol_column);
      add(m_quote_column);
    }

    DictionaryDisplay::DictionaryDisplay()
      : m_tree_model(Gtk::ListStore::create(m_columns))
    {
      const auto& symbol_column = m_columns.symbol_column();
      const auto& quote_column = m_columns.quote_column();

      m_tree_model->set_sort_column(symbol_column, Gtk::SORT_ASCENDING);
      m_tree_view.override_font(utils::get_monospace_font());
      m_tree_view.set_model(m_tree_model);
      m_tree_view.append_column("Symbol", symbol_column);
      m_tree_view.append_column("Quote", quote_column);
      m_tree_view.signal_row_activated().connect(sigc::mem_fun(
        this,
        &DictionaryDisplay::on_row_activated
      ));

      m_scrolled_window.set_policy(
        Gtk::POLICY_AUTOMATIC,
        Gtk::POLICY_AUTOMATIC
      );
      m_scrolled_window.add(m_tree_view);

      add(m_scrolled_window);
    }

    void DictionaryDisplay::update(const class dictionary& dictionary)
    {
      const auto symbol_column = m_columns.symbol_column();
      const auto quote_column = m_columns.quote_column();

      m_tree_model->clear();

      for (const auto& word : dictionary.words())
      {
        auto row = *(m_tree_model->append());
        const auto& symbol = word->symbol();
        const auto& quote = word->quote();

        row[symbol_column] = utils::string_convert<Glib::ustring, unistring>(
          symbol ? symbol->to_string() : U""
        );
        row[quote_column] = utils::string_convert<Glib::ustring, unistring>(
          quote ? quote->to_string() : U"(null)"
        );
      }
    }

    void DictionaryDisplay::on_row_activated(const Gtk::TreeModel::Path& path,
                                             Gtk::TreeViewColumn* column)
    {
      const auto iter = m_tree_model->get_iter(path);

      if (iter)
      {
        const auto& row = *iter;

        m_signal_word_activated.emit(
          row[m_columns.symbol_column()],
          row[m_columns.quote_column()]
        );
      }
    }
  }
}

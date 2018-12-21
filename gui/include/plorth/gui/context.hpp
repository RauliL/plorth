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
#ifndef PLORTH_GUI_CONTEXT_HPP_GUARD
#define PLORTH_GUI_CONTEXT_HPP_GUARD

#include <plorth/plorth.hpp>

#include <glibmm.h>

namespace plorth
{
  namespace gui
  {
    /**
     * Wraps Plorth runtime and execution context into single Glib object.
     */
    class Context : public Glib::ObjectBase
    {
    public:
      using error_thrown_signal = sigc::signal<void, ref<error>>;
      using text_written_signal = sigc::signal<void, Glib::ustring>;

      explicit Context();

      /**
       * Returns the data stack of the execution context.
       */
      inline context::container_type& stack()
      {
        return m_context->data();
      }

      /**
       * Returns the data stack of the execution context.
       */
      inline const context::container_type& stack() const
      {
        return m_context->data();
      }

      /**
       * Returns local dictionary of the execution context.
       */
      inline class dictionary& dictionary()
      {
        return m_context->dictionary();
      }

      /**
       * Returns local dictionary of the execution context.
       */
      inline const class dictionary& dictionary() const
      {
        return m_context->dictionary();
      }

      void execute(
        const Glib::ustring& source_code,
        const Glib::ustring& file = "<eval>",
        int line = 1
      );

      inline error_thrown_signal& signal_error_thrown()
      {
        return m_signal_error_thrown;
      }

      inline const error_thrown_signal& signal_error_thrown() const
      {
        return m_signal_error_thrown;
      }

      inline text_written_signal& signal_text_written()
      {
        return m_signal_text_written;
      }

      inline const text_written_signal& signal_text_written() const
      {
        return m_signal_text_written;
      }

    private:
      memory::manager m_memory_manager;
      ref<runtime> m_runtime;
      ref<context> m_context;
      error_thrown_signal m_signal_error_thrown;
      text_written_signal m_signal_text_written;
    };
  }
}

#endif /* !PLORTH_GUI_CONTEXT_HPP_GUARD */

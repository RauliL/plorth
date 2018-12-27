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
#ifndef PLORTH_IO_OUTPUT_HPP_GUARD
#define PLORTH_IO_OUTPUT_HPP_GUARD

#include <plorth/memory.hpp>
#include <plorth/unicode.hpp>

namespace plorth
{
  namespace io
  {
    /**
     * Class which implements text output for the Plorth interpreter.
     */
    class output : public memory::managed
    {
    public:
      /**
       * Constructs new output which prints everything into the standard output
       * stream (stdout) of the process, if standard I/O has been enabled. The
       * output will be encoded in UTF-8.
       */
      static std::shared_ptr<output> standard(memory::manager& memory_manager);

      /**
       * Constructs new output which ignores everything that will be written
       * into it.
       */
      static std::shared_ptr<output> dummy(memory::manager& memory_manager);

      /**
       * Writes given Unicode string into the output.
       *
       * \param str String to write into the output.
       */
      virtual void write(const std::u32string& str) = 0;
    };
  }
}

#endif /* !PLORTH_IO_OUTPUT_HPP_GUARD */

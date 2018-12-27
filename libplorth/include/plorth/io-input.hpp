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
#ifndef PLORTH_IO_INPUT_HPP_GUARD
#define PLORTH_IO_INPUT_HPP_GUARD

#include <plorth/memory.hpp>
#include <plorth/unicode.hpp>

namespace plorth
{
  namespace io
  {
    /**
     * Class which implements text input for the Plorth interpreter.
     */
    class input : public memory::managed
    {
    public:
      using size_type = std::size_t;

      /**
       * Represents results of read operation.
       */
      enum class result
      {
        /** Reading from standard input stream was successful. */
        ok = 1,
        /** End of input was encountered. */
        eof = -1,
        /** Unicode decoding error was encountered. */
        failure = 0
      };

      /**
       * Constructs new input which reads from standard input stream (stdin) of
       * the process, if standard I/O has been enabled. The input is expected
       * to be UTF-8 encoded.
       */
      static std::shared_ptr<input> standard(memory::manager& memory_manager);

      /**
       * Constructs new input which reads nothing.
       */
      static std::shared_ptr<input> dummy(memory::manager& memory_manager);

      /**
       * Reads Unicode code points from the input and places them into the
       * string given as argument.
       *
       * \param size   Number of Unicode characters to be read. If zero is
       *               given, input will be consumed until there is no more
       *               input to be read.
       * \param output Where the read Unicode characters will be placed into.
       * \param read   Where the amount of read Unicode characters will be
       *               placed into.
       */
      virtual result read(
        size_type size,
        std::u32string& output,
        size_type& read
      ) = 0;
    };
  }
}

#endif /* !PLORTH_IO_INPUT_HPP_GUARD */

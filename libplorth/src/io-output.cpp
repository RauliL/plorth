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
#include <plorth/io-output.hpp>

namespace plorth
{
  namespace
  {
    class dummy_output : public io::output
    {
    public:
      void write(const std::u32string&) {}
    };

#if PLORTH_ENABLE_STANDARD_IO
    class standard_output : public io::output
    {
    public:
      void write(const std::u32string& str)
      {
        const auto bytes = utf8_encode(str);

        std::fwrite(
          static_cast<const void*>(bytes.c_str()),
          sizeof(char),
          bytes.length(),
          stdout
        );
      }
    };
#endif
  }

  namespace io
  {
    std::shared_ptr<output> output::standard(memory::manager& memory_manager)
    {
#if PLORTH_ENABLE_STANDARD_IO
      return std::shared_ptr<output>(new (memory_manager) standard_output());
#else
      return dummy(memory_manager);
#endif
    }

    std::shared_ptr<output> output::dummy(memory::manager& memory_manager)
    {
      return std::shared_ptr<output>(new (memory_manager) dummy_output());
    }
  }
}

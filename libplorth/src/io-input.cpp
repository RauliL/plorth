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
#include <plorth/io-input.hpp>

namespace plorth
{
  namespace
  {
#if PLORTH_ENABLE_STANDARD_IO
    class standard_input : public io::input
    {
    public:
      result read(size_type size, std::u32string& output, size_type& read)
      {
        const bool infinite = !size;
        const auto eof = std::char_traits<char>::eof();
        std::string buffer;

        buffer.reserve(6);
        read = 0;
        while (infinite || size > 0)
        {
          auto byte = std::cin.get();
          std::size_t unicode_size;

          if (byte == eof)
          {
            return result::eof;
          }
          else if (!(unicode_size = utf8_sequence_length(byte)))
          {
            return result::failure;
          }
          buffer.clear();
          buffer.append(1, byte);
          for (std::size_t i = 1; i < unicode_size; ++i)
          {
            if ((byte = std::cin.get()) == eof)
            {
              return result::failure;
            }
            buffer.append(1, byte);
          }
          if (!utf8_decode_test(buffer, output))
          {
            return result::failure;
          }
          if (!infinite)
          {
            --size;
          }
          ++read;
        }

        return result::ok;
      }
    };
#endif

    class dummy_input : public io::input
    {
    public:
      result read(size_type size, std::u32string& output, size_type& read)
      {
        read = 0;

        return result::eof;
      }
    };
  }

  namespace io
  {
    std::shared_ptr<input> input::standard(memory::manager& memory_manager)
    {
#if PLORTH_ENABLE_STANDARD_IO
      return std::shared_ptr<input>(new (memory_manager) standard_input());
#else
      return dummy(memory_manager);
#endif
    }

    std::shared_ptr<input> input::dummy(memory::manager& memory_manager)
    {
      return std::shared_ptr<input>(new (memory_manager) dummy_input());
    }
  }
}

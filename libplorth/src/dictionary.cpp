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
#include <plorth/dictionary.hpp>

namespace plorth
{
  dictionary::dictionary() {}

  dictionary::dictionary(const dictionary& that)
    : m_words(that.m_words) {}

  dictionary& dictionary::operator=(const dictionary& that)
  {
    m_words = that.m_words;

    return *this;
  }

  dictionary::value_type dictionary::find(
    const std::shared_ptr<symbol>& id
  ) const
  {
    return find(id->id());
  }

  std::vector<dictionary::value_type> dictionary::words() const
  {
    std::vector<value_type> result;

    result.reserve(m_words.size());
    for (const auto& entry : m_words)
    {
      result.push_back(entry.second);
    }

    return result;
  }

  dictionary::value_type dictionary::find(const std::u32string& id) const
  {
    const auto entry = m_words.find(id);

    if (entry == std::end(m_words))
    {
      return value_type();
    } else {
      return entry->second;
    }
  }

  void dictionary::insert(const value_type& word)
  {
    m_words[word->symbol()->id()] = word;
  }
}

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
#ifndef PLORTH_DICTIONARY_HPP
#define PLORTH_DICTIONARY_HPP

#include <plorth/value-word.hpp>

#include <unordered_map>
#include <vector>

namespace plorth
{
  /**
   * Dictionary is a collection of words, containing one quote for each
   * individual symbol.
   */
  class dictionary
  {
  public:
    using value_type = ref<word>;
    using reference = value_type&;
    using const_reference = const value_type&;

    /**
     * Constructs new empty dictionary.
     */
    dictionary();

    /**
     * Constructs copy of existing dictionary.
     */
    dictionary(const dictionary& that);

    /**
     * Copies contents of another dictionary into this one.
     */
    dictionary& operator=(const dictionary& that);

    /**
     * Returns words from the dictionary as iterable vector.
     */
    std::vector<value_type> words() const;

    /**
     * Searches for a word from the dictionary which symbol matches with given
     * symbol. If no such word is found from the dictionary, null reference
     * will be returned instead.
     */
    value_type find(const ref<symbol>& id) const;

    /**
     * Searches for a word from the dictionary which symbol matches with given
     * string. If no such word is found from the dictionary, null reference
     * will be returned instead.
     */
    value_type find(const unistring& id) const;

    /**
     * Inserts given word into the dictionary. Existing words with identical
     * symbol will be overridden.
     */
    void insert(const_reference word);

    /**
     * Goes through all the words stored in the dictionary and marks them.
     */
    void mark();

  private:
    /** Container for the words in the dictionary. */
    std::unordered_map<unistring, word*> m_words;
  };
}

#endif /* !PLORTH_DICTIONARY_HPP */

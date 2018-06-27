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
#ifndef PLORTH_VALUE_OBJECT_HPP_GUARD
#define PLORTH_VALUE_OBJECT_HPP_GUARD

#include <plorth/value.hpp>

#include <unordered_map>

namespace plorth
{
  class object : public value
  {
  public:
    using key_type = unistring;
    using mapped_type = ref<value>;
    using value_type = std::pair<const key_type, mapped_type>;
    using size_type = std::size_t;
    using container_type = std::unordered_map<key_type, mapped_type>;

    virtual size_type size() const = 0;
    virtual std::vector<key_type> keys() const = 0;
    virtual std::vector<mapped_type> values() const = 0;
    virtual std::vector<value_type> entries() const = 0;

    /**
     * Retrieves property with given name from the object itself and it's
     * prototypes.
     *
     * \param runtime   Scripting runtime. Required for prototype chain
     *                  inheritance.
     * \param name      Name of the property to retrieve.
     * \param slot      Where value of the found property will be assigned to.
     * \param inherited Whether inherited properties from prototype chain
     *                  should be included in the search or not.
     * \return          Boolean flag which tells whether the property was found
     *                  or not.
     */
    bool property(
      const ref<class runtime>& runtime,
      const key_type& name,
      mapped_type& slot,
      bool inherited = true
    ) const;

    virtual bool own_property(
      const key_type& name,
      mapped_type& slot
    ) const = 0;

    inline enum type type() const
    {
      return type_object;
    }

    bool equals(const ref<value>& that) const;
    unistring to_string() const;
    unistring to_source() const;
  };
}

#endif /* !PLORTH_VALUE_OBJECT_HPP_GUARD */

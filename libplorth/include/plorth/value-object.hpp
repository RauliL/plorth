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

#include <utility>
#include <vector>

#include <plorth/value.hpp>

namespace plorth
{
  /**
   * Object is an associative container which maps strings into values. Just
   * like in JavaScript, Plorth objects also supports inheritance through
   * prototypes, if the object has an property called "__proto__" that maps to
   * another object.
   */
  class object : public value
  {
  public:
    using size_type = std::size_t;
    using key_type = std::u32string;
    using mapped_type = std::shared_ptr<value>;
    using value_type = std::pair<key_type, mapped_type>;

    /**
     * Tests whether the object has property with given name, including
     * inherited properties.
     *
     * \param runtime Scripting runtime. Required for prototype chain
     *                inheritance.
     * \param key     Name of the property to test existance of.
     * \return        Boolean flag which tells whether the property exists or
     *                not.
     */
    bool has_property(
      const std::shared_ptr<class runtime>& runtime,
      const key_type& key
    ) const;

    /**
     * Tests whether the object has property with given name, omitting
     * inherited properties.
     *
     * \param key Name of the property to test existance of.
     * \return    Boolean flag which tells whether the property exists or not.
     */
    virtual bool has_own_property(const key_type& key) const = 0;

    /**
     * Retrieves property with given name from the object itself and it's
     * prototypes.
     *
     * \param runtime Scripting runtime. Required for prototype chain
     *                inheritance.
     * \param key     Name of the property to retrieve.
     * \param slot    Where value of the found property will be assigned to.
     * \return        Boolean flag which tells whether the property was found
     *                or not.
     */
    bool property(
      const std::shared_ptr<class runtime>& runtime,
      const key_type& key,
      mapped_type& slot
    ) const;

    /**
     * Retrieves property with given name from the object itself, omitting
     * prototype inheritance.
     *
     * \param key  Name of the property to retrieve.
     * \param slot Where value of the found property will be assigned to.
     * \return     Boolean flag which tells whether the property was found or
     *             not.
     */
    virtual bool own_property(
      const key_type& key,
      mapped_type& slot
    ) const = 0;

    /**
     * Returns the number of properties (not including inherited ones) which
     * the object has.
     */
    virtual size_type size() const = 0;

    /**
     * Returns names of the properties which the object has. This does not
     * include inherited properties.
     */
    virtual std::vector<key_type> keys() const = 0;

    /**
     * Returns values of the properties which the object has. This does not
     * include inherited properties.
     */
    virtual std::vector<mapped_type> values() const = 0;

    /**
     * Returns each property which the object has. This does not include
     * inherited properties.
     */
    virtual std::vector<value_type> entries() const = 0;

    inline enum type type() const
    {
      return type::object;
    }

    bool equals(const std::shared_ptr<value>& that) const;
    std::u32string to_string() const;
    std::u32string to_source() const;
  };
}

#endif /* !PLORTH_VALUE_OBJECT_HPP_GUARD */

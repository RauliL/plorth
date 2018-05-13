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
#ifndef PLORTH_REF_HPP_GUARD
#define PLORTH_REF_HPP_GUARD

namespace plorth
{
  /**
   * Wrapper class for objects that are managed by the garbage collector. It
   * automatically increases and decreases reference counter of the wrapped
   * object which is used by the gargabe collector to find out whether the
   * object is still used or not.
   */
  template<class T>
  class ref
  {
  public:
    using value_type = T;
    using pointer = value_type*;

    /**
     * Constructs null reference.
     */
    ref()
      : m_object(nullptr) {}

    /**
     * Constructs copy of existing reference.
     */
    ref(const ref<T>& that)
      : m_object(that.m_object)
    {
      if (m_object)
      {
        m_object->inc_use_count();
      }
    }

    /**
     * Moves wrapped object from another reference into this one.
     */
    ref(ref<T>&& that)
      : m_object(that.m_object)
    {
      that.m_object = nullptr;
    }

    /**
     * Constructs reference from given pointer to an reference counted object.
     */
    explicit ref(pointer object)
      : m_object(object)
    {
      if (m_object)
      {
        m_object->inc_use_count();
      }
    }

    /**
     * Destructor.
     */
    ~ref()
    {
      reset();
    }

    /**
     * Copies wrapped object from another reference into this one.
     */
    ref& operator=(const ref<T>& that)
    {
      if (m_object != that.m_object)
      {
        reset();
        if ((m_object = that.m_object))
        {
          m_object->inc_use_count();
        }
      }

      return *this;
    }

    /**
     * Moves wrapped object from another reference into this one.
     */
    ref& operator=(ref<T>&& that)
    {
      if (this != &that)
      {
        reset();
        m_object = that.m_object;
        that.m_object = nullptr;
      }

      return *this;
    }

    /**
     * Returns pointer to the wrapped object.
     */
    inline pointer get() const
    {
      return m_object;
    }

    /**
     * Returns pointer to the wrapped object, so that it's members can be
		 * accessed directly.
     */
    inline pointer operator->() const
    {
      return m_object;
    }

    /**
     * Releases reference to the wrapped oject. After this has been called,
     * this reference will become a null reference.
     */
    void reset()
    {
      if (m_object)
      {
        m_object->dec_use_count();
        m_object = nullptr;
      }
    }

    /**
     * Boolean coercion of the wrapped object. Null reference will coerce into
     * false, everything else into true.
     */
    inline explicit operator bool() const
    {
      return !!m_object;
    }

    /**
     * Negated boolean coercion of the wrapped object. Non-null reference will
     * coerce into false while null reference will coerce into true.
     */
    inline bool operator!() const
    {
      return !m_object;
    }

    /**
     * Casts the wrapped object into another type and returns the casted
     * pointer.
     */
    template<class U>
    inline ref<U> cast() const
    {
      return ref<U>(static_cast<U*>(m_object));
    }

  private:
    /** Pointer to the wrapped object. */
    pointer m_object;
  };
}

#endif /* !PLORTH_REF_HPP_GUARD */

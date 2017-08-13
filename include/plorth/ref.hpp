#ifndef PLORTH_REF_HPP_GUARD
#define PLORTH_REF_HPP_GUARD

#include <plorth/plorth.hpp>

namespace plorth
{
  /**
   * Wrapper class for objects that are managed by the garbage collector. It
   * automatically increases and decreases reference counter of the wrapped
   * object and destroy the object when it's reference count reaches to zero.
   */
  template< typename T >
  class ref
  {
  public:
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
        m_object->inc_ref_count();
      }
    }

    /**
     * Constructs copy of existing reference.
     */
    template< typename U >
    ref(const ref<U>& that)
      : m_object(that.get())
    {
      if (m_object)
      {
        m_object->inc_ref_count();
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
    template< typename U >
    ref(U* object)
      : m_object(object)
    {
      if (m_object)
      {
        m_object->inc_ref_count();
      }
    }

    /**
     * Destructor.
     */
    ~ref()
    {
      release();
    }

    /**
     * Copies wrapped object from another reference into this one.
     */
    ref& operator=(const ref<T>& that)
    {
      if (m_object != that.m_object)
      {
        release();
        if ((m_object = that.m_object))
        {
          m_object->inc_ref_count();
        }
      }

      return *this;
    }

    /**
     * Copies wrapped object from another reference into this one.
     */
    template< typename U >
    ref& operator=(const ref<U>& that)
    {
      U* object = that.get();

      if (m_object != object)
      {
        release();
        if ((m_object = object))
        {
          m_object->inc_ref_count();
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
        m_object = that.m_object;
        that.m_object = nullptr;
      }

      return *this;
    }

    /**
     * Returns pointer to the wrapped object.
     */
    inline T* get() const
    {
      return m_object;
    }

    /**
     * Returns pointer to the wrapped object, so that it's members can be
     * accessed directly.
     */
    inline T* operator->() const
    {
      return m_object;
    }

    /**
     * Releases reference to the wrapped oject. After this has been called,
     * this reference will become a null reference.
     */
    void release()
    {
      if (!m_object)
      {
        return;
      }
      else if (m_object->dec_ref_count())
      {
        delete m_object;
      }
      m_object = nullptr;
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
    template< typename U >
    inline U* cast() const
    {
      return static_cast<U*>(m_object);
    }

  private:
    /** Pointer to the wrapped object. */
    T* m_object;
  };
}

#endif /* !PLORTH_REF_HPP_GUARD */

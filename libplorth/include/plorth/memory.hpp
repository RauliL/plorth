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
#ifndef PLORTH_MEMORY_HPP_GUARD
#define PLORTH_MEMORY_HPP_GUARD

#include <plorth/config.hpp>

#include <cstddef>
#include <memory>

namespace plorth
{
  class context;
  class runtime;

  /**
   * Wrapper class for objects that are managed by the garbage collector. It
   * automatically increases and decreases reference counter of the wrapped
   * object and destroys the object when it's reference count reaches to zero.
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
      : m_value(nullptr) {}

    /**
     * Constructs copy of existing reference.
     */
    ref(const ref<T>& that)
      : m_value(that.m_value)
    {
      if (m_value)
      {
        m_value->inc_ref_count();
      }
    }

    /**
     * Constructs copy of existing reference.
     */
    template<class U>
    ref(const ref<U>& that)
      : m_value(that.get())
    {
      if (m_value)
      {
        m_value->inc_ref_count();
      }
    }

    /**
     * Moves wrapped object from existing reference into new one.
     */
    ref(ref<T>&& that)
      : m_value(that.m_value)
    {
      that.m_value = nullptr;
    }

    /**
     * Constructs reference from pointer to an referenced counted object.
     */
    template<class U>
    ref(U* value)
      : m_value(value)
    {
      if (m_value)
      {
        m_value->inc_ref_count();
      }
    }

    /**
     * Destructor. Decreases reference count of the wrapped object.
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
      if (m_value != that.m_value)
      {
        reset();
        if ((m_value = that.m_value))
        {
          m_value->inc_ref_count();
        }
      }

      return *this;
    }

    /**
     * Copies wrapped object from another reference into this one.
     */
    template<class U>
    ref& operator=(const ref<U>& that)
    {
      auto value = that.get();

      if (m_value != value)
      {
        reset();
        if ((m_value = value))
        {
          m_value->inc_ref_count();
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
        m_value = that.m_value;
        that.m_value = nullptr;
      }

      return *this;
    }

    /**
     * Statically casts reference into another type.
     */
    template<class U>
    inline ref<U> cast() const
    {
      return ref<U>(static_cast<U*>(m_value));
    }

    /**
     * Releases refernece to the wrapped object. After this has been called,
     * this reference will become null reference.
     */
    void reset()
    {
      if (!m_value)
      {
        return;
      }
      m_value->dec_ref_count();
      if (m_value->reference_count() == 0)
      {
        delete m_value;
      }
      m_value = nullptr;
    }

    /**
     * Returns pointer to the wrapped object.
     */
    inline pointer get() const
    {
      return m_value;
    }

    /**
     * Returns pointer to the wrapped object, so that it's members can be
     * accessed directly.
     */
    inline pointer operator->() const
    {
      return m_value;
    }

    /**
     * Boolean coercion of the wrapped object. Null references will coerce into
     * false, everything else into true.
     */
    inline explicit operator bool() const
    {
      return !!m_value;
    }

    /**
     * Negated boolean coercion of the wrapped object. Non-null references will
     * coerce into false, while null references will coerce into true.
     */
    inline bool operator!() const
    {
      return !m_value;
    }

  private:
    /** Pointer to the referenced object. */
    pointer m_value;
  };

  namespace memory
  {
    struct pool;
    struct slot;

    /**
     * Memory manager manages memory pools used by the interpreter and is used
     * for allocated memory for managed objects.
     */
    class manager
    {
    public:
      /**
       * Constructs new memory manager.
       */
      explicit manager();

      /**
       * Destructor. Note that when a memory manager is being destroyed, all
       * objects allocated using it are also destroyed and the memory used by
       * them is being freed.
       */
      ~manager();

      /**
       * Allocates memory for a managed object from memory pools of this memory
       * manager. New memory pools are being created when previous ones are
       * full.
       *
       * \param size Size of the object to allocate memory for.
       * \return     Pointer to the allocated memory.
       */
      void* allocate(std::size_t size);

      manager(const manager&) = delete;
      manager(manager&&) = delete;
      void operator=(const manager&) = delete;
      void operator=(manager&&) = delete;

#if PLORTH_ENABLE_MEMORY_POOL
    private:
      /** Pointer to the first memory pool used by this manager. */
      pool* m_pool_head;
      /** Pointer to the last memory pool used by this manager. */
      pool* m_pool_tail;
#endif
    };

    /**
     * Base class for all objects which memory allocation is being handled
     * through a memory manager.
     *
     * Instances of classes which derive from this class should always be
     * wrapped into references, as they keep track of the usage of the object.
     */
    class managed
    {
    public:
      /**
       * Constructs new managed object with zero references.
       */
      explicit managed();

      /**
       * Default destructor for a managed object. Marks the memory slot used by
       * the object as free in the memory pool where the object belongs to.
       */
      virtual ~managed();

      /**
       * Returns the number of references to the object.
       */
      inline std::size_t reference_count() const
      {
        return m_reference_count;
      }

      /**
       * Increases the reference counter of the object.
       */
      inline void inc_ref_count()
      {
        ++m_reference_count;
      }

      /**
       * Decreases the reference counter of the object.
       */
      inline void dec_ref_count()
      {
        --m_reference_count;
      }

      void* operator new(std::size_t size, class manager& manager);
      void operator delete(void* pointer);

      managed(const managed&) = delete;
      managed(managed&&) = delete;
      void operator=(const managed&) = delete;
      void operator=(managed&&) = delete;

    private:
      /** Number of references to the object. */
      std::size_t m_reference_count;
    };

#if PLORTH_ENABLE_MEMORY_POOL
    struct pool
    {
      /** Pointer to the next pool in the memory manager. */
      pool* next;
      /** Pointer to the previous pool in the memory manager. */
      pool* prev;
      /** Amount of bytes still unslotted in this pool. */
      std::size_t remaining;
      /** Pointer to the allocated memory. */
      char* memory;
      /** Pointer to the first free slot in the pool. */
      slot* free_head;
      /** Pointer to the last free slot in the pool. */
      slot* free_tail;
      /** Pointer to the first used slot in the pool. */
      slot* used_head;
      /** Pointer to the last used slot in the pool. */
      slot* used_tail;
    };

    struct slot
    {
      /** Memory pool where this slot belongs to. */
      struct pool* pool;
      /** Pointer to the next slot in the pool. */
      slot* next;
      /** Pointer to the previous slot in the pool. */
      slot* prev;
      /** Size of the slot. */
      std::size_t size;
      /** Pointer to the allocated memory. */
      char* memory;
    };
#endif
  }
}

#endif /* !PLORTH_MEMORY_HPP_GUARD */

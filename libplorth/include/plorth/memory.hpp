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
  namespace memory
  {
    struct slot;

    /**
     * Memory manager acts as an garbage collector for the interpreter,
     * managing lifespan of slots of allocated memory used to store objects
     * used by the interpreter.
     */
    class manager
    {
    public:
      using allocator_type = std::allocator<char>;

      /**
       * Constructs new memory manager.
       *
       * \param allocator Allocator used for allocating chunks of memory.
       */
      explicit manager(const allocator_type& allocator = allocator_type());

      /**
       * Destructor. Note that when a memory manager is being destroyed, all
       * objects allocated using it are also destroyed and the memory used by
       * them is being freed.
       */
      ~manager();

      /**
       * Returns the allocator used for allocating chunks of memory.
       */
      inline allocator_type& allocator()
      {
        return m_allocator;
      }

      /**
       * Returns the allocator used for allocating chunks of memory.
       */
      inline const allocator_type& allocator() const
      {
        return m_allocator;
      }

      /**
       * Allocates memory for a managed object from memory pools of this memory
       * manager. New memory pools are being created when previous ones are
       * full.
       *
       * \param size Size of the object to allocate memory for.
       * \return     Pointer to the allocated memory.
       */
      slot* allocate(std::size_t size);

      /**
       * Removes given slot from the generation where it currently resides and
       * places it into list of free slots, so that it can be re-used when
       * another slot of same size is being allocated.
       */
      void deallocate(struct slot* slot);

      /**
       * Launches the garbage collection.
       */
      void collect();

      manager(const manager&) = delete;
      manager(manager&&) = delete;
      void operator=(const manager&) = delete;
      void operator=(manager&&) = delete;

    private:
      /** The allocator used for allocating memory. */
      allocator_type m_allocator;
      /** Pointer to first unused slot. */
      slot* m_free_head;
      /** Pointer to last unused slot. */
      slot* m_free_tail;
      /** Pointer to first slot in nursery generation. */
      slot* m_nursery_head;
      /** Pointer to last slot in nursery generation. */
      slot* m_nursery_tail;
      int m_nursery_counter;
      slot* m_tenured_head;
      slot* m_tenured_tail;
      int m_tenured_counter;
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
       * Returns true if this object has been marked as used by the garbage
       * collector, false otherwise.
       */
      inline bool marked() const
      {
        return m_marked;
      }

      /**
       * Marks the object as used. Derived classes should override this method
       * so that it marks all the objects used by the the derived class.
       */
      inline virtual void mark()
      {
        m_marked = true;
      }

      /**
       * Sets the object as unused, if it has been previously marked as used by
       * the garbage collector.
       */
      inline void unmark()
      {
        m_marked = false;
      }

      inline int use_count() const
      {
        return m_use_count;
      }

      inline void inc_use_count()
      {
        ++m_use_count;
      }

      inline void dec_use_count()
      {
        --m_use_count;
      }

      void* operator new(std::size_t size, class manager& manager);
      void operator delete(void* pointer);

      managed(const managed&) = delete;
      managed(managed&&) = delete;
      void operator=(const managed&) = delete;
      void operator=(managed&&) = delete;

    private:
      /** Whether this object has been marked or not. */
      bool m_marked;
      /** How many references are currently being held to this object. */
      int m_use_count;
    };

    /**
     * Represents single slot of allocated memory.
     */
    struct slot
    {
      /** Pointer to the memory manager handling this slot. */
      manager* manager;
      /** Pointer to the next slot in the generation. */
      slot* next;
      /** Pointer to the previous slot in the generation. */
      slot* prev;
      /** Size of this slot. */
      std::size_t size;
      /** Pointer to the allocated memory. */
      managed* object;
    };
  }

  /**
   * Reference of a managed object. Keeps track of the objects usage, so that
   * the garbage collector knows which objects are being used as local
   * variables in the C++ code.
   */
  template<class T>
  class ref
  {
  public:
    using value_type = T;
    using pointer = T*;

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
     * Constructs copy of existing reference.
     */
    template<class U>
    ref(const ref<U>& that)
      : m_object(that.get())
    {
      if (m_object)
      {
        m_object->inc_use_count();
      }
    }

    /**
     * Moves value of existing reference into new one.
     */
    ref(ref<T>&& that)
      : m_object(that.m_object)
    {
      that.m_object = nullptr;
    }

    /**
     * Constructs reference from pointer to the given object.
     */
    ref(pointer object)
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
      if (m_object)
      {
        m_object->dec_use_count();
      }
    }

    /**
     * Copies value from another reference into this one.
     */
    template<class U>
    ref<T>& operator=(const ref<U>& that)
    {
      U* object = that.get();

      if (m_object != object)
      {
        if (m_object)
        {
          m_object->dec_use_count();
        }
        if ((m_object = object))
        {
          m_object->inc_use_count();
        }
      }

      return *this;
    }

    /**
     * Copies value from another reference into this one.
     */
    ref<T>& operator=(const ref<T>& that)
    {
      if (m_object != that.m_object)
      {
        if (m_object)
        {
          m_object->dec_use_count();
        }
        if ((m_object = that.m_object))
        {
          m_object->inc_use_count();
        }
      }

      return *this;
    }

    /**
     * Moves value from another reference into this one.
     */
    ref<T>& operator=(ref<T>&& that)
    {
      if (m_object != that.m_object)
      {
        if (m_object)
        {
          m_object->dec_use_count();
        }
        if ((m_object = that.m_object))
        {
          m_object->inc_use_count();
        }
        that.m_object = nullptr;
      }

      return *this;
    }

    /**
     * Returns pointer to the referenced object.
     */
    inline pointer get() const
    {
      return m_object;
    }

    /**
     * Casts the reference into another type.
     */
    template<class U>
    inline ref<U> cast() const
    {
      return static_cast<U*>(m_object);
    }

    /**
     * Resets the reference to null pointer.
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
     * Returns pointer to the referenced object.
     */
    inline pointer operator->() const
    {
      return m_object;
    }

    /**
     * Boolean coercion operator. Returns true if the reference is non-null,
     * false otherwise.
     */
    inline explicit operator bool() const
    {
      return !!m_object;
    }

    /**
     * Boolean negation operator. Returns true if the reference is null, false
     * otherwise.
     */
    inline bool operator!() const
    {
      return !m_object;
    }

  private:
    /** Pointer to the referenced object. */
    pointer m_object;
  };
}

#endif /* !PLORTH_MEMORY_HPP_GUARD */

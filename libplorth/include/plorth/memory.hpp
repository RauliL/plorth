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
#include <plorth/ref.hpp>

#include <cstddef>
#include <memory>

namespace plorth
{
  class context;
  class runtime;

  namespace memory
  {
#if PLORTH_ENABLE_MEMORY_POOL
    struct pool;
#endif
    struct slot;

    struct generation
    {
      slot* head;
      slot* tail;
    };

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

      void collect();

      manager(const manager&) = delete;
      manager(manager&&) = delete;
      void operator=(const manager&) = delete;
      void operator=(manager&&) = delete;

    private:
      int m_allocation_counter;
      generation m_nursery;
      generation m_tenured;
#if PLORTH_ENABLE_MEMORY_POOL
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
       * Returns true if this object has been marked by the memory manager to
       * still be in use.
       */
      inline bool marked() const
      {
        return m_marked;
      }

      /**
       * Sets the object to be "marked", e.g. meaning that it's still being
       * used somewhere and should not be automatically deleted by the memory
       * manager.
       *
       * Classes which derive from this class should override this method so
       * that it calls this method to all managed objects which it uses.
       */
      inline virtual void mark()
      {
        m_marked = true;
      }

      /**
       * Removes the marked flag from the object.
       */
      inline void unmark()
      {
        m_marked = false;
      }

      /**
       * Returns the number of references which this object currently has.
       */
      inline long use_count() const
      {
        return m_use_count;
      }

      /**
       * Increases the reference counter of the object.
       */
      inline void inc_use_count()
      {
        ++m_use_count;
      }

      /**
       * Decreases the reference counter of the object.
       */
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
      /** Number of references the object currently has. */
      long m_use_count;
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
#endif

    struct slot
    {
      /** Generation where the slot belongs to. */
      struct generation* generation;
      /** Next slot in the generation. */
      slot* next_in_generation;
      /** Previous slot in the generation. */
      slot* prev_in_generation;
#if PLORTH_ENABLE_MEMORY_POOL
      /** Memory pool where this slot belongs to. */
      struct pool* pool;
      /** Pointer to the next slot in the pool. */
      slot* next;
      /** Pointer to the previous slot in the pool. */
      slot* prev;
      /** Size of the slot. */
      std::size_t size;
#endif
      /** Pointer to the allocated memory. */
      char* memory;
    };
  }
}

#endif /* !PLORTH_MEMORY_HPP_GUARD */

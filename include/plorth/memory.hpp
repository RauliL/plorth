/*
 * Copyright (c) 2017, Rauli Laine
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

#include <plorth/ref.hpp>

#include <cstddef>

namespace plorth
{
  namespace memory
  {
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

      /**
       * Creates new scripting runtime that uses this memory manager for memory
       * allocation.
       */
      ref<runtime> new_runtime();

      manager(const manager&) = delete;
      manager(manager&&) = delete;
      void operator=(const manager&) = delete;
      void operator=(manager&&) = delete;

    private:
      /** Pointer to the first memory pool used by this manager. */
      pool* m_pool_head;
      /** Pointer to the last memory pool used by this manager. */
      pool* m_pool_tail;
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
       * Increases the reference counter.
       */
      inline void inc_ref_count()
      {
        ++m_ref_count;
      }

      /**
       * Decreases the reference counter.
       *
       * \return True if there are no more references to the object.
       */
      inline bool dec_ref_count()
      {
        return !--m_ref_count;
      }

      void* operator new(std::size_t size, class manager& manager);
      void operator delete(void* pointer);

      managed(const managed&) = delete;
      managed(managed&&) = delete;
      void operator=(const managed&) = delete;
      void operator=(managed&&) = delete;

    private:
      /** Used to track references to the object. */
      std::size_t m_ref_count;
    };

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
      pool* pool;
      /** Pointer to the next slot in the pool. */
      slot* next;
      /** Pointer to the previous slot in the pool. */
      slot* prev;
      /** Size of the slot. */
      std::size_t size;
      /** Pointer to the allocated memory. */
      char* memory;
    };
  }
}

#endif /* !PLORTH_MEMORY_HPP_GUARD */

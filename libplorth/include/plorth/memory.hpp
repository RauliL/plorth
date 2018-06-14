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

      void* operator new(std::size_t size, class manager& manager);
      void operator delete(void* pointer);

      managed(const managed&) = delete;
      managed(managed&&) = delete;
      void operator=(const managed&) = delete;
      void operator=(managed&&) = delete;
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
      char* memory;
    };
  }
}

#endif /* !PLORTH_MEMORY_HPP_GUARD */

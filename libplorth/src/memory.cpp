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
#include <plorth/context.hpp>
#if !defined(PLORTH_ALLOCATION_THRESHOLD)
# define PLORTH_ALLOCATION_THRESHOLD 1024
#endif
#if PLORTH_ENABLE_MEMORY_POOL
# if !defined(PLORTH_MEMORY_POOL_SIZE)
#  define PLORTH_MEMORY_POOL_SIZE (4096 * 32)
# endif
#endif

namespace plorth
{
  namespace memory
  {
    static void destroy_generation(generation&);
    static void add_slot_to_generation(slot*, generation&);
    static void remove_slot_from_generation(slot*);
    static void generation_mark(generation&);
    static void generation_unmark(generation&);
    static void generation_sweep(generation&, generation&);
#if PLORTH_ENABLE_MEMORY_POOL
    static pool* pool_create();
    static slot* pool_allocate(pool*, std::size_t);
#endif

    manager::manager()
      : m_allocation_counter(0)
#if PLORTH_ENABLE_MEMORY_POOL
      , m_pool_head(nullptr)
      , m_pool_tail(nullptr)
#endif
    {
      m_nursery.head = nullptr;
      m_nursery.tail = nullptr;
      m_tenured.head = nullptr;
      m_tenured.tail = nullptr;
    }

    manager::~manager()
    {
#if PLORTH_ENABLE_MEMORY_POOL
      pool* current;
      pool* prev;
#endif

      destroy_generation(m_nursery);
      destroy_generation(m_tenured);
#if PLORTH_ENABLE_MEMORY_POOL
      for (current = m_pool_tail; current; current = prev)
      {
        prev = current->prev;
        for (struct slot* slot = current->used_head; slot; slot = slot->next)
        {
          delete reinterpret_cast<managed*>(slot->memory);
        }
        std::free(static_cast<void*>(current));
      }
#endif
    }

    void* manager::allocate(std::size_t size)
    {
      struct slot* slot;

      if (++m_allocation_counter >= PLORTH_ALLOCATION_THRESHOLD)
      {
        m_allocation_counter = 0;
        collect();
      }

#if PLORTH_ENABLE_MEMORY_POOL
      const std::size_t remainder = size % 8;
      struct pool* pool;

      if (remainder)
      {
        size += 8 - remainder;
      }

      // First go through existing memory pools and check whether we can slice
      // a slot from any of them.
      for (pool = m_pool_tail; pool; pool = pool->prev)
      {
        if ((slot = pool_allocate(pool, size)))
        {
          return static_cast<void*>(slot->memory);
        }
      }

      // If all existing pools are full, create a new one. If that one fails,
      // abort the entire process as it's a signal that we are out of memory.
      if (!(pool = pool_create()))
      {
        std::abort();
      }

# if defined(PLORTH_ENABLE_GC_DEBUG)
      std::fprintf(stderr, "GC: Memory pool allocated.\n");
# endif

      // Place the newly created pool into linked list of memory pools.
      if ((pool->prev = m_pool_tail))
      {
        m_pool_tail->next = pool;
      } else {
        m_pool_head = pool;
      }
      m_pool_tail = pool;

      // Try to allocate slot from the freshly created memory pool. If even
      // that is not possible, crash and burn as something is seriously wrong
      // now.
      if (!(slot = pool_allocate(pool, size)))
      {
        std::abort();
      }
#else
      void* pointer = std::malloc(size);

      if (!pointer)
      {
        std::abort();
      }
      slot = static_cast<struct slot*>(pointer);
      slot->memory = static_cast<char*>(pointer) + sizeof(struct slot);
#endif

      add_slot_to_generation(slot, m_nursery);

      return static_cast<void*>(slot->memory);
    }

    void manager::collect()
    {
      generation_mark(m_nursery);
      generation_sweep(m_nursery, m_tenured);
      generation_unmark(m_tenured);
    }

    managed::managed()
      : m_reference_count(0)
      , m_marked(false) {}

    managed::~managed() {}

    void managed::mark()
    {
      m_marked = true;
    }

    void managed::unmark()
    {
      m_marked = false;
    }

    void* managed::operator new(std::size_t size, class manager& manager)
    {
      return manager.allocate(size);
    }

    void managed::operator delete(void* pointer)
    {
      struct slot* slot;

      if (!pointer)
      {
        return;
      }

      slot = reinterpret_cast<struct slot*>(
        static_cast<char*>(pointer) - sizeof(struct slot)
      );

      remove_slot_from_generation(slot);

#if PLORTH_ENABLE_MEMORY_POOL
      auto pool = slot->pool;

      // Remove the slot from the linked of list of used slots in the pool.
      if (slot->next && slot->prev)
      {
        slot->next->prev = slot->prev;
        slot->prev->next = slot->next;
      }
      else if (slot->next)
      {
        slot->next->prev = nullptr;
        pool->used_head = slot->next;
      }
      else if (slot->prev)
      {
        slot->prev->next = nullptr;
        pool->used_tail = slot->prev;
      } else {
        pool->used_head = nullptr;
        pool->used_tail = nullptr;
      }

      // Then place the slot into linked of list of free slots in the pool.
      slot->next = nullptr;
      if ((slot->prev = slot->pool->free_tail))
      {
        pool->free_tail->next = slot;
      } else {
        pool->free_head = slot;
      }
      pool->free_tail = slot;

      // Remove the pool if it's no longer used.
      if (pool->next && pool->prev && !pool->used_head && !pool->used_tail)
      {
        pool->next->prev = pool->prev;
        pool->prev->next = pool->next;
# if defined(PLORTH_ENABLE_GC_DEBUG)
        std::fprintf(stderr, "GC: Memory pool removed.\n");
# endif
        std::free(static_cast<void*>(pool));
      }
#else
      std::free(pointer);
#endif
    }

    static void destroy_generation(struct generation& generation)
    {
      slot* next;

      for (auto slot = generation.head; slot; slot = next)
      {
        next = slot->next_in_generation;
        delete reinterpret_cast<managed*>(slot->memory);
      }
    }

    static void add_slot_to_generation(struct slot* slot,
                                       struct generation& generation)
    {
      slot->generation = &generation;
      slot->next_in_generation = nullptr;
      if ((slot->prev_in_generation = generation.tail))
      {
        generation.tail->next_in_generation = slot;
      } else {
        generation.head = slot;
      }
      generation.tail = slot;
    }

    static void remove_slot_from_generation(struct slot* slot)
    {
      auto generation = slot->generation;

      if (!generation)
      {
        return;
      }
      if (slot->next_in_generation && slot->prev_in_generation)
      {
        slot->next_in_generation->prev_in_generation =
          slot->prev_in_generation;
        slot->prev_in_generation->next_in_generation =
          slot->next_in_generation;
      }
      else if (slot->next_in_generation)
      {
        slot->next_in_generation->prev_in_generation = nullptr;
        generation->head = slot->next_in_generation;
      }
      else if (slot->prev_in_generation)
      {
        slot->prev_in_generation->next_in_generation = nullptr;
        generation->tail = slot->prev_in_generation;
      } else {
        generation->head = nullptr;
        generation->tail = nullptr;
      }
    }

    static void generation_mark(struct generation& generation)
    {
      for (auto slot = generation.head; slot; slot = slot->next_in_generation)
      {
        auto object = reinterpret_cast<managed*>(slot->memory);

        if (!object->marked() && object->reference_count() > 0)
        {
          object->mark();
        }
      }
    }

    static void generation_unmark(struct generation& generation)
    {
      for (auto slot = generation.head; slot; slot = slot->next_in_generation)
      {
        auto object = reinterpret_cast<managed*>(slot->memory);

        if (object->marked())
        {
          object->unmark();
        }
      }
    }

    static void generation_sweep(generation& young, generation& old)
    {
      slot* next;
      slot* saved_head = nullptr;
      slot* saved_tail = nullptr;
#if PLORTH_ENABLE_GC_DEBUG
      int saved_count = 0;
      int trashed_count = 0;
#endif

      for (auto slot = young.head; slot; slot = next)
      {
        auto object = reinterpret_cast<managed*>(slot->memory);

        next = slot->next_in_generation;
        if (object->marked())
        {
          slot->next_in_generation = nullptr;
          if ((slot->prev_in_generation = saved_tail))
          {
            saved_tail->next_in_generation = slot;
          } else {
            saved_head = slot;
          }
          saved_tail = slot;
#if PLORTH_ENABLE_GC_DEBUG
          ++saved_count;
#endif
        } else {
          delete object;
#if PLORTH_ENABLE_GC_DEBUG
          ++trashed_count;
#endif
        }
      }

      if (saved_head)
      {
        if ((saved_head->prev_in_generation = old.tail))
        {
          old.tail->next_in_generation = saved_head;
        }
        old.tail = saved_tail;
      }

#if PLORTH_ENABLE_GC_DEBUG
      std::fprintf(
        stderr,
        "GC: Sweep done; %d trashed, %d saved.\n",
        trashed_count,
        saved_count
      );
#endif
    }

#if PLORTH_ENABLE_MEMORY_POOL
    static pool* pool_create()
    {
      char* memory = static_cast<char*>(std::malloc(sizeof(struct pool) + PLORTH_MEMORY_POOL_SIZE));
      struct pool* pool;

      if (!memory)
      {
        return nullptr;
      }

      pool = reinterpret_cast<struct pool*>(memory);
      pool->next = nullptr;
      pool->prev = nullptr;
      pool->remaining = PLORTH_MEMORY_POOL_SIZE;
      pool->memory = memory + sizeof(struct pool);
      pool->free_head = nullptr;
      pool->free_tail = nullptr;
      pool->used_head = nullptr;
      pool->used_tail = nullptr;

      return pool;
    }

    static slot* pool_allocate(struct pool* pool, std::size_t size)
    {
      struct slot* slot;
      char* memory;

      for (slot = pool->free_head; slot; slot = slot->next)
      {
        if (slot->size < size)
        {
          continue;
        }

        if (slot->next && slot->prev)
        {
          slot->next->prev = slot->prev;
          slot->prev->next = slot->next;
        }
        else if (slot->next)
        {
          slot->next->prev = nullptr;
          pool->free_head = slot->next;
        }
        else if (slot->prev)
        {
          slot->prev->next = nullptr;
          pool->free_tail = slot->prev;
        } else {
          pool->free_head = nullptr;
          pool->free_tail = nullptr;
        }

        slot->next = nullptr;
        if ((slot->prev = pool->used_tail))
        {
          slot->prev->next = slot;
        } else {
          pool->used_head = slot;
        }
        pool->used_tail = slot;

        return slot;
      }

      if (pool->remaining < size + sizeof(struct slot))
      {
        return nullptr;
      }

      memory = pool->memory + (PLORTH_MEMORY_POOL_SIZE - pool->remaining);
      pool->remaining -= size + sizeof(struct slot);

      slot = reinterpret_cast<struct slot*>(memory);
      slot->pool = pool;
      slot->next = nullptr;
      if ((slot->prev = pool->used_tail))
      {
        slot->prev->next = slot;
      } else {
        pool->used_head = slot;
      }
      pool->used_tail = slot;
      slot->size = size;
      slot->memory = memory + sizeof(struct slot);

      return slot;
    }
#endif /* PLORTH_ENABLE_MEMORY_POOL */
  }
}

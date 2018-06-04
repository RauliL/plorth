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
#if PLORTH_ENABLE_MEMORY_POOL
# if !defined(PLORTH_MEMORY_POOL_SIZE)
#  define PLORTH_MEMORY_POOL_SIZE (4096 * 32)
# endif
#endif

namespace plorth
{
  namespace memory
  {
#if PLORTH_ENABLE_MEMORY_POOL
    static pool* pool_create();
    static slot* pool_allocate(pool*, std::size_t);
#endif

    manager::manager()
#if PLORTH_ENABLE_MEMORY_POOL
      : m_pool_head(nullptr)
      , m_pool_tail(nullptr)
#endif
      {}

    manager::~manager()
    {
#if PLORTH_ENABLE_MEMORY_POOL
      pool* current;
      pool* prev;

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
#if PLORTH_ENABLE_MEMORY_POOL
      const std::size_t remainder = size % 8;
      struct pool* pool;
      struct slot* slot;

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

      return static_cast<void*>(slot->memory);
#else
      return std::malloc(size);
#endif
    }

    managed::managed() {}

    managed::~managed() {}

    void* managed::operator new(std::size_t size, class manager& manager)
    {
      return manager.allocate(size);
    }

    void managed::operator delete(void* pointer)
    {
#if PLORTH_ENABLE_MEMORY_POOL
      struct slot* slot;
      struct pool* pool;

      if (!pointer)
      {
        return;
      }

      slot = reinterpret_cast<struct slot*>(static_cast<char*>(pointer) - sizeof(struct slot));
      pool = slot->pool;

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
      if (pointer)
      {
        std::free(pointer);
      }
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

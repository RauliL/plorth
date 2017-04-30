#include "./config.hpp"

#include <cassert>

#include <plorth/plorth-runtime.hpp>

namespace plorth
{
  /** Size of single garbage collector pool. */
  static const std::size_t kPoolSize = 4096 * 32;

  struct MemoryPool
  {
    std::size_t index;
    /** Pointer to next pool in memory manager. */
    MemoryPool* next;
    /** Pointer to previous pool in memory manager. */
    MemoryPool* prev;
    /** Amount of bytes still available in this pool. */
    std::size_t remaining;
    /** Pointer to data allocated by the pool. */
    char* data;
    /** Pointer to the first free slot in the pool. */
    MemorySlot* free_head;
    /** Pointer to the last free slot in the pool. */
    MemorySlot* free_tail;
    /** Pointer to the first used slot in the pool. */
    MemorySlot* used_head;
    /** Pointer to the last used slot in the pool. */
    MemorySlot* used_tail;
  };

  struct MemorySlot
  {
    /** Pointer to the memory pool where this slot belongs to. */
    MemoryPool* pool;
    /** Pointer to next slot allocated by the pool. */
    MemorySlot* next;
    /** Pointer to previous slot allocated by the pool. */
    MemorySlot* prev;
    /** Size of the slot. */
    std::size_t size;
    /** Pointer to the allocated data. */
    char* data;
  };

  static MemoryPool* pool_create(std::size_t);
  static MemorySlot* pool_allocate(MemoryPool*, std::size_t);

  ManagedObject::ManagedObject()
    : m_reference_count(0) {}

  ManagedObject::~ManagedObject() {}

  void ManagedObject::DecReferenceCount()
  {
    if (!--m_reference_count)
    {
      delete this;
    }
  }

  void* ManagedObject::operator new(std::size_t size, MemoryManager* memory_manager)
  {
    return static_cast<void*>(memory_manager->Allocate(size)->data);
  }

  void* ManagedObject::operator new(std::size_t size, const Ref<Runtime>& runtime)
  {
    return static_cast<void*>(runtime->GetMemoryManager()->Allocate(size)->data);
  }

  void ManagedObject::operator delete(void* pointer)
  {
    MemorySlot* slot = reinterpret_cast<MemorySlot*>(
      static_cast<char*>(pointer) - sizeof(MemorySlot)
    );
    MemoryPool* pool = slot->pool;

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
#if defined(PLORTH_GC_DEBUG)
      std::fprintf(stderr, "GC: Pool removed: %ld\n", pool->index);
#endif
      std::free(static_cast<void*>(pool));
    }
  }

  MemoryManager::MemoryManager()
    : m_next_pool_index(0)
    , m_pool_head(nullptr)
    , m_pool_tail(nullptr) {}

  MemoryManager::~MemoryManager()
  {
    MemoryPool* current;
    MemoryPool* next;

    for (current = m_pool_head; current; current = next)
    {
      next = current->next;
      for (MemorySlot* slot = current->used_head; slot; slot = slot->next)
      {
        delete (ManagedObject*) slot->data;
      }
      std::free(static_cast<void*>(current));
    }
  }

  Ref<Runtime> MemoryManager::CreateRuntime()
  {
    return new (this) Runtime(this);
  }

  MemorySlot* MemoryManager::Allocate(std::size_t size)
  {
    const std::size_t remainder = size % 8;
    MemoryPool* pool;
    MemorySlot* slot;

    if (remainder)
    {
      size += 8 - remainder;
    }

    for (pool = m_pool_tail; pool; pool = pool->prev)
    {
      if ((slot = pool_allocate(pool, size)))
      {
        return slot;
      }
    }

    pool = pool_create(m_next_pool_index++);
    if ((pool->prev = m_pool_tail))
    {
      m_pool_tail->next = pool;
    } else {
      m_pool_head = pool;
    }
    m_pool_tail = pool;

#if defined(PLORTH_GC_DEBUG)
    std::fprintf(stderr, "GC: Allocated pool: %ld\n", pool->index);
#endif

    if (!(slot = pool_allocate(pool, size)))
    {
      std::abort();
    }

    return slot;
  }

  static MemoryPool* pool_create(std::size_t index)
  {
    char* data = static_cast<char*>(std::malloc(sizeof(MemoryPool) + kPoolSize));
    MemoryPool* pool;

    if (!data)
    {
      std::abort();
    }
    pool = reinterpret_cast<MemoryPool*>(data);
    pool->index = index;
    pool->next = nullptr;
    pool->prev = nullptr;
    pool->remaining = kPoolSize;
    pool->data = data + sizeof(MemoryPool);
    pool->free_head = nullptr;
    pool->free_tail = nullptr;
    pool->used_head = nullptr;
    pool->used_tail = nullptr;

    return pool;
  }

  static MemorySlot* pool_allocate(MemoryPool* pool, std::size_t size)
  {
    MemorySlot* slot;
    char* data;

    for (slot = pool->free_head; slot; slot = slot->next)
    {
      if (slot->size < size)
      {
        continue;
      }
      else if (slot->next && slot->prev)
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

    if (pool->remaining < sizeof(MemorySlot) + size)
    {
      return nullptr;
    }

    data = pool->data + (kPoolSize - pool->remaining);
    pool->remaining -= size + sizeof(MemorySlot);
    slot = (MemorySlot*) data;
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
    slot->data = data + sizeof(MemorySlot);

    return slot;
  }
}

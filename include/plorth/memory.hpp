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

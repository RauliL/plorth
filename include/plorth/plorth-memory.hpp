#ifndef PLORTH_MEMORY_HPP_GUARD
#define PLORTH_MEMORY_HPP_GUARD

#include <plorth/plorth.hpp>

namespace plorth
{
  /**
   * Container class for objects managed by the garbage collector. They
   * automatically increase and decrease usage counter of the underlying
   * object.
   */
  template< class T >
  class Ref
  {
  public:
    /**
     * Constructs null reference.
     */
    explicit Ref()
      : m_object(nullptr) {}

    /**
     * Constructs copy of existing reference.
     */
    Ref(const Ref<T>& that)
      : m_object(that.m_object)
    {
      if (m_object)
      {
        m_object->IncReferenceCount();
      }
    }

    /**
     * Constructs copy of existing reference of different type.
     */
    template< class U >
    Ref(const Ref<U>& that)
      : m_object(that.Get())
    {
      if (m_object)
      {
        m_object->IncReferenceCount();
      }
    }

    template< class U >
    Ref(U* object)
      : m_object(object)
    {
      if (m_object)
      {
        m_object->IncReferenceCount();
      }
    }

    virtual ~Ref()
    {
      if (m_object)
      {
        m_object->DecReferenceCount();
      }
    }

    Ref& operator=(const Ref<T>& that)
    {
      if (m_object != that.m_object)
      {
        if (m_object)
        {
          m_object->DecReferenceCount();
        }
        if ((m_object = that.m_object))
        {
          m_object->IncReferenceCount();
        }
      }

      return *this;
    }

    template< class U >
    Ref& operator=(const Ref<U>& that)
    {
      U* object = that.Get();

      if (m_object != object)
      {
        if (m_object)
        {
          m_object->DecReferenceCount();
        }
        if ((m_object = object))
        {
          m_object->IncReferenceCount();
        }
      }

      return *this;
    }

    /**
     * Returns pointer to the underlying object.
     */
    inline T* Get() const
    {
      return m_object;
    }

    /**
     * Casts underlying object into different type using static cast.
     */
    template< class U >
    inline Ref<U> As() const
    {
      return static_cast<U*>(m_object);
    }

    /**
     * Releases reference to the underlying object. After this has been called,
     * the reference will become a null reference.
     */
    inline void Clear()
    {
      if (m_object)
      {
        m_object->DecReferenceCount();
        m_object = nullptr;
      }
    }

    /**
     * Returns pointer to the underlying object.
     */
    inline T* operator->() const
    {
      return m_object;
    }

    /**
     * Boolean coercion of the underlying object. Null references coerce into
     * false, non-null into true.
     */
    inline operator bool() const
    {
      return !!m_object;
    }

    /**
     * Negated boolean coercion of the underlying object. Null reference coerce
     * into true, while non-null references into false.
     */
    inline bool operator!() const
    {
      return !m_object;
    }

  private:
    /** Pointer to the referenced object. */
    T* m_object;
  };

  /**
   * Base class for all objects which are allocated using the memory manager.
   */
  class ManagedObject
  {
  public:
    explicit ManagedObject();

    virtual ~ManagedObject();

    inline void IncReferenceCount()
    {
      ++m_reference_count;
    }

    void DecReferenceCount();

    void* operator new(std::size_t size, MemoryManager* memory_manager);
    void* operator new(std::size_t size, const Ref<Runtime>& runtime);

    void operator delete(void* pointer);

    ManagedObject(ManagedObject&) = delete;
    void operator=(ManagedObject&) = delete;

  private:
    int m_reference_count;
  };

  class MemoryManager
  {
  public:
    explicit MemoryManager();

    ~MemoryManager();

    Ref<Runtime> CreateRuntime();

    MemorySlot* Allocate(std::size_t size);

    MemoryManager(const MemoryManager&) = delete;
    void operator=(const MemoryManager&) = delete;

  private:
    /** Index of the next memory pool. */
    std::size_t m_next_pool_index;
    /** Pointer to the first pool allocated by this memory manager. */
    MemoryPool* m_pool_head;
    /** Pointer to the last pool allocated by this memory manager. */
    MemoryPool* m_pool_tail;
  };
}

#endif /* !PLORTH_MEMORY_HPP_GUARD */

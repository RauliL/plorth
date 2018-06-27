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

namespace plorth
{
  namespace memory
  {
    static void add_slot_to_generation(slot*, slot**, slot**);
    static void remove_slot_from_generation(slot*, slot**, slot**);
    static inline void deallocate_slot(manager::allocator_type&, slot*);
    static void mark_generation(slot*);
    static void unmark_generation(slot*);
    static void sweep_generation(manager::allocator_type&, slot**, slot**);

    manager::manager(const allocator_type& allocator)
      : m_allocator(allocator)
      , m_free_head(nullptr)
      , m_free_tail(nullptr)
      , m_nursery_head(nullptr)
      , m_nursery_tail(nullptr)
      , m_nursery_counter(0)
      , m_tenured_head(nullptr)
      , m_tenured_tail(nullptr)
      , m_tenured_counter(0) {}

    manager::~manager()
    {
      slot* current;
      slot* next;

      for (current = m_free_head; current; current = next)
      {
        next = current->next;
        deallocate_slot(m_allocator, current);
      }
      for (current = m_nursery_head; current; current = next)
      {
        next = current->next;
        delete current->object;
        deallocate_slot(m_allocator, current);
      }
      for (current = m_tenured_head; current; current = next)
      {
        next = current->next;
        delete current->object;
        deallocate_slot(m_allocator, current);
      }
    }

    slot* manager::allocate(std::size_t size)
    {
      const std::size_t remainder = size % 8;
      struct slot* slot;

      if (remainder)
      {
        size += 8 - remainder;
      }

      // First go through the unused slots and see if any of them is large
      // enough to hold memory for the object being allocated.
      for (slot = m_free_head; slot; slot = slot->next)
      {
        // Too small? Continue to next slot.
        if (slot->size < size)
        {
          continue;
        }

        // Otherwise remove the slot from the list of free slots and place it
        // into nursery instead. Afterwards, return the slot so it can be used.
        remove_slot_from_generation(slot, &m_free_head, &m_free_tail);
        add_slot_to_generation(slot, &m_nursery_head, &m_nursery_tail);

        return slot;
      }

      if (++m_nursery_counter >= 1024)
      {
        m_nursery_counter = 0;
        collect();
      }

      for (slot = m_free_head; slot; slot = slot->next)
      {
        // Too small? Continue to next slot.
        if (slot->size < size)
        {
          continue;
        }

        // Otherwise remove the slot from the list of free slots and place it
        // into nursery instead. Afterwards, return the slot so it can be used.
        remove_slot_from_generation(slot, &m_free_head, &m_free_tail);
        add_slot_to_generation(slot, &m_nursery_head, &m_nursery_tail);

        return slot;
      }

      // No free slots are available, so allocate memory for new one.
      slot = static_cast<struct slot*>(
        static_cast<void*>(
          m_allocator.allocate(sizeof(struct slot) + size)
        )
      );

      slot->manager = this;
      slot->size = size;
      slot->object = static_cast<managed*>(
        static_cast<void*>(
          static_cast<char*>(static_cast<void*>(slot)) + sizeof(struct slot)
        )
      );

      add_slot_to_generation(slot, &m_nursery_head, &m_nursery_tail);

      return slot;
    }

    void manager::deallocate(struct slot* slot)
    {
      remove_slot_from_generation(slot, &m_nursery_head, &m_nursery_tail);
      add_slot_to_generation(slot, &m_free_head, &m_free_tail);
    }

    void manager::collect()
    {
      mark_generation(m_nursery_head);
      sweep_generation(m_allocator, &m_nursery_head, &m_nursery_tail);
      if (++m_tenured_counter < 16)
      {
        unmark_generation(m_nursery_head);
        // TODO: Perform move.
        return;
      }
      m_tenured_counter = 0;
      mark_generation(m_tenured_head);
      sweep_generation(m_allocator, &m_tenured_head, &m_tenured_tail);
      // TODO: Perform move.
      unmark_generation(m_nursery_head);
      unmark_generation(m_tenured_head);
    }

    managed::managed()
      : m_use_count(0) {}

    managed::~managed() {}

    void* managed::operator new(std::size_t size, class manager& manager)
    {
      return static_cast<void*>(manager.allocate(size)->object);
    }

    void managed::operator delete(void* pointer) {}

    static void add_slot_to_generation(struct slot* slot,
                                       struct slot** head,
                                       struct slot** tail)
    {
      slot->next = nullptr;
      if ((slot->prev = *tail))
      {
        slot->prev->next = slot;
      } else {
        *head = slot;
      }
      *tail = slot;
    }

    static void remove_slot_from_generation(struct slot* slot,
                                            struct slot** head,
                                            struct slot** tail)
    {
      if (slot->next && slot->prev)
      {
        slot->next->prev = slot->prev;
        slot->prev->next = slot->next;
      }
      else if (slot->next)
      {
        slot->next->prev = nullptr;
        *head = slot->next;
      }
      else if (slot->prev)
      {
        slot->prev->next = nullptr;
        *tail = slot->prev;
      } else {
        *head = nullptr;
        *tail = nullptr;
      }

      slot->next = nullptr;
      slot->prev = nullptr;
    }

    static inline void deallocate_slot(manager::allocator_type& allocator,
                                       struct slot* slot)
    {
      allocator.deallocate(
        static_cast<char*>(static_cast<void*>(slot)),
        sizeof(struct slot) + slot->size
      );
    }

    static void mark_generation(struct slot* slot)
    {
      for (; slot; slot = slot->next)
      {
        if (!slot->object->marked() && slot->object->use_count() > 0)
        {
          slot->object->mark();
        }
      }
    }

    static void unmark_generation(struct slot* slot)
    {
      for (; slot; slot = slot->next)
      {
        slot->object->unmark();
      }
    }

    static void sweep_generation(manager::allocator_type& allocator,
                                 slot** head,
                                 slot** tail)
    {
      slot* current;
      slot* next;
      slot* new_head = nullptr;
      slot* new_tail = nullptr;

      for (current = *head; current; current = next)
      {
        next = current->next;
        if (current->object->marked())
        {
          current->object->unmark();
          add_slot_to_generation(current, &new_head, &new_tail);
        } else {
          delete current->object;
          deallocate_slot(allocator, current);
        }
      }

      *head = new_head;
      *tail = new_tail;
    }
  }
}

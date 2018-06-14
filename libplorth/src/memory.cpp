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

    manager::manager(const allocator_type& allocator)
      : m_allocator(allocator)
      , m_free_head(nullptr)
      , m_free_tail(nullptr)
      , m_nursery_head(nullptr)
      , m_nursery_tail(nullptr) {}

    manager::~manager()
    {
      slot* current;
      slot* next;

      for (current = m_free_head; current; current = next)
      {
        next = current->next;
        m_allocator.deallocate(
          static_cast<char*>(static_cast<void*>(current)),
          sizeof(slot) + current->size
        );
      }
      for (current = m_nursery_head; current; current = next)
      {
        next = current->next;
        delete reinterpret_cast<managed*>(current->memory);
        m_allocator.deallocate(
          static_cast<char*>(static_cast<void*>(current)),
          sizeof(slot) + current->size
        );
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

      // No free slots are available, so allocate memory for new one.
      slot = static_cast<struct slot*>(
        static_cast<void*>(
          m_allocator.allocate(sizeof(struct slot) + size)
        )
      );

      slot->manager = this;
      slot->size = size;
      slot->memory = (
        static_cast<char*>(static_cast<void*>(slot)) + sizeof(struct slot)
      );

      add_slot_to_generation(slot, &m_nursery_head, &m_nursery_tail);

      return slot;
    }

    void manager::deallocate(struct slot* slot)
    {
      remove_slot_from_generation(slot, &m_nursery_head, &m_nursery_tail);
      add_slot_to_generation(slot, &m_free_head, &m_free_tail);
    }

    managed::managed()
      : m_use_count(0) {}

    managed::~managed() {}

    void* managed::operator new(std::size_t size, class manager& manager)
    {
      return static_cast<void*>(manager.allocate(size)->memory);
    }

    void managed::operator delete(void* pointer)
    {
      auto slot = static_cast<struct slot*>(
        static_cast<void*>(
          static_cast<char*>(pointer) - sizeof(struct slot)
        )
      );

      slot->manager->deallocate(slot);
    }

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
  }
}

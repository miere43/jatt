#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#include <QtGlobal>

/**
 * Represents crappy block allocator.
 */
template <typename T>
class BlockAllocator
{
public:
    // set this const to amount of bits of 'usedSlots' type.
    static const int BLOCK_MAX_ITEMS = 32; // 32 = sizeof(qint32) * 8 bits

    struct Block {
        quint32 usedSlots; // if you change type, make sure everything works
        T      items[BLOCK_MAX_ITEMS];
        Block* next;

        T* set(int slot)
        {
            Q_ASSERT(slot >= 0 && slot < BLOCK_MAX_ITEMS);
            Q_ASSERT(false == (usedSlots & (1 << slot)));
            usedSlots = usedSlots | (1 << slot);
            return &items[slot];
        }

        void unset(T* value)
        {
            const size_t itemsOffset = offsetof(Block, items);
            uintptr_t slot = ((uintptr_t)value - ((uintptr_t)this + (uintptr_t)itemsOffset)) / (uintptr_t)sizeof(T);
            Q_ASSERT(slot >= 0 && slot <= BLOCK_MAX_ITEMS);
            usedSlots = usedSlots & (~(1 << slot));
#ifdef QT_DEBUG
            memset(value, 0xCD, sizeof(T));
#endif
        }

        int getFreeSlotIndex()
        {
            // INT32_MAX doesn't work for some reason :/
            if (usedSlots == 0xFFFFFFFF)
            {
                return -1;
            }

            for (int i = 0; i < BLOCK_MAX_ITEMS; ++i)
            {
                int bitValue = usedSlots & (1 << i);
                if (bitValue == false)
                {
                    return i;
                }
            }
            Q_ASSERT(false);
            return -1;
        }
    };

    int blocksAllocated = 0;
    int totalSlotsUsed = 0;
    Block* first = nullptr;
    Block* last = nullptr;

    Block* pushNewBlock() {
        Block* block = new Block();
        // @TODO: Handle out of memory condition.
        if (block == nullptr)
        {
            return nullptr;
        }
        block->usedSlots = 0;
        block->next = nullptr;
        ++blocksAllocated;
        if (last != nullptr)
        {
            last->next = block;
            last = block;
        }
        return block;
    }

    T* allocate() {
        Block* block;
        if (totalSlotsUsed == blocksAllocated * BLOCK_MAX_ITEMS)
        {
            block = pushNewBlock();
        }
        else
        {
            block = first;
        }

        getSlot:
        int freeSlot = block->getFreeSlotIndex();
        if (freeSlot == -1)
        {
            if (block->next != nullptr)
            {
                block = block->next;
                goto getSlot;
            }
            else
            {
                block = pushNewBlock();
                freeSlot = 0;
            }
        }

        T* value = block->set(freeSlot);
        // @TODO call constructor.
        // new(value) T();
        ++totalSlotsUsed;
        return value;
    }

    void deallocate(T* value)
    {
        Q_ASSERT(first != nullptr);
        Block* block = first;
        do
        {
            if ((void*)value >= (void*)block && (void*)value <= (void*)((char*)block + sizeof(Block)))
            {
                // @TODO call destructor.
                //value->~T();
                block->unset(value);
                return;
            }
        } while (block = block->next);
        Q_ASSERT(false); // value was not allocated by this allocator!
    }

    int itemsAllocated() const
    {
        return totalSlotsUsed;
    }

    BlockAllocator() {
        first = pushNewBlock();
        last = first;
    }

    ~BlockAllocator() {
        Block* block = 0;
    }
};

#endif // BLOCK_ALLOCATOR_H

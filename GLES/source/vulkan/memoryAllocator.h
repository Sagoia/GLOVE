#ifndef __VKMEMORYALLOCATOR_H__
#define __VKMEMORYALLOCATOR_H__

#include "vulkan/vulkan.h"
#include "utils/glLogger.h"
#include <queue>
#include <vector>
#include <map>

namespace vulkanAPI {

struct MemoryBlock {
    MemoryBlock():chunkId(0), offset(0), vkMemory(VK_NULL_HANDLE) { }
    uint64_t                        chunkId;
    VkDeviceSize                    offset;
    VkDeviceMemory                  vkMemory;
};

class MemoryAllocator {

private:
    struct Chunk {
        VkDeviceMemory              vkMemory;
        std::queue<VkDeviceSize>    freeBlocks;
    };

    struct ChunkList {
        VkDeviceSize                size;
        VkDeviceSize                alignment;
        uint32_t                    memoryTypeIndex;
        std::vector<Chunk *>        chunks;
    };

    const static VkDeviceSize       BASE_ALIGNMENT = 256;
    const static VkDeviceSize       MAX_BLOCK_SIZE = 4096;
    const static uint32_t           BLOCK_COUNT = 64;

    VkDevice                        mVkDevice;
    std::map<uint64_t, ChunkList *> mChunkLists;

    void                            FillBlock(MemoryBlock &block, Chunk &chunk);

    Chunk *                         CreateChunk(VkDeviceSize size, uint32_t memoryTypeIndex);

    void                            CleanUp(void);

public:
// Constructor
    MemoryAllocator(VkDevice vkDevice);

// Destructor
    ~MemoryAllocator();

    inline bool                     CanAllocate(VkDeviceSize size)  { FUN_ENTRY(GL_LOG_TRACE); return size < MAX_BLOCK_SIZE; }

    void                            Allocate(VkDeviceSize size, VkDeviceSize alignment, uint32_t memoryTypeIndex, MemoryBlock &block);

    bool                            Deallocate(MemoryBlock &block);

};

}

#endif // __VKMEMORYALLOCATOR_H__

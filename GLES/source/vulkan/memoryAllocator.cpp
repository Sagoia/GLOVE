#include "memoryAllocator.h"

namespace vulkanAPI {

MemoryAllocator::MemoryAllocator(VkDevice vkDevice)
: mVkDevice(vkDevice)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

MemoryAllocator::~MemoryAllocator()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUp();
}

MemoryAllocator::Chunk *
MemoryAllocator::CreateChunk(VkDeviceSize size, uint32_t memoryTypeIndex)
{
    FUN_ENTRY(GL_LOG_TRACE);

    Chunk *chunk = new Chunk;

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = BLOCK_COUNT * size;

    VkResult err = vkAllocateMemory(mVkDevice, &allocInfo, nullptr, &(chunk->vkMemory));
    assert(!err);

    if (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_TOO_MANY_OBJECTS) {
        for (uint32_t i = 0; i < BLOCK_COUNT; ++i) {
            chunk->freeBlocks.push(i * size);
        }
    } else {
        delete chunk;
        chunk = nullptr;
    }

    return chunk;
}

void
MemoryAllocator::FillBlock(MemoryBlock &block, MemoryAllocator::Chunk &chunk)
{
    FUN_ENTRY(GL_LOG_TRACE);

    VkDeviceSize offset = chunk.freeBlocks.front();
    chunk.freeBlocks.pop();

    block.vkMemory = chunk.vkMemory;
    block.offset = offset;
}

void
MemoryAllocator::CleanUp(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    for (auto chunkListPair : mChunkLists) {
        ChunkList *chunkList = chunkListPair.second;
        for (auto chunk : chunkList->chunks) {
            vkFreeMemory(mVkDevice, chunk->vkMemory, nullptr);
            delete chunk;
        }
        chunkList->chunks.clear();
        delete chunkList;
    }
    mChunkLists.clear();
}

void
MemoryAllocator::Allocate(VkDeviceSize size, VkDeviceSize alignment, uint32_t memoryTypeIndex, MemoryBlock &block)
{
    FUN_ENTRY(GL_LOG_TRACE);

    // detect alignment is POT
    assert(alignment > 0 && (alignment & (alignment - 1)) == 0);
    size = (size + (alignment - 1)) & ~(alignment - 1);

    uint64_t chunkId = (size << 32) + (alignment << 16) + memoryTypeIndex;

    ChunkList *chunkList = nullptr;
    std::map<uint64_t, ChunkList *>::iterator it = mChunkLists.find(chunkId);
    if (it != mChunkLists.end()) {
        chunkList = it->second;
    } else {
        chunkList = new ChunkList;
        chunkList->size = size;
        chunkList->alignment = alignment;
        chunkList->memoryTypeIndex = memoryTypeIndex;
        mChunkLists[chunkId] = chunkList;
    }

    block.chunkId = chunkId;
    block.vkMemory = VK_NULL_HANDLE;
    for (auto chunk : chunkList->chunks) {
        if (chunk->freeBlocks.size() > 0) {
            FillBlock(block, *chunk);
            break;
        }
    }

    if (block.vkMemory == VK_NULL_HANDLE) {
        Chunk *chunk = CreateChunk(size, memoryTypeIndex);
        if (chunk) {
            chunkList->chunks.push_back(chunk);
            FillBlock(block, *chunk);
        }
    }
}

bool
MemoryAllocator::Deallocate(MemoryBlock &block)
{
    if (block.vkMemory == VK_NULL_HANDLE) {
        return false;
    }

    std::map<uint64_t, ChunkList *>::iterator it = mChunkLists.find(block.chunkId);
    if (it == mChunkLists.end()) {
        return false;
    }

    for (auto chunk : it->second->chunks) {
        if (chunk->vkMemory == block.vkMemory) {
            chunk->freeBlocks.push(block.offset);
            return true;
        }
    }

    return false;
}


}

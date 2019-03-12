#include "uniformMemory.h"

namespace vulkanAPI {

UniformMemory::UniformMemory(const XContext_t *vkContext, VkFlags flags)
: mVkContext(vkContext), mVkMemoryFlags(0), mVkFlags(flags), mCacheManager(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

UniformMemory::~UniformMemory()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

bool
UniformMemory::Create()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = mVkRequirements.size;

    VkResult err;
    err = GetMemoryTypeIndexFromProperties(&allocInfo.memoryTypeIndex);
    assert(!err);

    mVkContext->memoryAllocator->Allocate(mVkRequirements.size, mVkRequirements.alignment, allocInfo.memoryTypeIndex, mMemoryBlock);
    assert(mMemoryBlock.vkMemory);

    return (mMemoryBlock.vkMemory != VK_NULL_HANDLE);
}

void
UniformMemory::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (mMemoryBlock.vkMemory != VK_NULL_HANDLE) {
        mVkContext->memoryAllocator->Deallocate(mMemoryBlock);
    }
}

bool
UniformMemory::GetBufferMemoryRequirements(VkBuffer &buffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkRequirements), 0, sizeof(mVkRequirements));
    vkGetBufferMemoryRequirements(mVkContext->vkDevice, buffer, &mVkRequirements);

    return true;
}

VkResult
UniformMemory::GetMemoryTypeIndexFromProperties(uint32_t *typeIndex)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t typeBitsShift = mVkRequirements.memoryTypeBits;

    // Search memtypes to find first index with those properties
    for(uint32_t i = 0; i < mVkContext->vkDeviceMemoryProperties.memoryTypeCount; i++) {
        if((typeBitsShift & 1) == 1) {
            // Type is available, does it match user properties?
            if ((mVkContext->vkDeviceMemoryProperties.memoryTypes[i].propertyFlags & mVkFlags) == mVkFlags) {
                *typeIndex = i;
                return VK_SUCCESS;
            }
        }
        typeBitsShift >>= 1;
    }

    typeBitsShift = mVkRequirements.memoryTypeBits;

    // Retry with properties = 0x0
    for(uint32_t i = 0; i < mVkContext->vkDeviceMemoryProperties.memoryTypeCount; i++) {
        if((typeBitsShift & 1) == 1) {
            // Type is available, does it match user properties?
            if ((mVkContext->vkDeviceMemoryProperties.memoryTypes[i].propertyFlags & 0) == 0) {
                *typeIndex = i;
                return VK_SUCCESS;
            }
        }
        typeBitsShift >>= 1;
    }

     // No memory types matched, return failure
     return VK_ERROR_FORMAT_NOT_SUPPORTED;
}

bool
UniformMemory::BindBufferMemory(VkBuffer &buffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkBindBufferMemory(mVkContext->vkDevice, buffer, mMemoryBlock.vkMemory, mMemoryBlock.offset);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
UniformMemory::Lock(void **pData)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkMapMemory(mVkContext->vkDevice, mMemoryBlock.vkMemory, mMemoryBlock.offset, mVkRequirements.size, mVkMemoryFlags, pData);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
UniformMemory::Unlock(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    vkUnmapMemory(mVkContext->vkDevice, mMemoryBlock.vkMemory);

    return true;
}

}

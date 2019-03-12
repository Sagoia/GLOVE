/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

/**
 *  @file       memory.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Device Memory Allocation Functionality in Vulkan
 *
 *  @section
 *
 *  Device memory is memory that is visible to the device — for example the
 *  contents of the image or buffer objects, which can be natively used by
 *  the device. Memory properties of a physical device describe the memory
 *  heaps and memory types available.
 *
 */

#include "memory.h"
#include "caches.h"
#include "utils/cacheManager.h"

namespace vulkanAPI {

Memory::Memory(const XContext_t *xContext, VkFlags flags)
: mXContext(xContext), mVkMemory (VK_NULL_HANDLE), mVkMemoryFlags(0), mVkFlags(flags), mCacheManager(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

Memory::~Memory()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

bool
Memory::Create()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = 0;
    allocInfo.allocationSize = mVkRequirements.size;

    VkResult err = GetMemoryTypeIndexFromProperties(&allocInfo.memoryTypeIndex);
    assert(!err);

    err = vkAllocateMemory(mXContext->vkDevice, &allocInfo, nullptr, &mVkMemory);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_TOO_MANY_OBJECTS);
}

void
Memory::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkMemory != VK_NULL_HANDLE) {
        if (mCacheManager) {
            mCacheManager->GetSubCaches()->CacheDeviceMemory(mVkMemory);
        } else {
            vkFreeMemory(mXContext->vkDevice, mVkMemory, nullptr);
        }
        mVkMemory = VK_NULL_HANDLE;
    }
}

bool
Memory::GetData(VkDeviceSize size, VkDeviceSize offset, void *data) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    void *pData;
    VkResult err = vkMapMemory(mXContext->vkDevice, mVkMemory, offset, size, mVkMemoryFlags, &pData);
    assert(!err);

    if(err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_MEMORY_MAP_FAILED)
    {
        memcpy(data, pData, size);
        vkUnmapMemory(mXContext->vkDevice, mVkMemory);

        return true;
    }
    return false;
}

void
Memory::UpdateData(VkDeviceSize size, VkDeviceSize offset, const void *data)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    SetData(size, offset, data);
}

bool
Memory::SetData(VkDeviceSize size, VkDeviceSize offset, const void *data)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    void *pData = nullptr;

    VkResult err = vkMapMemory(mXContext->vkDevice, mVkMemory, offset, size ? size : mVkRequirements.size, mVkMemoryFlags, &pData);
    assert(!err);

    if(data) {
        memcpy(pData, data, size);
    } else {
        memset(pData, 0x0, size);
    }

    vkUnmapMemory(mXContext->vkDevice, mVkMemory);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
Memory::GetBufferMemoryRequirements(VkBuffer &buffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkRequirements), 0, sizeof(mVkRequirements));
    vkGetBufferMemoryRequirements(mXContext->vkDevice, buffer, &mVkRequirements);

    return true;
}

void
Memory::GetImageMemoryRequirements(VkImage &image)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkRequirements), 0, sizeof(mVkRequirements));
    vkGetImageMemoryRequirements(mXContext->vkDevice, image, &mVkRequirements);
}

VkResult
Memory::GetMemoryTypeIndexFromProperties(uint32_t *typeIndex)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t typeBitsShift = mVkRequirements.memoryTypeBits;

    // Search memtypes to find first index with those properties
    for(uint32_t i = 0; i < mXContext->vkDeviceMemoryProperties.memoryTypeCount; i++) {
        if((typeBitsShift & 1) == 1) {
            // Type is available, does it match user properties?
            if ((mXContext->vkDeviceMemoryProperties.memoryTypes[i].propertyFlags & mVkFlags) == mVkFlags) {
                *typeIndex = i;
                return VK_SUCCESS;
            }
        }
        typeBitsShift >>= 1;
    }

    typeBitsShift = mVkRequirements.memoryTypeBits;

    // Retry with properties = 0x0
    for(uint32_t i = 0; i < mXContext->vkDeviceMemoryProperties.memoryTypeCount; i++) {
        if((typeBitsShift & 1) == 1) {
            // Type is available, does it match user properties?
            if ((mXContext->vkDeviceMemoryProperties.memoryTypes[i].propertyFlags & 0) == 0) {
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
Memory::BindBufferMemory(VkBuffer &buffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkBindBufferMemory(mXContext->vkDevice, buffer, mVkMemory, 0);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
Memory::BindImageMemory(VkImage &image)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkBindImageMemory(mXContext->vkDevice, image, mVkMemory, 0);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

}

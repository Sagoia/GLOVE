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
 *  @file       buffer.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Resource Creation Functionality via Buffers in Vulkan
 *
 *  @section
 *
 *  Buffers represent linear arrays of data which are used for
 *  various purposes by binding them to a graphics or compute
 *  pipeline via descriptor sets or via certain commands, or
 *  by directly specifying them as parameters to certain commands.
 *  Buffers are represented by VkBuffer.
 *
 */

#include "buffer.h"

namespace vulkanAPI {

Buffer::Buffer(const vkContext_t *vkContext, const VkBufferUsageFlags vkBufferUsageFlags, const VkSharingMode vkSharingMode)
: mVkContext(vkContext), mVkBuffer(VK_NULL_HANDLE),
mVkBufferSharingMode(vkSharingMode), mVkBufferUsageFlags(vkBufferUsageFlags),
mVkSize(0), mVkOffset(0)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

Buffer::~Buffer()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
Buffer::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mVkSize = 0;
    if(mVkBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(mVkContext->vkDevice, mVkBuffer, nullptr);
        mVkBuffer = VK_NULL_HANDLE;
    }
}

bool
Buffer::Create(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkBufferCreateInfo info;
    info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.pNext                 = nullptr;
    info.flags                 = 0;
    info.size                  = mVkSize;
    info.usage                 = mVkBufferUsageFlags;
    info.sharingMode           = mVkBufferSharingMode;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices   = nullptr;

    VkResult err = vkCreateBuffer(mVkContext->vkDevice, &info, nullptr, &mVkBuffer);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

void
Buffer::CreateVkDescriptorBufferInfo(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mVkDescriptorBufferInfo.buffer = mVkBuffer;
    mVkDescriptorBufferInfo.range  = mVkSize;
    mVkDescriptorBufferInfo.offset = mVkOffset;
}

}

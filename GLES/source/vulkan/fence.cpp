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
 *  @file       fence.cpp
 *  @author     Think Silicon
 *  @date       03/09/2018
 *  @version    1.0
 *
 *  @brief      Determine completion of execution of queue operations via Fences in Vulkan
 *
 *
 */

#include "fence.h"

namespace vulkanAPI {

Fence::Fence(const vkContext_t *vkContext)
: mVkContext(vkContext), mVkFence(VK_NULL_HANDLE)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

Fence::~Fence()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
Fence::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkFence != VK_NULL_HANDLE) {
        vkDestroyFence(mVkContext->vkDevice, mVkFence, nullptr);
        mVkFence = VK_NULL_HANDLE;
    }
}

bool
Fence::Reset(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkResetFences(mVkContext->vkDevice, 1, &mVkFence);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
Fence::Wait(VkBool32 waitAll, uint64_t timeout)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = VK_TIMEOUT;

    do {

      err = vkWaitForFences(mVkContext->vkDevice, 1, &mVkFence, waitAll, timeout);
      assert(!err);

      if(err == VK_ERROR_OUT_OF_HOST_MEMORY || err == VK_ERROR_OUT_OF_DEVICE_MEMORY || err == VK_ERROR_DEVICE_LOST)
          return false;

    } while (err == VK_TIMEOUT);

    return true;
}

bool
Fence::Create(bool signaled)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkFenceCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VkResult err = vkCreateFence(mVkContext->vkDevice, &info, nullptr, &mVkFence);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

}

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
 *  @file       commandBufferPool.cpp
 *  @author     Think Silicon
 *  @date       16/10/2018
 *  @version    1.0
 *
 *  @brief      Command Buffer Pool Functionality in Vulkan
 *
 */

#include "commandBufferPool.h"

namespace vulkanAPI {

CommandBufferPool::CommandBufferPool()
: mSize(0), mLastUsedBuffer(0), mLocked(true)
{

}

CommandBufferPool::~CommandBufferPool()
{

}

void
CommandBufferPool::AddBuffer(VkCommandBuffer *commandBuffer)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mPool.push_back(commandBuffer);
    mSize++;
    mLocked = true;
}

VkCommandBuffer *
CommandBufferPool::RemoveBuffer(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    VkCommandBuffer *removedBuffer = mPool.back();
    if(removedBuffer) {
        mPool.pop_back();
        mSize--;
    }

    return removedBuffer;
}

VkCommandBuffer *
CommandBufferPool::BindNextAvailableBuffer(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    VkCommandBuffer *nextAvailableBuffer = nullptr;

    if(!mLocked && mLastUsedBuffer < mSize) {
        nextAvailableBuffer = mPool[mLastUsedBuffer];
        mLastUsedBuffer++;
    }

    return nextAvailableBuffer;
}

void
CommandBufferPool::UnbindAllBuffers(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mLastUsedBuffer = 0;
    mLocked = false;
}

}

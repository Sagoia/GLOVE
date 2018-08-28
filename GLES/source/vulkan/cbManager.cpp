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
 *  @file       cbManager.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Command Buffers Manager Functionality in Vulkan
 *
 *  @section
 *
 *  Command buffers are objects used to record commands which can be
 *  subsequently submitted to a device queue for execution. There are
 *  two levels of command buffers - primary command buffers, which can
 *  execute secondary command buffers, and which are submitted to queues,
 *  and secondary command buffers, which can be executed by primary command
 *  buffers, and which are not directly submitted to queues.
 *  Command buffers are represented by VkCommandBuffer.
 *
 */

#include "cbManager.h"

namespace vulkanAPI {

#define GLOVE_NO_BUFFER_TO_WAIT                         0x7FFFFFFF
#define GLOVE_NUM_COMMAND_BUFFERS                       2
#define GLOVE_FENCE_WAIT_TIMEOUT                        UINT64_MAX

CommandBufferManager *CommandBufferManager::mInstance = nullptr;

CommandBufferManager::CommandBufferManager(vkContext_t *context)
: mVkContext(context)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mActiveCmdBuffer    = 0;
    mLastSubmittedBuffer= GLOVE_NO_BUFFER_TO_WAIT;

    mVkCmdPool          = VK_NULL_HANDLE;
    mVkAuxCommandBuffer = VK_NULL_HANDLE;
    mVkAuxFence         = VK_NULL_HANDLE;
}

CommandBufferManager::~CommandBufferManager()
{
    FUN_ENTRY(GL_LOG_TRACE);

    assert(this == mInstance);

    for(uint32_t i = 0; i < mReferencedResources.size(); ++i) {
        mReferencedResources[i]->mRefCount = 0;
    }

    FreeResources();

    mReferencedResources.clear();

    if(mVkContext->vkDevice != VK_NULL_HANDLE ) {

        vkDeviceWaitIdle(mVkContext->vkDevice);

        for(uint32_t i = 0; i < mVkCommandBuffers.fence.size(); ++i) {
            if(mVkCommandBuffers.fence[i] != VK_NULL_HANDLE) {
                vkDestroyFence(mVkContext->vkDevice, mVkCommandBuffers.fence[i], NULL);
            }
        }

        vkFreeCommandBuffers(mVkContext->vkDevice, mVkCmdPool, mVkCommandBuffers.commandBuffer.size(), mVkCommandBuffers.commandBuffer.data());
        mVkCommandBuffers.commandBuffer.clear();
        mVkCommandBuffers.commandBufferState.clear();
        mVkCommandBuffers.fence.clear();
        memset((void *)&mVkCommandBuffers, 0, mVkCommandBuffers.commandBuffer.size()*sizeof(State));

        if(mVkAuxCommandBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(mVkContext->vkDevice, mVkCmdPool, 1, &mVkAuxCommandBuffer);
            mVkAuxCommandBuffer = VK_NULL_HANDLE;
        }

        if(mVkCmdPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(mVkContext->vkDevice, mVkCmdPool, NULL);
            mVkCmdPool = VK_NULL_HANDLE;
        }
    }
}

void
CommandBufferManager::Release(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    delete mInstance;
    mInstance = nullptr;
}

void
CommandBufferManager::FreeResources(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(std::vector<resourceBase_t *>::iterator it = mReferencedResources.begin(); it != mReferencedResources.end();) {
        const resourceBase_t *resourceBase = *it;
        if(!resourceBase->mRefCount) {
            switch(resourceBase->mType) {
            case RESOURCE_TYPE_SHADER: {
                referencedResource_t<VkShaderModule> *resource = (referencedResource_t<VkShaderModule> *)resourceBase;
                vkDestroyShaderModule(mVkContext->vkDevice, resource->mResourcePtr, NULL);
                } break;
            case RESOURCE_TYPE_PIPELINE_LAYOUT: {
                referencedResource_t<VkPipelineLayout> *resource = (referencedResource_t<VkPipelineLayout> *)resourceBase;
                vkDestroyPipelineLayout(mVkContext->vkDevice, resource->mResourcePtr, NULL);
                } break;
            case RESOURCE_TYPE_DESC_POOL: {
                referencedResource_t<VkDescriptorPool> *resource = (referencedResource_t<VkDescriptorPool> *)resourceBase;
                vkDestroyDescriptorPool(mVkContext->vkDevice, resource->mResourcePtr, NULL);
                } break;
            case RESOURCE_TYPE_DESC_SET_LAYOUT: {
                referencedResource_t<VkDescriptorSetLayout> *resource = (referencedResource_t<VkDescriptorSetLayout> *)resourceBase;
                vkDestroyDescriptorSetLayout(mVkContext->vkDevice, resource->mResourcePtr, NULL);
                } break;
            default: NOT_REACHED(); break;
            }

            delete *it;
            it = mReferencedResources.erase(it);
        } else {
            ++it;
        }
    }
}

CommandBufferManager *
CommandBufferManager::Get(vkContext_t *context)
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(mInstance == nullptr) {
        mInstance = new CommandBufferManager(context);
    }

    return mInstance;
}

bool
CommandBufferManager::AllocateVkCmdBuffers(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandPoolCreateInfo cmdPoolInfo;
    memset((void *)&cmdPoolInfo, 0 ,sizeof(cmdPoolInfo));
    cmdPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext            = NULL;
    cmdPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = mVkContext->vkGraphicsQueueNodeIndex;

    VkResult err = vkCreateCommandPool(mVkContext->vkDevice, &cmdPoolInfo, NULL, &mVkCmdPool);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    mVkCommandBuffers.commandBuffer.resize(GLOVE_NUM_COMMAND_BUFFERS);
    mVkCommandBuffers.commandBufferState.resize(GLOVE_NUM_COMMAND_BUFFERS);
    mVkCommandBuffers.fence.resize(GLOVE_NUM_COMMAND_BUFFERS);

    VkCommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext              = NULL;
    cmdAllocInfo.commandPool        = mVkCmdPool;
    cmdAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = GLOVE_NUM_COMMAND_BUFFERS;

    err = vkAllocateCommandBuffers(mVkContext->vkDevice, &cmdAllocInfo, mVkCommandBuffers.commandBuffer.data());
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    cmdAllocInfo.commandBufferCount = 1;
    err = vkAllocateCommandBuffers(mVkContext->vkDevice, &cmdAllocInfo, &mVkAuxCommandBuffer);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = 0;

    for(uint32_t i = 0; i < GLOVE_NUM_COMMAND_BUFFERS; ++i) {
        mVkCommandBuffers.commandBufferState[i] = CMD_BUFFER_INITIAL_STATE;

        err = vkCreateFence(mVkContext->vkDevice, &fenceInfo, NULL, &mVkCommandBuffers.fence[i]);
        assert(!err);

        if(err != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

bool
CommandBufferManager::BeginVkDrawCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] == CMD_BUFFER_RECORDING_STATE) {
        return true;
    }

    VkCommandBufferBeginInfo info;
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext            = NULL;
    info.flags            = 0;
    info.pInheritanceInfo = NULL;

    VkResult err = vkBeginCommandBuffer(mVkCommandBuffers.commandBuffer[mActiveCmdBuffer], &info);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] = CMD_BUFFER_RECORDING_STATE;

    return true;
}

void
CommandBufferManager::EndVkDrawCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] == CMD_BUFFER_EXECUTABLE_STATE ||
       mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] == CMD_BUFFER_INITIAL_STATE) {
        return;
    }

    vkEndCommandBuffer(mVkCommandBuffers.commandBuffer[mActiveCmdBuffer]);

    mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] = CMD_BUFFER_EXECUTABLE_STATE;
}

bool
CommandBufferManager::SubmitVkDrawCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] == CMD_BUFFER_INITIAL_STATE) {
        return true;
    }

    vector<VkSemaphore> pSems;
    vector<VkPipelineStageFlags> pFlags;
    if(mVkContext->vkSyncItems->acquireSemaphoreFlag) {
        pSems.push_back(mVkContext->vkSyncItems->vkAcquireSemaphore);
        pFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
    if(mVkContext->vkSyncItems->drawSemaphoreFlag) {
        pSems.push_back(mVkContext->vkSyncItems->vkDrawSemaphore);
        pFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = NULL;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &mVkCommandBuffers.commandBuffer[mActiveCmdBuffer];
    submitInfo.waitSemaphoreCount   = pSems.size();
    submitInfo.pWaitSemaphores      = pSems.data();
    submitInfo.pWaitDstStageMask    = pFlags.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &mVkContext->vkSyncItems->vkDrawSemaphore;

    mVkContext->vkSyncItems->drawSemaphoreFlag    = true;
    mVkContext->vkSyncItems->acquireSemaphoreFlag = false;

    VkResult err = vkQueueSubmit(mVkContext->vkQueue, 1, &submitInfo, mVkCommandBuffers.fence[mActiveCmdBuffer]);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] = CMD_BUFFER_SUBMITED_STATE;

    mLastSubmittedBuffer = mActiveCmdBuffer;
    mActiveCmdBuffer = (mActiveCmdBuffer + 1) % GLOVE_NUM_COMMAND_BUFFERS;

    mVkCommandBuffers.commandBufferState[mActiveCmdBuffer] = CMD_BUFFER_INITIAL_STATE;

    return true;
}

bool
CommandBufferManager::WaitLastSubmition(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err;

    if(mLastSubmittedBuffer != GLOVE_NO_BUFFER_TO_WAIT) {

        do
        {
            err = vkWaitForFences(mVkContext->vkDevice, 1, &mVkCommandBuffers.fence[mLastSubmittedBuffer], VK_TRUE, GLOVE_FENCE_WAIT_TIMEOUT);
        }
        while (err == VK_TIMEOUT);
        assert(!err);

        FreeResources();

        err = vkResetFences(mVkContext->vkDevice, 1, &mVkCommandBuffers.fence[mLastSubmittedBuffer]);
        assert(!err);

        if(err != VK_SUCCESS) {
            return false;
        }

        mLastSubmittedBuffer = GLOVE_NO_BUFFER_TO_WAIT;
    }

    return true;
}

bool
CommandBufferManager::BeginVkAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandBufferBeginInfo info;
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext            = NULL;
    info.flags            = 0;
    info.pInheritanceInfo = NULL;

    VkResult err = vkBeginCommandBuffer(mVkAuxCommandBuffer, &info);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
CommandBufferManager::EndVkAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkEndCommandBuffer(mVkAuxCommandBuffer);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
CommandBufferManager::SubmitVkAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkSubmitInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext                  = NULL;
    info.commandBufferCount     = 1;
    info.pCommandBuffers        = &mVkAuxCommandBuffer;

    VkResult err = vkQueueSubmit(mVkContext->vkQueue, 1, &info, mVkAuxFence);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_DEVICE_LOST);
}

bool
CommandBufferManager::WaitVkAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkQueueWaitIdle(mVkContext->vkQueue);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_DEVICE_LOST);
}

}

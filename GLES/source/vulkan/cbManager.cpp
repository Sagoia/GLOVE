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

CommandBufferManager::CommandBufferManager(const XContext_t *context)
: mXContext(context)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mActiveCmdBuffer    = 0;
    mLastSubmittedBuffer= GLOVE_NO_BUFFER_TO_WAIT;

    mCmdPool          = VK_NULL_HANDLE;
    mAuxCmdBuffer = VK_NULL_HANDLE;
    mAuxFence         = VK_NULL_HANDLE;

    if(!AllocateCommandPool()) {
        assert(false);
        return ;
    }

    if(!AllocateCommandBuffers()) {
        assert(false);
        return ;
    }

}

CommandBufferManager::~CommandBufferManager()
{
    FUN_ENTRY(GL_LOG_TRACE);

    for(uint32_t i = 0; i < mReferencedResources.size(); ++i) {
        mReferencedResources[i]->mRefCount = 0;
    }

    FreeResources();

    mReferencedResources.clear();

    if(mXContext->vkDevice != VK_NULL_HANDLE ) {

        vkDeviceWaitIdle(mXContext->vkDevice);

        DestroyCommandBuffers();

        if(mCmdPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(mXContext->vkDevice, mCmdPool, nullptr);
            mCmdPool = VK_NULL_HANDLE;
        }
    }
}

void
CommandBufferManager::DestroyCommandBuffers(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    for(uint32_t i = 0; i < mPrimaryCmdBuffers.fence.size(); ++i) {
        mPrimaryCmdBuffers.fence[i].Release();
    }

    vkFreeCommandBuffers(mXContext->vkDevice, mCmdPool, (uint32_t)mPrimaryCmdBuffers.commandBuffer.size(), mPrimaryCmdBuffers.commandBuffer.data());
    mPrimaryCmdBuffers.commandBuffer.clear();
    mPrimaryCmdBuffers.commandBufferState.clear();
    mPrimaryCmdBuffers.fence.clear();
    memset(static_cast<void *>(&mPrimaryCmdBuffers), 0, mPrimaryCmdBuffers.commandBuffer.size()*sizeof(State));

    if(mAuxCmdBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(mXContext->vkDevice, mCmdPool, 1, &mAuxCmdBuffer);
        mAuxCmdBuffer = VK_NULL_HANDLE;
    }

    uint32_t secondaryBuffersPoolSize = mSecondaryCmdBufferPool.GetSize();

    for(uint32_t i = 0; i < secondaryBuffersPoolSize; ++i) {
        VkCommandBuffer *removingSecondaryBuffer = mSecondaryCmdBufferPool.RemoveBuffer();
        if(removingSecondaryBuffer) {
            vkFreeCommandBuffers(mXContext->vkDevice, mCmdPool, 1, removingSecondaryBuffer);
            delete removingSecondaryBuffer;
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

VkCommandBuffer *
CommandBufferManager::AllocateSecondaryCommandBuffers(uint32_t numOfBuffers)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandBuffer *reusedCommandBuffer = mSecondaryCmdBufferPool.BindNextAvailableBuffer();

    if(nullptr != reusedCommandBuffer) {
        return reusedCommandBuffer;
    }

    VkResult err;
    VkCommandBuffer *commandBuffers = new VkCommandBuffer[numOfBuffers];

    VkCommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext              = nullptr;
    cmdAllocInfo.commandPool        = mCmdPool;
    cmdAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmdAllocInfo.commandBufferCount = numOfBuffers;

    err = vkAllocateCommandBuffers(mXContext->vkDevice, &cmdAllocInfo, commandBuffers);
    assert(!err);

    if(err != VK_SUCCESS) {
        return nullptr;
    }

    for (uint32_t i = 0; i < numOfBuffers; ++i) {
        mSecondaryCmdBufferPool.AddBuffer(commandBuffers + i);
    }

    return commandBuffers;
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
                vkDestroyShaderModule(mXContext->vkDevice, resource->mResourcePtr, nullptr);
                } break;
            case RESOURCE_TYPE_PIPELINE_LAYOUT: {
                referencedResource_t<VkPipelineLayout> *resource = (referencedResource_t<VkPipelineLayout> *)resourceBase;
                vkDestroyPipelineLayout(mXContext->vkDevice, resource->mResourcePtr, nullptr);
                } break;
            case RESOURCE_TYPE_DESC_POOL: {
                referencedResource_t<VkDescriptorPool> *resource = (referencedResource_t<VkDescriptorPool> *)resourceBase;
                vkDestroyDescriptorPool(mXContext->vkDevice, resource->mResourcePtr, nullptr);
                } break;
            case RESOURCE_TYPE_DESC_SET_LAYOUT: {
                referencedResource_t<VkDescriptorSetLayout> *resource = (referencedResource_t<VkDescriptorSetLayout> *)resourceBase;
                vkDestroyDescriptorSetLayout(mXContext->vkDevice, resource->mResourcePtr, nullptr);
                } break;
            default: NOT_REACHED(); break;
            }

            delete *it;
            it = mReferencedResources.erase(it);
        } else {
            ++it;
        }
    }

    mSecondaryCmdBufferPool.UnbindAllBuffers();
}

bool
CommandBufferManager::AllocateCommandPool(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandPoolCreateInfo cmdPoolInfo;
    memset(static_cast<void *>(&cmdPoolInfo), 0 ,sizeof(cmdPoolInfo));
    cmdPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext            = nullptr;
    cmdPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = mXContext->vkGraphicsQueueNodeIndex;

    VkResult err = vkCreateCommandPool(mXContext->vkDevice, &cmdPoolInfo, nullptr, &mCmdPool);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool
CommandBufferManager::AllocateCommandBuffers(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mPrimaryCmdBuffers.commandBuffer.resize(GLOVE_NUM_COMMAND_BUFFERS);
    mPrimaryCmdBuffers.commandBufferState.resize(GLOVE_NUM_COMMAND_BUFFERS);
    mPrimaryCmdBuffers.fence.resize(GLOVE_NUM_COMMAND_BUFFERS);

    VkCommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext              = nullptr;
    cmdAllocInfo.commandPool        = mCmdPool;
    cmdAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = GLOVE_NUM_COMMAND_BUFFERS;

    VkResult err = vkAllocateCommandBuffers(mXContext->vkDevice, &cmdAllocInfo, mPrimaryCmdBuffers.commandBuffer.data());
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    cmdAllocInfo.commandBufferCount = 1;
    err = vkAllocateCommandBuffers(mXContext->vkDevice, &cmdAllocInfo, &mAuxCmdBuffer);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    for(uint32_t i = 0; i < GLOVE_NUM_COMMAND_BUFFERS; ++i) {
        mPrimaryCmdBuffers.commandBufferState[i] = COMMAND_BUFFER_INITIAL_STATE;

        mPrimaryCmdBuffers.fence[i].SetContext(mXContext);
        if(!mPrimaryCmdBuffers.fence[i].Create(false)) {
            return false;
        }
    }

    return true;
}

bool
CommandBufferManager::BeginDrawCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] == COMMAND_BUFFER_RECORDING_STATE) {
        return true;
    }

    VkCommandBufferBeginInfo info;
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext            = nullptr;
    info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.pInheritanceInfo = nullptr;

    VkResult err = vkBeginCommandBuffer(mPrimaryCmdBuffers.commandBuffer[mActiveCmdBuffer], &info);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] = COMMAND_BUFFER_RECORDING_STATE;

    return true;
}

bool
CommandBufferManager::BeginSecondaryCommandBuffer(const VkCommandBuffer *cmdBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext = nullptr;
    inheritanceInfo.renderPass = renderPass;
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = framebuffer;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo cmdBeginInfo;
    cmdBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext            = nullptr;
    cmdBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmdBeginInfo.pInheritanceInfo = &inheritanceInfo;

    VkResult err = vkBeginCommandBuffer(*cmdBuffer, &cmdBeginInfo);

    if(err != VK_SUCCESS) {
        return false;
    }

    return true;
}


void
CommandBufferManager::EndDrawCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] == COMMAND_BUFFER_EXECUTABLE_STATE ||
       mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] == COMMAND_BUFFER_INITIAL_STATE) {
        return;
    }

    vkEndCommandBuffer(mPrimaryCmdBuffers.commandBuffer[mActiveCmdBuffer]);

    mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] = COMMAND_BUFFER_EXECUTABLE_STATE;
}

void
CommandBufferManager::EndSecondaryCommandBuffer(const VkCommandBuffer *cmdBuffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    vkEndCommandBuffer(*cmdBuffer);
}

bool
CommandBufferManager::SubmitDrawCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] == COMMAND_BUFFER_INITIAL_STATE) {
        return true;
    }

    std::vector<VkSemaphore> pSems;
    std::vector<VkPipelineStageFlags> pFlags;
    if(mXContext->vkSyncItems->acquireSemaphoreFlag) {
        pSems.push_back(mXContext->vkSyncItems->vkAcquireSemaphore);
        pFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
    if(mXContext->vkSyncItems->drawSemaphoreFlag) {
        pSems.push_back(mXContext->vkSyncItems->vkDrawSemaphore);
        pFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = nullptr;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &mPrimaryCmdBuffers.commandBuffer[mActiveCmdBuffer];
    submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(pSems.size());
    submitInfo.pWaitSemaphores      = pSems.data();
    submitInfo.pWaitDstStageMask    = pFlags.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &mXContext->vkSyncItems->vkDrawSemaphore;

    mXContext->vkSyncItems->drawSemaphoreFlag    = true;
    mXContext->vkSyncItems->acquireSemaphoreFlag = false;

    VkResult err = vkQueueSubmit(mXContext->vkQueue, 1, &submitInfo, mPrimaryCmdBuffers.fence[mActiveCmdBuffer].GetFence());
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] = COMMAND_BUFFER_SUBMITED_STATE;

    mLastSubmittedBuffer = mActiveCmdBuffer;
    mActiveCmdBuffer = (mActiveCmdBuffer + 1) % GLOVE_NUM_COMMAND_BUFFERS;

    mPrimaryCmdBuffers.commandBufferState[mActiveCmdBuffer] = COMMAND_BUFFER_INITIAL_STATE;

    return true;
}

bool
CommandBufferManager::WaitLastSubmition(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mLastSubmittedBuffer != GLOVE_NO_BUFFER_TO_WAIT) {

        if(!mPrimaryCmdBuffers.fence[mLastSubmittedBuffer].Wait(VK_TRUE, GLOVE_FENCE_WAIT_TIMEOUT))
            return false;

        FreeResources();

        if(!mPrimaryCmdBuffers.fence[mLastSubmittedBuffer].Reset()) {
            return false;
        }

        mLastSubmittedBuffer = GLOVE_NO_BUFFER_TO_WAIT;
        return true;
    }

    return true;
}

bool
CommandBufferManager::BeginAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandBufferBeginInfo info;
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext            = nullptr;
    info.flags            = 0;
    info.pInheritanceInfo = nullptr;

    VkResult err = vkBeginCommandBuffer(mAuxCmdBuffer, &info);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
CommandBufferManager::EndAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkEndCommandBuffer(mAuxCmdBuffer);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
CommandBufferManager::SubmitAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkSubmitInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext                  = nullptr;
    info.commandBufferCount     = 1;
    info.pCommandBuffers        = &mAuxCmdBuffer;

    VkResult err = vkQueueSubmit(mXContext->vkQueue, 1, &info, mAuxFence);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_DEVICE_LOST);
}

bool
CommandBufferManager::WaitAuxCommandBuffer(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err = vkQueueWaitIdle(mXContext->vkQueue);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY && err != VK_ERROR_DEVICE_LOST);
}

}

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
 *  @file       commandBufferManager.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Command Buffers Manager Functionality in Vulkan
 *
 */

#ifndef __VKCBMANAGER_H__
#define __VKCBMANAGER_H__

#include <vector>
#include "context.h"
#include "fence.h"
#include "commandBufferPool.h"

namespace vulkanAPI {

typedef enum {
    CMD_BUFFER_INITIAL_STATE = 0,
    CMD_BUFFER_RECORDING_STATE,
    CMD_BUFFER_EXECUTABLE_STATE,
    CMD_BUFFER_SUBMITED_STATE
} cmdBufferState_t;

class CommandBufferManager final {
private:

    typedef struct State {
        std::vector<VkCommandBuffer>         commandBuffer;
        std::vector<cmdBufferState_t>        commandBufferState;
        std::vector<Fence>                   fence;

        State()  { FUN_ENTRY(GL_LOG_TRACE); }
        ~State() { FUN_ENTRY(GL_LOG_TRACE); }
    } State;

    VkCommandPool                   mVkCmdPool;
    const vkContext_t              *mVkContext;

    uint32_t                        mActiveCmdBuffer;
    int32_t                         mLastSubmittedBuffer;

    State                           mVkCommandBuffers;

    VkCommandBuffer                 mVkAuxCommandBuffer;
    VkFence                         mVkAuxFence;
    CommandBufferPool               mSecondaryCmdBufferPool;

    void FreeResources(void);

public:
// Constructor
    CommandBufferManager(const vkContext_t *context = nullptr);

// Destructor
    ~CommandBufferManager();

// Release Functions
    void Release(void);

// Allocate Functions
    bool AllocateVkCmdPool(void);
    bool AllocateVkCmdBuffers(void);

// Destroy Functions
    void DestroyVkCmdBuffers(void);
    VkCommandBuffer *AllocateVkSecondaryCmdBuffers(uint32_t numOfBuffers);

// Begin Functions
    bool BeginVkAuxCommandBuffer(void);
    bool BeginVkDrawCommandBuffer(void);
    bool BeginVkSecondaryCommandBuffer(const VkCommandBuffer *cmdBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer);

// End Functions
    bool EndVkAuxCommandBuffer(void);
    void EndVkDrawCommandBuffer(void);
    void EndVkSecondaryCommandBuffer(const VkCommandBuffer *cmdBuffer);

// Submit Functions
    bool SubmitVkDrawCommandBuffer(void);
    bool SubmitVkAuxCommandBuffer(void);

// Wait Functions
    bool WaitLastSubmition(void);
    bool WaitVkAuxCommandBuffer(void);

// Get Functions
    inline VkCommandBuffer GetActiveCommandBuffer(void)                   const { FUN_ENTRY(GL_LOG_TRACE); return mVkCommandBuffers.commandBuffer[mActiveCmdBuffer]; }
    inline VkCommandBuffer GetAuxCommandBuffer(void)                      const { FUN_ENTRY(GL_LOG_TRACE); return mVkAuxCommandBuffer; }
};

}
#endif // __VKCBMANAGER_H__

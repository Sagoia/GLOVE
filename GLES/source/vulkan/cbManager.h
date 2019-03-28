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
 *  @file       cbManager.h
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
    COMMAND_BUFFER_INITIAL_STATE = 0,
    COMMAND_BUFFER_RECORDING_STATE,
    COMMAND_BUFFER_EXECUTABLE_STATE,
    COMMAND_BUFFER_SUBMITED_STATE
} commandBufferState_t;

typedef enum {
    RESOURCE_TYPE_SHADER = 0,
    RESOURCE_TYPE_PIPELINE_LAYOUT,
    RESOURCE_TYPE_DESC_POOL,
    RESOURCE_TYPE_DESC_SET_LAYOUT,
    RESOURCE_TYPE_DESC_SET,
    RESOURCE_TYPE_LAST
} resourceType_t;

class resourceBase_t {
public:

    uint32_t                        mRefCount;
    resourceType_t                  mType;

    resourceBase_t(resourceType_t type) : mRefCount(1), mType(type)             { FUN_ENTRY(GL_LOG_TRACE); }
    virtual ~resourceBase_t()                                                   { FUN_ENTRY(GL_LOG_TRACE); }
};

template<typename T>
class referencedResource_t : public resourceBase_t {
public:

    T                               mResourcePtr;

    referencedResource_t(T resourcePtr, resourceType_t type)
    : resourceBase_t(type), mResourcePtr(resourcePtr)                           { FUN_ENTRY(GL_LOG_TRACE); }
};

class CommandBufferManager {
private:

    typedef struct State {
        std::vector<VkCommandBuffer>         commandBuffer;
        std::vector<commandBufferState_t>    commandBufferState;
        std::vector<Fence>                   fence;

        State()  { FUN_ENTRY(GL_LOG_TRACE); }
        ~State() { FUN_ENTRY(GL_LOG_TRACE); }
    } State;

    static CommandBufferManager    *mInstance;
    VkCommandPool                   mCmdPool;
    const XContext_t               *mXContext;

    uint32_t                        mActiveCmdBuffer;
    int32_t                         mLastSubmittedBuffer;

    State                           mPrimaryCmdBuffers;

    VkCommandBuffer                 mAuxCmdBuffer;
    VkFence                         mAuxFence;
    CommandBufferPool               mSecondaryCmdBufferPool;

    std::vector<resourceBase_t *>   mReferencedResources;

    void FreeResources(void);

public:
// Constructor
    CommandBufferManager(const XContext_t *context = nullptr);

// Destructor
    ~CommandBufferManager();

// Release Functions
    void Release(void);

// Allocate Functions
    bool AllocateCommandPool(void);
    bool AllocateCommandBuffers(void);
    VkCommandBuffer *AllocateSecondaryCommandBuffers(uint32_t numOfBuffers);

// Destroy Functions
    void DestroyCommandBuffers(void);

// Begin Functions
    bool BeginAuxCommandBuffer(void);
    bool BeginDrawCommandBuffer(void);
    bool BeginSecondaryCommandBuffer(const VkCommandBuffer *cmdBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer);

// End Functions
    bool EndAuxCommandBuffer(void);
    void EndDrawCommandBuffer(void);
    void EndSecondaryCommandBuffer(const VkCommandBuffer *cmdBuffer);

// Submit Functions
    bool SubmitDrawCommandBuffer(void);
    bool SubmitAuxCommandBuffer(void);

// Wait Functions
    bool WaitLastSubmition(void);
    bool WaitAuxCommandBuffer(void);

// Get Functions
    inline VkCommandBuffer GetActiveCommandBuffer(void)                   const { FUN_ENTRY(GL_LOG_TRACE); return mPrimaryCmdBuffers.commandBuffer[mActiveCmdBuffer]; }
    inline VkCommandBuffer GetAuxCommandBuffer(void)                      const { FUN_ENTRY(GL_LOG_TRACE); return mAuxCmdBuffer; }

// Resource Functions
    template<typename T>
    void RefResource(T resource, resourceType_t type)
    {
        FUN_ENTRY(GL_LOG_TRACE);

        int index = LocateResource(resource);
        if(index == -1) {
            mReferencedResources.push_back(new referencedResource_t<T>(resource, type));
        } else {
            ++mReferencedResources[index]->mRefCount;
        }
    }

    template<typename T>
    void UnrefResouce(T resource)
    {
        FUN_ENTRY(GL_LOG_TRACE);

        int index = LocateResource(resource);
        if(index != -1) {
            assert(mReferencedResources[index]->mRefCount);
            --mReferencedResources[index]->mRefCount;
        }
    }

    template<typename T>
    int LocateResource(T resource)
    {
        FUN_ENTRY(GL_LOG_TRACE);

        assert(resource);

        for(uint32_t i = 0; i < mReferencedResources.size(); ++i) {
            referencedResource_t<T> *res = (referencedResource_t<T> *)mReferencedResources[i];
            assert(res);
            if(res->mResourcePtr == resource) {
                return i;
            }
        }

        return -1;
    }
};

}
#endif // __VKCBMANAGER_H__

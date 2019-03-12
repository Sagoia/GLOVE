#ifndef __UNIFORMBUFFEROBJECT_H__
#define __UNIFORMBUFFEROBJECT_H__

#include "GLES2/gl2.h"
#include "vulkan/uniformMemory.h"
#include "vulkan/buffer.h"

class CacheManager;

class UniformBufferObject {
private:
    const
    vulkanAPI::XContext_t*     mVkContext;

    GLenum                      mUsage;
    bool                        mAllocated;
    uint32_t                    mCacheIndex;

    vulkanAPI::UniformMemory*   mMemory;
    vulkanAPI::Buffer*          mBuffer;

public:
    explicit                    UniformBufferObject(const vulkanAPI::XContext_t *vkContext = nullptr);
    virtual                     ~UniformBufferObject();

// Allocate Functions
    bool                        Allocate(size_t bufferSize, const void *data, size_t dataSize);
    inline void                 AllocateVkDescriptorBufferInfo()                    { FUN_ENTRY(GL_LOG_DEBUG); mBuffer->CreateVkDescriptorBufferInfo(); }

// Release Functions
    void                        Release(void);

// Update Functions
    bool                        UpdateData(size_t size, size_t offset, const void *data);

// Get Functions
    inline GLenum               GetUsage(void)                              const   { FUN_ENTRY(GL_LOG_TRACE); return mUsage; }
    inline uint32_t             GetCacheIndex(void)                         const   { FUN_ENTRY(GL_LOG_TRACE); return mCacheIndex; }
    inline size_t               GetSize(void)                               const   { FUN_ENTRY(GL_LOG_TRACE); return mBuffer->GetSize(); }
    inline VkBuffer             GetVkBuffer(void)                                   { FUN_ENTRY(GL_LOG_TRACE); return mBuffer->GetVkBuffer(); }
    inline VkDescriptorBufferInfo * 
                                GetBufferDescInfo(void)                             { FUN_ENTRY(GL_LOG_TRACE); return mBuffer->GetVkDescriptorBufferInfo(); }

// Set Functions
    inline void                 SetUsage(GLenum usage)                              { FUN_ENTRY(GL_LOG_TRACE); mUsage     = usage; }
    inline void                 SetVkContext(const vulkanAPI::XContext_t *vkContext)
                                                                                    { FUN_ENTRY(GL_LOG_TRACE); mVkContext = vkContext;
                                                                                                               mBuffer->SetContext(vkContext);
                                                                                                               mMemory->SetContext(vkContext); }
    inline void                 SetCacheManager(CacheManager *cacheManager)         { FUN_ENTRY(GL_LOG_TRACE); mBuffer->SetCacheManager(cacheManager);
                                                                                                               mMemory->SetCacheManager(cacheManager); }
// Has/Is Functions
    inline bool                 HasData(void)                               const   { FUN_ENTRY(GL_LOG_TRACE); return mBuffer->GetVkBuffer() != nullptr; }
};

#endif //__UNIFORMBUFFEROBJECT_H__

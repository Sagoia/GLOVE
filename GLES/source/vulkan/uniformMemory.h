#ifndef __VKUNIFORMMEMORY_H__
#define __VKUNIFORMMEMORY_H__

#include "context.h"
#include "memoryAllocator.h"
#include "buffer.h"

class CacheManager;

namespace vulkanAPI {

class UniformMemory {

private:

    const
    XContext_t *                    mXContext;

    const
    VkMemoryMapFlags                mVkMemoryFlags;
    VkFlags                         mVkFlags;
    VkMemoryRequirements            mVkRequirements;

    MemoryBlock                     mMemoryBlock;

    CacheManager *                  mCacheManager;

public:
    // Constructor
    UniformMemory(const XContext_t *xContext = nullptr, VkFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Destructor
    ~UniformMemory();

    // Allocate Functions
    bool                            Create(void);

    // Release Functions
    void                            Release(void);

    // Bind Functions
    bool                            BindBufferMemory(Buffer *buffer);

    // Get Functions
    inline VkFlags                  GetFlags(void)                            { FUN_ENTRY(GL_LOG_DEBUG); return mVkFlags; }
    bool                            GetBufferMemoryRequirements(Buffer *buffer);
    VkResult                        GetMemoryTypeIndexFromProperties(uint32_t *typeIndex);

    // Set Functions
    inline void                     SetContext(const XContext_t *xContext)    { FUN_ENTRY(GL_LOG_TRACE); mXContext = xContext; }
    inline void                     SetCacheManager(CacheManager *manager)    { FUN_ENTRY(GL_LOG_TRACE); mCacheManager = manager; }

    // Update Functions
    bool                            Lock(void **pData);
    bool                            Unlock(void);

};

}



#endif //__VKUNIFORMMEMORY_H__

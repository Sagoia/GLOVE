/**
 *  @file       cacheManager.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief
 *
 */

#include <vector>
#include "vulkan/vulkan.h"

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <vector>
#include "utils/glLogger.h"
#include "utils/globals.h"
#include "resources/bufferObject.h"

class CacheManager {
private:
    const
    vulkanAPI::vkContext_t *            mVkContext;

    std::vector<UniformBufferObject *>  mUniformBufferObjectCache;
    std::vector<BufferObject *>         mVBOCache;
    std::vector<VkPipeline>             mVkPipelineCache;

    void                                CleanUpUniformBufferObjectCache(void);
    void                                CleanUpVBOCache();
    void                                CleanUpVkPipelineCache();

public:
     CacheManager(const vulkanAPI::vkContext_t *vkContext) : mVkContext(vkContext) { }
    ~CacheManager() { }

    void                                CacheUniformBufferObject(UniformBufferObject *uniformBufferObject);
    void                                CacheVBO(BufferObject *vbo);
    void                                CacheVkPipeline(VkPipeline pipeline);
    void                                CleanUpCaches();
};

#endif //__CACHEMANAGER_H__

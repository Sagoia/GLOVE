/**
 *  @file       cacheManager.h
 *  @author     Think Silicon
 *  @date       13/09/2018
 *  @version    1.0
 *
 *  @brief      Vulkan objects cache manager. These caches are needed to keep in memory Vulkan objects referred to by secondary command buffers.
 *
 */
#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <vector>
#include "vulkan/vulkan.h"
#include "utils/glLogger.h"
#include "resources/bufferObject.h"
#include "resources/texture.h"

class CacheManager {
private:
    const
    vulkanAPI::vkContext_t *            mVkContext;

    std::vector<UniformBufferObject *>  mUBOCache;
    std::vector<BufferObject *>         mVBOCache;
    std::vector<Texture *>              mTextureCache;
    std::vector<VkPipeline>             mVkPipelineObjectCache;

    void                                CleanUpUBOCache();
    void                                CleanUpVBOCache();
    void                                CleanUpTextureCache();
    void                                CleanUpVkPipelineObjectCache();

public:
     CacheManager(const vulkanAPI::vkContext_t *vkContext) : mVkContext(vkContext) { }
    ~CacheManager() { }

    void                                CacheUBO(UniformBufferObject *uniformBufferObject);
    void                                CacheVBO(BufferObject *vbo);
    void                                CacheTexture(Texture *tex);
    void                                CacheVkPipelineObject(VkPipeline pipeline);
    void                                CleanUpCaches();
};

#endif //__CACHEMANAGER_H__

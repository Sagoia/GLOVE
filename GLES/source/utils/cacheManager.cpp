/**
 *  @file       cacheManager.cpp
 *  @author     Think Silicon
 *  @date       13/09/2018
 *  @version    1.0
 *
 *  @brief      Vulkan objects cache manager. These caches are needed to keep in memory Vulkan objects referred to by secondary command buffers.
 *
 */

#include "cacheManager.h"

void
CacheManager::CleanUpUBOCache(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mUBOCache.empty()) {
        for(uint32_t i = 0; i < mUBOCache.size(); ++i) {
            if(mUBOCache[i] != nullptr) {
                delete mUBOCache[i];
                mUBOCache[i] = nullptr;
            }
        }

        mUBOCache.clear();
    }
}

void
CacheManager::CleanUpVBOCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVBOCache.empty()) {
        for(uint32_t i = 0; i < mVBOCache.size(); ++i) {
            if(mVBOCache[i] != nullptr) {
                delete mVBOCache[i];
                mVBOCache[i] = nullptr;
            }
        }

        mVBOCache.clear();
    }
}

void
CacheManager::CleanUpVkPipelineObjectCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVkPipelineObjectCache.empty()) {
        for(uint32_t i = 0; i < mVkPipelineObjectCache.size(); ++i) {
            if(mVkPipelineObjectCache[i] != VK_NULL_HANDLE){
                vkDestroyPipeline(mVkContext->vkDevice, mVkPipelineObjectCache[i], nullptr);
                mVkPipelineObjectCache[i] = VK_NULL_HANDLE;
            }
        }

        mVkPipelineObjectCache.clear();
    }
}

void
CacheManager::CacheUBO(UniformBufferObject *uniformBufferObject)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mUBOCache.push_back(uniformBufferObject);
}

void
CacheManager::CacheVBO(BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVBOCache.push_back(vbo);
}

void
CacheManager::CacheVkPipelineObject(VkPipeline pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkPipelineObjectCache.push_back(pipeline);
}

void
CacheManager::CleanUpCaches()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUpUBOCache();
    CleanUpVBOCache();
    CleanUpVkPipelineObjectCache();
}

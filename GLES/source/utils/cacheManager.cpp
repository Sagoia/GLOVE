/**
 *  @file       cacheManager.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief
 *
 */

#include "cacheManager.h"

void
CacheManager::CleanUpUniformBufferObjectCache(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(!mUniformBufferObjectCache.empty()) {
        for(uint32_t i = 0; i < mUniformBufferObjectCache.size(); ++i) {
            delete mUniformBufferObjectCache[i];
        }

        mUniformBufferObjectCache.clear();
    }
}

void
CacheManager::CleanUpVBOCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVBOCache.empty()) {
        for(uint32_t i = 0; i < mVBOCache.size(); ++i) {
            delete mVBOCache[i];
        }

        mVBOCache.clear();
    }
}

void
CacheManager::CleanUpVkPipelineCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVkPipelineCache.empty()) {
        for(uint32_t i = 0; i < mVkPipelineCache.size(); ++i) {
            vkDestroyPipeline(mVkContext->vkDevice, mVkPipelineCache[i], NULL);
        }

        mVkPipelineCache.clear();
    }
}

void
CacheManager::CacheUniformBufferObject(UniformBufferObject *uniformBufferObject)
{
    mUniformBufferObjectCache.push_back(uniformBufferObject);    
}

void
CacheManager::CacheVBO(BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_TRACE);
    mVBOCache.push_back(vbo);
}

void
CacheManager::CacheVkPipeline(VkPipeline pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);
    mVkPipelineCache.push_back(pipeline);
}

void
CacheManager::CleanUpCaches()
{
    CleanUpUniformBufferObjectCache();
    CleanUpVBOCache();
    CleanUpVkPipelineCache();
}

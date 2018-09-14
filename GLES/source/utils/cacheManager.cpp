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
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mUniformBufferObjectCache.empty()) {
        for(uint32_t i = 0; i < mUniformBufferObjectCache.size(); ++i) {
            if(mUniformBufferObjectCache[i] != nullptr) {
                delete mUniformBufferObjectCache[i];
                mUniformBufferObjectCache[i] = nullptr;
            }
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
            if(mVBOCache[i] != nullptr) {
                delete mVBOCache[i];
                mVBOCache[i] = nullptr;
            }
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
            if(mVkPipelineCache[i] != VK_NULL_HANDLE){
                vkDestroyPipeline(mVkContext->vkDevice, mVkPipelineCache[i], NULL);
                mVkPipelineCache[i] = VK_NULL_HANDLE;
            }
        }

        mVkPipelineCache.clear();
    }
}

void
CacheManager::CacheUniformBufferObject(UniformBufferObject *uniformBufferObject)
{
    FUN_ENTRY(GL_LOG_TRACE);

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
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUpUniformBufferObjectCache();
    CleanUpVBOCache();
    CleanUpVkPipelineCache();
}


#include "caches.h"

namespace vulkanAPI {

Caches::Caches(const vulkanAPI::XContext_t *xContext) 
: mXContext(xContext) 
{ 
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageViewCache.Reserve(DEFAULT_COUNT);
    mVkImageCache.Reserve(DEFAULT_COUNT);
    mVkBufferCache.Reserve(DEFAULT_COUNT);
    mVkDeviceMemoryCache.Reserve(DEFAULT_COUNT);
}

Caches::~Caches() 
{
    FUN_ENTRY(GL_LOG_TRACE);
}

void
Caches::CleanUpImageViewCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageViewCache.Empty()) {
        for (uint32_t i = 0; i < mVkImageViewCache.Size(); ++i) {
            vkDestroyImageView(mXContext->vkDevice, mVkImageViewCache[i], nullptr);
        }

        mVkImageViewCache.Clear();
    }
}

void
Caches::CleanUpImageCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageCache.Empty()) {
        for (uint32_t i = 0; i < mVkImageCache.Size(); ++i) {
            vkDestroyImage(mXContext->vkDevice, mVkImageCache[i], nullptr);
        }

        mVkImageCache.Clear();
    }
}

void
Caches::CleanUpBufferCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkBufferCache.Empty()) {
        for (uint32_t i = 0; i < mVkBufferCache.Size(); ++i) {
            vkDestroyBuffer(mXContext->vkDevice, mVkBufferCache[i], nullptr);
        }

        mVkBufferCache.Clear();
    }
}

void
Caches::CleanUpDeviceMemoryCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkDeviceMemoryCache.Empty()) {
        for (uint32_t i = 0; i < mVkDeviceMemoryCache.Size(); ++i) {
            vkFreeMemory(mXContext->vkDevice, mVkDeviceMemoryCache[i], nullptr);
        }

        mVkDeviceMemoryCache.Clear();
    }
}

void
Caches::CleanUpSampleCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkSamplerCache.empty()) {
        for (auto sampler : mVkSamplerCache) {
            vkDestroySampler(mXContext->vkDevice, sampler.second, nullptr);
        }

        mVkSamplerCache.clear();
    }
}

void
Caches::CleanUpRenderPassCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkRenderPassCache.empty()) {
        for (auto renderPass : mVkRenderPassCache) {
            vkDestroyRenderPass(mXContext->vkDevice, renderPass.second, nullptr);
        }

        mVkRenderPassCache.clear();
    }
}

void
Caches::CleanUpPipelineCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkPipelineCache.empty()) {
        for (auto pipelineMap : mVkPipelineCache) {
            for (auto pipeline : pipelineMap.second) {
                vkDestroyPipeline(mXContext->vkDevice, pipeline.second, nullptr);
            }
            pipelineMap.second.clear();
        }

        mVkRenderPassCache.clear();
    }
}

void
Caches::CacheVkImageView(VkImageView imageView)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageViewCache.PushBack(imageView);
}

void
Caches::CacheVkImage(VkImage image)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageCache.PushBack(image);
}

void
Caches::CacheVkBuffer(VkBuffer buffer)
{
    FUN_ENTRY(GL_LOG_TRACE);
    
    mVkBufferCache.PushBack(buffer);
}

void
Caches::CacheDeviceMemory(VkDeviceMemory deviceMemory)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkDeviceMemoryCache.PushBack(deviceMemory);
}

void
Caches::CacheSampler(uint64_t hash, VkSampler sampler)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkSamplerCache[hash] = sampler;
}

VkSampler
Caches::GetSampler(uint64_t hash)
{
    FUN_ENTRY(GL_LOG_TRACE);

    SamplerMap::iterator it = mVkSamplerCache.find(hash);
    if (it != mVkSamplerCache.end()) {
        return it->second;
    }

    return VK_NULL_HANDLE;
}

void
Caches::CacheRenderPass(uint64_t hash, VkRenderPass renderPass)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkRenderPassCache[hash] = renderPass;
}

VkRenderPass
Caches::GetRenderPass(uint64_t hash)
{
    FUN_ENTRY(GL_LOG_TRACE);

    RenderPassMap::iterator it = mVkRenderPassCache.find(hash);
    if (it != mVkRenderPassCache.end()) {
        return it->second;
    }

    return VK_NULL_HANDLE;
}

void
Caches::CachePipeline(VkPipelineCache pipelineCache, uint64_t hash, VkPipeline pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkPipelineCache[pipelineCache][hash] = pipeline;
}

VkPipeline
Caches::GetPipeline(VkPipelineCache pipelineCache, uint64_t hash)
{
    FUN_ENTRY(GL_LOG_TRACE);

    PipelineMap::iterator it = mVkPipelineCache.find(pipelineCache);
    if (it != mVkPipelineCache.end()) {
        PipelineHashMap::iterator it2 = it->second.find(hash);
        if (it2 != it->second.end()) {
            return it2->second;
        }
    }

    return VK_NULL_HANDLE;
}

}

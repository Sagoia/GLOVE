
#ifndef __VKCACHEMANAGER_H__
#define __VKCACHEMANAGER_H__

#include "context.h"
#include "arrays.hpp"
#include <unordered_map>

namespace vulkanAPI {

class Caches {
private:
    const static uint32_t DEFAULT_COUNT     = 256;

    const XContext_t *     mVkContext;

    Array<VkImageView>      mVkImageViewCache;
    Array<VkImage>          mVkImageCache;
    Array<VkBuffer>         mVkBufferCache;
    Array<VkDeviceMemory>   mVkDeviceMemoryCache;

    typedef std::unordered_map<uint64_t, VkSampler> SamplerMap;
    SamplerMap              mVkSamplerCache;
    typedef std::unordered_map<uint64_t, VkRenderPass> RenderPassMap;
    RenderPassMap           mVkRenderPassCache;

    typedef std::unordered_map<uint64_t, VkPipeline> PipelineHashMap;
    typedef std::unordered_map<VkPipelineCache, PipelineHashMap> PipelineMap;
    PipelineMap             mVkPipelineCache;

public:
     Caches(const vulkanAPI::XContext_t *vkContext);
    ~Caches();

    void                    CleanUpImageViewCache();
    void                    CleanUpImageCache();
    void                    CleanUpBufferCache();
    void                    CleanUpDeviceMemoryCache();

    void                    CleanUpSampleCache();
    void                    CleanUpRenderPassCache();
    void                    CleanUpPipelineCache();

    void                    CacheVkImageView(VkImageView imageView);
    void                    CacheVkImage(VkImage image);
    void                    CacheVkBuffer(VkBuffer buffer);
    void                    CacheDeviceMemory(VkDeviceMemory deviceMemory);

    void                    CacheSampler(uint64_t hash, VkSampler sampler);
    VkSampler               GetSampler(uint64_t hash);

    void                    CacheRenderPass(uint64_t hash, VkRenderPass renderPass);
    VkRenderPass            GetRenderPass(uint64_t hash);

    void                    CachePipeline(VkPipelineCache pipelineCache, uint64_t hash, VkPipeline pipeline);
    VkPipeline              GetPipeline(VkPipelineCache pipelineCache, uint64_t hash);
};

}

#endif // __VKCACHEMANAGER_H__

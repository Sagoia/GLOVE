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
 *  @file       screenSpacePass.h
 *  @author     Think Silicon
 *  @date       26/10/2018
 *  @version    1.0
 *
 *  @brief      Screen Space Vulkan Pass used for various operations (e.g., clear with ColorMask)
 *
 */

#ifndef __SCREENSPACEPASS_H__
#define __SCREENSPACEPASS_H__

#include "utils/glUtils.h"
#include "utils/glLogger.h"
#include "bufferObject.h"
#include "vulkan/context.h"
#include "vulkan/renderPass.h"
#include "vulkan/pipeline.h"
#include "vulkan/pipelineCache.h"
#include <utility>
#include <map>

class ScreenSpacePass {
private:

struct ScreenSpaceVertex {
    float pos[2];
};

struct UniformBufferObject_ScreenSpace {
    float color[4];
};
struct ShaderData {
    class ShaderCompiler                       *shaderCompiler;
    class ShaderProgram                        *shaderProgram;
    class Shader                               *vertShader;
    class Shader                               *fragShader;
    ShaderData():
    shaderCompiler(nullptr), shaderProgram(nullptr),
    vertShader(nullptr), fragShader(nullptr){

    }
    void InitResources(class Context *GLContext, CacheManager* cacheManager,
                       const vulkanAPI::vkContext_t *mVkContext);
    bool Generate(const std::string& vertexSource, const std::string& fragmentSource);
    void Destroy(void);
};

// ------------

    class Context                              *mGLContext;
    const vulkanAPI::vkContext_t               *mVkContext;
    CacheManager*                               mCacheManager;

    // shader
    ShaderData                                  mShaderData;

    // mesh
    uint32_t                                    mNumElements;
    VertexBufferObject                         *mVertexBuffer;
    std::vector<VkVertexInputBindingDescription> mVkBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> mVkAttributeDescriptions;
    VkBuffer                                    mVertexVkBuffer;
    VkDeviceSize                                mVertexVkBufferOffset;

    // pipeline/renderpass
    VkPipelineVertexInputStateCreateInfo        mVertexInputInfo;
    vulkanAPI::PipelineCache                   *mPipelineCache;
    vulkanAPI::Pipeline*                        mPipeline;

    // buffers

    bool                                        mInitialized;
    bool                                        mValid;

    bool                                        CreateShaderData();
    bool                                        CreateMeshData();

    bool                                        Destroy();
    bool                                        DestroyMeshData();
    bool                                        DestroyShaderData();

    bool                                        CreateDescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlagBits flags, uint32_t binding, uint32_t count);
    bool                                        CreateLayoutInfo();

    bool                                        CreateDescriptorPool(std::vector<VkDescriptorPoolSize>& poolSizes);
    bool                                        AllocateDescriptorSets(VkDescriptorSet& set, std::vector<VkDescriptorSetLayout>& layouts);
    bool                                        UpdateDescriptorSets(std::vector<VkWriteDescriptorSet>& descriptorWrites);
    bool                                        CreateWriteDescriptorSet(VkWriteDescriptorSet& writeSet, VkDescriptorSet& set,
                                                         uint32_t dstBinding, uint32_t dstArrayElement,
                                                         VkDescriptorType descriptorType,
                                                         std::vector<VkDescriptorImageInfo>* pImageInfos,
                                                         std::vector<VkDescriptorBufferInfo>* pBufferInfos,
                                                         std::vector<VkBufferView>* pTexelBufferView);
    bool                                        DestroyDescriptorPool();
    bool                                        DestroyDescriptorSets();

public:
    ScreenSpacePass(class Context *GLContext, const vulkanAPI::vkContext_t *vkContext);
    ~ScreenSpacePass();

    bool                                        Initialize();
    bool                                        CreateDefaultPipelineStates();
    void                                        BindVertexBuffers(const VkCommandBuffer *cmdBuffer) const;
    void                                        BindUniformDescriptors(const VkCommandBuffer *cmdBuffer) const;
    void                                        BindPipeline(const VkCommandBuffer *cmdBuffer) const;
    void                                        Draw(const VkCommandBuffer *cmdBuffer) const;
    bool                                        UpdateUniformBufferColor(float r, float g, float b, float a);

// Get Functions
    inline bool                                 Valid()                           {  FUN_ENTRY(GL_LOG_TRACE); return mValid; }
    inline vulkanAPI::Pipeline*                 GetPipeline()                     {  FUN_ENTRY(GL_LOG_TRACE); return mPipeline; }

// Set Functions
    void                                        SetCacheManager(CacheManager* cacheManager);

};

#endif // __SCREENSPACEPASS_H__

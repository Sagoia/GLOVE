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
#include "vulkan/cbManager.h"
#include "vulkan/renderPass.h"
#include "vulkan/pipeline.h"
#include "vulkan/pipelineCache.h"
#include <utility>
#include <map>

class ScreenSpacePass {

private:
// ------------

    VkRenderPass m_vkrenderPass;
    const vulkanAPI::vkContext_t               *mVkContext;
    vulkanAPI::CommandBufferManager            *mCommandBufferManager;

    // shader
    VkShaderModule                              mVkVertShaderModule;
    VkShaderModule                              mVkFragShaderModule;
    std::vector<VkPipelineShaderStageCreateInfo> mVkshaderStagesList;
    VkDescriptorSetLayout                       mVkDescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding>   mVkLayoutBindings;
    VkDescriptorPool                            mVkDescriptorPool;
    std::vector<VkDescriptorSet>                mVkDescriptorSetsList;
    VkDescriptorSet                             mVKDescriptorSet;

    // mesh
    uint32_t                                    mNumElements;
    VertexBufferObject                         *mVertexBuffer;
    std::vector<VkVertexInputBindingDescription> mVkBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> mVkAttributeDescriptions;

    // pipeline/renderpass
    VkPipelineVertexInputStateCreateInfo        mVertexInputInfo;
    VkPipelineLayoutCreateInfo                  mPipelineLayoutInfo;
    vulkanAPI::PipelineCache                   *mPipelineCache;
    vulkanAPI::Pipeline*                        mPipeline;
    VkPipelineLayout                            mVkPipelineLayout;

    // buffers
    UniformBufferObject*                        mClearColorUBO;
    std::vector<VkDescriptorSetLayout>          mUniformVkDescriptorSetLayoutList;
    std::vector<VkDescriptorBufferInfo>         mUniformVkDescriptorBufferInfoList;
    std::vector<VkDescriptorImageInfo>          mUniformVkDescriptorImageInfoList;

    bool                                        CreateShaderData();
    bool                                        CreateMeshData();
    bool                                        CreateUniformData();

    bool                                        Destroy();
    bool                                        DestroyMeshData();
    bool                                        DestroyShaderData();
    bool                                        DestroyUniformBufferData();

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
    ScreenSpacePass(const vulkanAPI::vkContext_t *vkContext, vulkanAPI::CommandBufferManager *cbManager);
    ~ScreenSpacePass();

    bool                                        Initialize();
    bool                                        CreateDefaultPipelineStates();
    void                                        BindVertexBuffers(const VkCommandBuffer *cmdBuffer) const;
    void                                        BindUniformDescriptors(const VkCommandBuffer *cmdBuffer) const;
    void                                        BindPipeline(const VkCommandBuffer *cmdBuffer) const;
    void                                        Draw(const VkCommandBuffer *cmdBuffer) const;
    bool                                        UpdateUniformBufferColor(float r, float g, float b, float a);

// Get Functions
    vulkanAPI::Pipeline*                        GetPipeline()                     { return mPipeline; }

// Set Functions

};

#endif // __SCREENSPACEPASS_H__

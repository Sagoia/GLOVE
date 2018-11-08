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

#include "screenSpacePass.h"
#include "utils/VkToGlConverter.h"
#include "shaderProgram.h"
#include <fstream>

struct ScreenSpaceVertex {
    float pos[2];
};

struct UniformBufferObject_ScreenSpace {
    float color[4];
};

ScreenSpacePass::ScreenSpacePass(const vulkanAPI::vkContext_t *vkContext, vulkanAPI::CommandBufferManager *cbManager):
    mVkContext(vkContext), mCommandBufferManager(cbManager),
    mVkVertShaderModule(VK_NULL_HANDLE), mVkFragShaderModule(VK_NULL_HANDLE),
    mVkDescriptorSetLayout(VK_NULL_HANDLE), mVkDescriptorPool(VK_NULL_HANDLE),
    mVKDescriptorSet(VK_NULL_HANDLE),
    mNumElements(0), mVertexBuffer(nullptr),
    mVertexInputInfo(), mPipelineLayoutInfo(),
    mPipelineCache(new vulkanAPI::PipelineCache(mVkContext)), mPipeline(new vulkanAPI::Pipeline(vkContext)),
    mVkPipelineLayout(VK_NULL_HANDLE), mClearColorUBO(nullptr)
{

}

ScreenSpacePass::~ScreenSpacePass()
{
    Destroy();
}

bool ScreenSpacePass::CreateMeshData()
{
    // the rate to load data from memory throughout the vertices
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ScreenSpaceVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    mVkBindingDescriptions.push_back(bindingDescription);

    // vertex attribute information
    const int num_attributes = 1;
    std::array<VkVertexInputAttributeDescription, num_attributes> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(ScreenSpaceVertex, pos);
    mVkAttributeDescriptions.push_back(attributeDescriptions[0]);

    float pos = 1.0f;
    std::vector<ScreenSpaceVertex> vertices =  {
     // bottom left
     //   {{0.0, -0.5}},
     //   {{0.5, 0.5}},
     //   {{-0.5, 0.5}}
     // bottom left
     {{-pos, -pos}},
     // bottom right
     {{pos,  -pos}},
     // top right
      {{pos,  pos}},
     // top right
      {{pos,  pos}},
     // top left
      {{-pos, pos}},
     // bottom left
      {{-pos, -pos}}
     };

    mNumElements = static_cast<uint32_t>(vertices.size());

    mVertexBuffer = new VertexBufferObject(mVkContext);
    mVertexBuffer->Allocate(mNumElements * sizeof(ScreenSpaceVertex), vertices.data());

    return true;
}

bool ScreenSpacePass::DestroyMeshData()
{
    if(mVertexBuffer) {
        mVertexBuffer->Release();
    }
    delete mVertexBuffer;
    mVertexBuffer = nullptr;
    return true;
}

bool
loadProgramBinary(const char* myBinaryFileName, VkDevice device, VkShaderModule& module)
{
    std::string filenameWithPath = std::string(ASSET_PATH).append(myBinaryFileName);
    std::ifstream input(filenameWithPath.c_str(), std::ios::in|std::ios::binary);
    if(!input.is_open()) {
        GLOVE_PRINT_ERR("Could not load spv file: %s\n", filenameWithPath.c_str());
        return false;
    }
    std::string bin = std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bin.length();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bin.c_str());

    VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &module);
    if(res != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool ScreenSpacePass::CreateShaderData()
{
    if(!loadProgramBinary("assets/screenspace.vert.spv", mVkContext->vkDevice, mVkVertShaderModule)) {
        return false;
    }
    if(!loadProgramBinary("assets/screenspace.frag.spv", mVkContext->vkDevice, mVkFragShaderModule)) {
        return false;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = mVkVertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;
    mVkshaderStagesList.push_back(vertShaderStageInfo);

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = mVkFragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;
    mVkshaderStagesList.push_back(fragShaderStageInfo);

    if(mPipelineCache->GetPipelineCache() == VK_NULL_HANDLE) {
        mPipelineCache->Create(nullptr, 0);
        mPipeline->SetCache(mPipelineCache->GetPipelineCache());
    }

    return true;
}

bool ScreenSpacePass::DestroyShaderData()
{
    vkDestroyShaderModule(mVkContext->vkDevice, mVkVertShaderModule, nullptr);
    mVkVertShaderModule = VK_NULL_HANDLE;
    vkDestroyShaderModule(mVkContext->vkDevice, mVkFragShaderModule, nullptr);
    mVkFragShaderModule = VK_NULL_HANDLE;
    return true;
}

bool ScreenSpacePass::CreateUniformData()
{
    mClearColorUBO = new UniformBufferObject(mVkContext);
    VkDeviceSize bufferSize = sizeof(UniformBufferObject_ScreenSpace);
    mClearColorUBO->Allocate(bufferSize, nullptr);

    // VkDescriptorSetLayoutBinding
    CreateDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1);
    // m_vkdescriptorSetLayout
    CreateLayoutInfo();
    mUniformVkDescriptorSetLayoutList = { mVkDescriptorSetLayout };

    std::vector<VkDescriptorPoolSize> poolSizes {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
    };

    if (!CreateDescriptorPool(poolSizes)) {
        return false;
    }
    AllocateDescriptorSets(mVKDescriptorSet, mUniformVkDescriptorSetLayoutList);
    mUniformVkDescriptorBufferInfoList = { *mClearColorUBO->GetBufferDescInfo() };
    VkWriteDescriptorSet uboSet;
    CreateWriteDescriptorSet(uboSet, mVKDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr,
                             &mUniformVkDescriptorBufferInfoList, nullptr);
    std::vector<VkWriteDescriptorSet> writeSets = { uboSet };
    UpdateDescriptorSets(writeSets);

    return true;
}

bool ScreenSpacePass::CreateDescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlagBits flags, uint32_t binding, uint32_t count)
{
    VkDescriptorSetLayoutBinding setLayoutBinding = {};
    setLayoutBinding.binding = binding;
    setLayoutBinding.descriptorType = type;
    setLayoutBinding.descriptorCount = count;
    setLayoutBinding.stageFlags = flags;
    setLayoutBinding.pImmutableSamplers = nullptr; // Optional

    mVkLayoutBindings.push_back(setLayoutBinding);
    return true;
}

bool ScreenSpacePass::CreateLayoutInfo()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(mVkLayoutBindings.size());
    layoutInfo.pBindings = mVkLayoutBindings.data();

    VkResult res = vkCreateDescriptorSetLayout(mVkContext->vkDevice, &layoutInfo, nullptr, &mVkDescriptorSetLayout);
    return res == VK_SUCCESS;
}

bool ScreenSpacePass::CreateDescriptorPool(std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkResult res = vkCreateDescriptorPool(mVkContext->vkDevice, &poolInfo, nullptr, &mVkDescriptorPool);
    return res == VK_SUCCESS;
}

bool ScreenSpacePass::AllocateDescriptorSets(VkDescriptorSet& set, std::vector<VkDescriptorSetLayout>& layouts)
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mVkDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    VkResult res = vkAllocateDescriptorSets(mVkContext->vkDevice, &allocInfo, &set);
    mVkDescriptorSetsList.push_back(set);
    return res == VK_SUCCESS;
}

bool ScreenSpacePass::CreateWriteDescriptorSet(VkWriteDescriptorSet& writeSet, VkDescriptorSet& set,
                                           uint32_t dstBinding, uint32_t dstArrayElement,
                                           VkDescriptorType descriptorType, std::vector<VkDescriptorImageInfo>* pImageInfos,
                                           std::vector<VkDescriptorBufferInfo>* pBufferInfos, std::vector<VkBufferView>* pTexelBufferView)
{
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.pNext = nullptr;
    writeSet.dstSet = set;
    writeSet.dstBinding = dstBinding;
    writeSet.dstArrayElement = dstArrayElement;
    writeSet.descriptorType = descriptorType;
    writeSet.descriptorCount = 0;
    writeSet.pImageInfo = nullptr;
    writeSet.pBufferInfo = nullptr;
    writeSet.pTexelBufferView = nullptr;
    if(pImageInfos != nullptr) {
        writeSet.descriptorCount = static_cast<uint32_t>(pImageInfos->size());
        writeSet.pImageInfo = pImageInfos->data();
    } else if(pBufferInfos != nullptr) {
        writeSet.descriptorCount = static_cast<uint32_t>(pBufferInfos->size());
        writeSet.pBufferInfo = pBufferInfos->data();
    } else if(pTexelBufferView != nullptr) {
        writeSet.descriptorCount = static_cast<uint32_t>(pTexelBufferView->size());
        writeSet.pTexelBufferView = pTexelBufferView->data();
    }
    return true;
}

bool ScreenSpacePass::UpdateDescriptorSets(std::vector<VkWriteDescriptorSet>& descriptorWrites)
{
    vkUpdateDescriptorSets(mVkContext->vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    return true;
}

bool ScreenSpacePass::DestroyUniformBufferData()
{
    delete mClearColorUBO;
    mClearColorUBO = nullptr;

    DestroyDescriptorSets();
    DestroyDescriptorPool();

    mUniformVkDescriptorBufferInfoList.clear();
    mUniformVkDescriptorImageInfoList.clear();
    mUniformVkDescriptorSetLayoutList.clear();
    return true;
}

bool ScreenSpacePass::DestroyDescriptorPool()
{
    if(mVkDescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(mVkContext->vkDevice, mVkDescriptorPool, nullptr);
        mVkDescriptorPool = VK_NULL_HANDLE;
    }
    return true;
}

bool ScreenSpacePass::DestroyDescriptorSets()
{
    if(mVkDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(mVkContext->vkDevice, mVkDescriptorSetLayout, nullptr);
        mVkDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if(mVKDescriptorSet != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(mVkContext->vkDevice, mVkDescriptorPool, 1, &mVKDescriptorSet);
        mVKDescriptorSet = VK_NULL_HANDLE;
    }
    mVkDescriptorSetsList.clear();
    return true;
}

bool ScreenSpacePass::Initialize()
{
    if(!CreateShaderData()) {
        return false;
    }

    if(!CreateMeshData()) {
        return false;
    }

    if(!CreateUniformData()) {
        return false;
    }

    return true;
}

bool
ScreenSpacePass::CreateDefaultPipelineStates()
{
    // the format of the vertex data that will be passed to the vertex shader
    mVertexInputInfo = {};
    mVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    mVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(mVkBindingDescriptions.size());
    mVertexInputInfo.pVertexBindingDescriptions = mVkBindingDescriptions.data();
    mVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(mVkAttributeDescriptions.size());
    mVertexInputInfo.pVertexAttributeDescriptions = mVkAttributeDescriptions.data();


    mPipeline->SetVertexInputState(&mVertexInputInfo);
    mPipeline->CreateInputAssemblyState(VK_FALSE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    mPipeline->CreateRasterizationState(VK_POLYGON_MODE_FILL,
                                         VK_CULL_MODE_FRONT_AND_BACK,
                                         VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                         VK_FALSE, 0.0f, 0.0f,
                                         0.0f, VK_FALSE, VK_FALSE);

    std::vector<VkDynamicState> states = {VK_DYNAMIC_STATE_VIEWPORT,
                                          VK_DYNAMIC_STATE_SCISSOR,
                                          VK_DYNAMIC_STATE_LINE_WIDTH};
    mPipeline->CreateDynamicState(states);
    GLfloat     blendcolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f};

    mPipeline->CreateColorBlendState(VK_FALSE,
                                     VK_COLOR_COMPONENT_R_BIT |
                                     VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT |
                                     VK_COLOR_COMPONENT_A_BIT,
                                     VK_BLEND_FACTOR_ONE,
                                     VK_BLEND_FACTOR_ZERO,
                                     VK_BLEND_FACTOR_ONE,
                                     VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_BLEND_OP_ADD,
                                    VK_LOGIC_OP_COPY,
                                    VK_FALSE,
                                    1,
                                    &blendcolor[0]);

    // multisampling
    mPipeline->CreateMultisampleState(VK_FALSE, VK_FALSE, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f);

    // to set shader uniforms (an empty one is needed if no uniforms are used)
    mPipelineLayoutInfo = {};
    mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    mPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(mUniformVkDescriptorSetLayoutList.size());
    mPipelineLayoutInfo.pSetLayouts = mUniformVkDescriptorSetLayoutList.data();
    mPipelineLayoutInfo.pushConstantRangeCount = 0;
    mPipelineLayoutInfo.pPushConstantRanges = nullptr;

    VkResult res = vkCreatePipelineLayout(mVkContext->vkDevice, &mPipelineLayoutInfo, nullptr, &mVkPipelineLayout);
    if (res != VK_SUCCESS)
    {
        return false;
    }

    mPipeline->SetLayout(mVkPipelineLayout);
    mPipeline->GetShaderStageCountRef() = 2;
    mPipeline->GetShaderStages()[0] = mVkshaderStagesList[0];
    mPipeline->GetShaderStages()[1] = mVkshaderStagesList[1];
    mPipeline->SetDepthTestEnable(false);
    mPipeline->SetDepthCompareOp(VK_COMPARE_OP_ALWAYS);
    mPipeline->SetStencilBackCompareOp(VK_COMPARE_OP_ALWAYS);
    mPipeline->SetStencilBackFailOp(VK_STENCIL_OP_KEEP);
    mPipeline->SetStencilBackZFailOp(VK_STENCIL_OP_KEEP);
    mPipeline->SetStencilBackPassOp(VK_STENCIL_OP_REPLACE);
    mPipeline->SetStencilFrontCompareOp(VK_COMPARE_OP_ALWAYS);
    mPipeline->SetStencilFrontFailOp(VK_STENCIL_OP_KEEP);
    mPipeline->SetStencilFrontZFailOp(VK_STENCIL_OP_KEEP);
    mPipeline->SetStencilFrontPassOp(VK_STENCIL_OP_REPLACE);

    return res == VK_SUCCESS;
}

bool
ScreenSpacePass::Destroy()
{
   DestroyShaderData();
   DestroyMeshData();
   DestroyUniformBufferData();
   vkDestroyPipelineLayout(mVkContext->vkDevice, mVkPipelineLayout, nullptr);
   mVkPipelineLayout = VK_NULL_HANDLE;
   if(mPipeline != nullptr) {
       delete mPipeline;
       mPipeline = nullptr;
   }
   if(mPipelineCache) {
       delete mPipelineCache;
       mPipelineCache = nullptr;
   }
   return true;
}

bool
ScreenSpacePass::UpdateUniformBufferColor(float r, float g, float b, float a)
{
    UniformBufferObject_ScreenSpace ubo = { r,g,b,a };
    mClearColorUBO->UpdateData(sizeof(ubo), 0, static_cast<void*>(&ubo));

    return true;
}

void
ScreenSpacePass::BindPipeline(const VkCommandBuffer *cmdBuffer) const
{
    mPipeline->Bind(cmdBuffer);
}

void
ScreenSpacePass::BindVertexBuffers(const VkCommandBuffer *cmdBuffer) const
{
    // bind vertex buffers
    VkBuffer vertexBuffers[] = { mVertexBuffer->GetVkBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(*cmdBuffer, 0, 1, vertexBuffers, offsets);
}

void
ScreenSpacePass::BindUniformDescriptors(const VkCommandBuffer *cmdBuffer) const
{
    vkCmdBindDescriptorSets(*cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mVkPipelineLayout, 0,
                            static_cast<uint32_t>(mVkDescriptorSetsList.size()), mVkDescriptorSetsList.data(), 0, nullptr);
}

void
ScreenSpacePass::Draw(const VkCommandBuffer *cmdBuffer) const
{
    vkCmdDraw(*cmdBuffer, mNumElements, 1, 0, 0);
}

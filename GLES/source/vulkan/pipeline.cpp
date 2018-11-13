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
 *  @file       pipeline.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Graphics Pipeline Functionality in Vulkan
 *
 *  @section
 *
 *  Graphics pipelines consist of multiple shader stages, multiple fixed-function
 *  pipeline stages, and a pipeline layout.
 *
 */

#include "pipeline.h"

namespace vulkanAPI {

Pipeline::Pipeline(const vkContext_t *vkContext)
: mVkContext(vkContext), mVkPipeline(VK_NULL_HANDLE), mVkPipelineLayout(VK_NULL_HANDLE),
  mVkPipelineCache(VK_NULL_HANDLE), mVkPipelineVertexInputState(VK_NULL_HANDLE),
  mVkPipelineShaderStageCount(0), mCacheManager(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkPipelineShaderStageIDs[0]  = -1;
    mVkPipelineShaderStageIDs[1]  = -1;

    mUpdateState.VertexAttribVBOs = true;
    mUpdateState.IndexBuffer      = false;
    mUpdateState.Viewport         = true;
    mUpdateState.Pipeline         = true;

    mEnabledDynamicStatesList.resize(VK_DYNAMIC_STATE_RANGE_SIZE);
}

Pipeline::~Pipeline()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
Pipeline::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    mVkViewport.x = x;
    mVkViewport.y = y;
    mVkViewport.width = width;
    mVkViewport.height = height;
}

void
Pipeline::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
    mVkScissorRect.offset.x = x;
    mVkScissorRect.offset.y = y;
    mVkScissorRect.extent.width = width;
    mVkScissorRect.extent.height = height;
}

void
Pipeline::MoveToCache()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkPipeline != VK_NULL_HANDLE) {
        mCacheManager->CacheVkPipelineObject(mVkPipeline);
        mVkPipeline = VK_NULL_HANDLE;
    }
}

void
Pipeline::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(mVkContext->vkDevice, mVkPipeline, nullptr);
        mVkPipeline = VK_NULL_HANDLE;
    }
}

void
Pipeline::ComputeViewport(int fboWidth, int fboHeight, int viewportX, int viewportY, int viewportW, int viewportH, float minDepth, float maxDepth)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    viewportW = std::min(viewportW, fboWidth);
    viewportH = std::min(viewportH, fboHeight);
    int viewportYinv = 0, viewportHinv = 0;
    if(mVkContext->mIsMaintenanceExtSupported) {
        viewportYinv = fboHeight - viewportY;
        viewportHinv = -viewportH;
    } else {
        viewportYinv = viewportY;
        viewportHinv = viewportH;
    }

    mVkViewport = {
                   (float)viewportX, (float)viewportYinv,
                   (float)viewportW, (float)viewportHinv,
                   minDepth , maxDepth};
}

void
Pipeline::ComputeScissor(int fboWidth, int fboHeight, int scissorX, int scissorY, int scissorW, int scissorH)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    scissorW = std::min(scissorW, fboWidth);
    scissorH = std::min(scissorH, fboHeight);

    int scissorYinv = fboHeight - scissorY - scissorH;

    mVkScissorRect  = {
                        {scissorX, scissorYinv},
                        {(uint32_t)scissorW, (uint32_t)scissorH}
                      };
}

void
Pipeline::CreateInfo()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineInfo), 0, sizeof(mVkPipelineInfo));
    mVkPipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    mVkPipelineInfo.pNext               = nullptr;
    mVkPipelineInfo.flags               = 0;
    mVkPipelineInfo.pInputAssemblyState = &mVkPipelineInputAssemblyState;
    mVkPipelineInfo.pRasterizationState = &mVkPipelineRasterizationState;
    mVkPipelineInfo.pColorBlendState    = &mVkPipelineColorBlendState;
    mVkPipelineInfo.pMultisampleState   = &mVkPipelineMultisampleState;
    mVkPipelineInfo.pViewportState      = &mVkPipelineViewportState;
    mVkPipelineInfo.pDepthStencilState  = &mVkPipelineDepthStencilState;
    mVkPipelineInfo.pDynamicState       = &mVkPipelineDynamicState;
}

void
Pipeline::CreateInputAssemblyState(VkBool32 primitiveRestartEnable, VkPrimitiveTopology topology)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineInputAssemblyState), 0, sizeof(mVkPipelineInputAssemblyState));
    mVkPipelineInputAssemblyState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    mVkPipelineInputAssemblyState.pNext                  = nullptr;
    mVkPipelineInputAssemblyState.flags                  = 0;
    mVkPipelineInputAssemblyState.primitiveRestartEnable = primitiveRestartEnable;

    SetInputAssemblyTopology(topology);
}

void
Pipeline::CreateRasterizationState(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace,
  VkBool32 depthBiasEnable, float depthBiasConstantFactor, float depthBiasSlopeFactor, float depthBiasClamp,
  VkBool32 depthClampEnable, VkBool32 rasterizerDiscardEnable)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineRasterizationState), 0, sizeof(mVkPipelineRasterizationState));
    mVkPipelineRasterizationState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    mVkPipelineRasterizationState.pNext                   = nullptr;
    mVkPipelineRasterizationState.flags                   = 0;
    mVkPipelineRasterizationState.depthClampEnable        = depthClampEnable;
    mVkPipelineRasterizationState.depthBiasClamp          = depthBiasClamp;
    mVkPipelineRasterizationState.rasterizerDiscardEnable = rasterizerDiscardEnable;
    mVkPipelineRasterizationState.lineWidth               = 1.0f;

    SetRasterizationPolygonMode(polygonMode);
    SetRasterizationCullMode(VK_FALSE, cullMode);
    SetRasterizationFrontFace(frontFace);

    SetRasterizationDepthBiasEnable(depthBiasEnable);
    SetRasterizationDepthBiasConstantFactor(depthBiasConstantFactor);
    SetRasterizationDepthBiasSlopeFactor(depthBiasSlopeFactor);
}

void
Pipeline::CreateColorBlendState(VkBool32 blendEnable, VkColorComponentFlags colorWriteMask,
    VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor,
    VkBlendOp colorBlendOp, VkBlendOp alphaBlendOp,
    VkLogicOp logicOp, VkBool32 logicOpEnable, uint32_t attachmentCount, float *blendConstants)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineColorBlendAttachmentState), 0, sizeof(mVkPipelineColorBlendAttachmentState));

    SetColorBlendAttachmentEnable(blendEnable);
    SetColorBlendAttachmentWriteMask(colorWriteMask);

    SetColorBlendAttachmentSrcColorFactor(srcColorBlendFactor);
    SetColorBlendAttachmentDstColorFactor(dstColorBlendFactor);
    SetColorBlendAttachmentSrcAlphaFactor(srcAlphaBlendFactor);
    SetColorBlendAttachmentDstAlphaFactor(dstAlphaBlendFactor);

    SetColorBlendAttachmentColorOp(colorBlendOp);
    SetColorBlendAttachmentAlphaOp(alphaBlendOp);

    memset(static_cast<void *>(&mVkPipelineColorBlendState), 0, sizeof(mVkPipelineColorBlendState));
    mVkPipelineColorBlendState.sType            = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    mVkPipelineColorBlendState.pNext            = nullptr;
    mVkPipelineColorBlendState.flags            = 0;
    mVkPipelineColorBlendState.logicOp          = logicOp;
    mVkPipelineColorBlendState.logicOpEnable    = logicOpEnable;
    mVkPipelineColorBlendState.attachmentCount  = attachmentCount;
    mVkPipelineColorBlendState.pAttachments     = &mVkPipelineColorBlendAttachmentState;

    SetColorBlendConstants(blendConstants);
}

void
Pipeline::CreateDepthStencilState(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthBoundsTestEnable,
                                  float minDepthBounds, float maxDepthBounds, VkBool32  stencilTestEnable, VkStencilOp backfailOp, VkStencilOp backpassOp,
                                  VkStencilOp backdepthFailOp, uint32_t backwriteMask, VkCompareOp backcompareOp, uint32_t backcompareMask, uint32_t backreference,
                                  VkStencilOp frontfailOp, VkStencilOp frontpassOp, VkStencilOp frontdepthFailOp, uint32_t frontwriteMask, VkCompareOp frontcompareOp,
                                  uint32_t  frontcompareMask, uint32_t frontreference)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineDepthStencilState), 0, sizeof(mVkPipelineDepthStencilState));
    mVkPipelineDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    mVkPipelineDepthStencilState.pNext = nullptr;
    mVkPipelineDepthStencilState.flags = 0;

    SetDepthTestEnable        (depthTestEnable);
    SetDepthWriteEnable       (depthWriteEnable);
    SetDepthCompareOp         (depthCompareOp);
    SetDepthBoundsTestEnable  (depthBoundsTestEnable);
    SetMinDepthBounds         (minDepthBounds);
    SetMaxDepthBounds         (maxDepthBounds);

    SetStencilTestEnable      (stencilTestEnable);
    SetStencilBackFailOp      (backfailOp);
    SetStencilBackPassOp      (backpassOp);
    SetStencilBackZFailOp     (backdepthFailOp);
    SetStencilBackWriteMask   (backwriteMask);
    SetStencilBackCompareOp   (backcompareOp);
    SetStencilBackCompareMask (backcompareMask);
    SetStencilBackReference   (backreference);

    SetStencilFrontFailOp     (frontfailOp);
    SetStencilFrontPassOp     (frontpassOp);
    SetStencilFrontZFailOp    (frontdepthFailOp);
    SetStencilFrontWriteMask  (frontwriteMask);
    SetStencilFrontCompareOp  (frontcompareOp);
    SetStencilFrontCompareMask(frontcompareMask);
    SetStencilFrontReference  (frontreference);
}

void
Pipeline::CreateViewportState(uint32_t viewportCount, uint32_t scissorCount)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineViewportState), 0, sizeof(mVkPipelineViewportState));
    mVkPipelineViewportState.sType          = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    mVkPipelineViewportState.pNext          = nullptr;
    mVkPipelineViewportState.flags          = 0;
    mVkPipelineViewportState.viewportCount  = viewportCount;
    mVkPipelineViewportState.scissorCount   = scissorCount;
}

void
Pipeline::CreateMultisampleState(VkBool32 alphaToOneEnable, VkBool32 alphaToCoverageEnable, VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable, float minSampleShading)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    memset(static_cast<void *>(&mVkPipelineMultisampleState), 0, sizeof(mVkPipelineMultisampleState));
    mVkPipelineMultisampleState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mVkPipelineMultisampleState.pNext                 = nullptr;
    mVkPipelineMultisampleState.flags                 = 0;
    mVkPipelineMultisampleState.alphaToOneEnable      = alphaToOneEnable;
    mVkPipelineMultisampleState.rasterizationSamples  = rasterizationSamples;
    mVkPipelineMultisampleState.sampleShadingEnable   = sampleShadingEnable;
    mVkPipelineMultisampleState.minSampleShading      = minSampleShading;
    mVkPipelineMultisampleState.pSampleMask           = nullptr; // TODO: GetSampleCoverageValue()

    SetMultisampleAlphaToCoverage(alphaToCoverageEnable);
}

void
Pipeline::CreateDynamicState(const std::vector<VkDynamicState>& states)
{
    FUN_ENTRY(GL_LOG_DEBUG);
    assert(states.size() <= VK_DYNAMIC_STATE_RANGE_SIZE);
    for(size_t index = 0; index < mEnabledDynamicStatesList.size(); ++index) {
        mEnabledDynamicStatesList[index] = false;
    }
    memset(mVkPipelineDynamicStateEnables, 0, sizeof(mVkPipelineDynamicStateEnables));

    for(size_t stateIndex = 0; stateIndex < states.size(); ++stateIndex) {
        VkDynamicState state = states[stateIndex];
        mVkPipelineDynamicStateEnables[stateIndex] = state;
        mEnabledDynamicStatesList[state] = true;
    }

    memset(static_cast<void *>(&mVkPipelineDynamicState), 0, sizeof(mVkPipelineDynamicState));
    mVkPipelineDynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    mVkPipelineDynamicState.pNext             = nullptr;
    mVkPipelineDynamicState.dynamicStateCount = static_cast<uint32_t>(states.size());
    mVkPipelineDynamicState.pDynamicStates    = mVkPipelineDynamicStateEnables;
}

void
Pipeline::UpdateDynamicState(const VkCommandBuffer *CmdBuffer, float lineWidth) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mEnabledDynamicStatesList[VK_DYNAMIC_STATE_VIEWPORT]) {
        vkCmdSetViewport  (*CmdBuffer, 0, mVkPipelineViewportState.viewportCount, &mVkViewport);
    }
    if(mEnabledDynamicStatesList[VK_DYNAMIC_STATE_SCISSOR]) {
        vkCmdSetScissor   (*CmdBuffer, 0, mVkPipelineViewportState.scissorCount , &mVkScissorRect);
    }
    if(mEnabledDynamicStatesList[VK_DYNAMIC_STATE_LINE_WIDTH]) {
        vkCmdSetLineWidth (*CmdBuffer, lineWidth);
    }
    /*
    TODO:: fill the remaining dynamic states
    VK_DYNAMIC_STATE_BLEND_CONSTANTS
    VK_DYNAMIC_STATE_DEPTH_BOUNDS
    VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK
    VK_DYNAMIC_STATE_STENCIL_WRITE_MASK
    VK_DYNAMIC_STATE_STENCIL_REFERENCE
    */
}

void
Pipeline::SetInfo(const VkRenderPass *renderpass)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mVkPipelineInfo.layout              = mVkPipelineLayout;
    mVkPipelineInfo.pVertexInputState   = mVkPipelineVertexInputState;
    mVkPipelineInfo.pStages             = mVkPipelineShaderStages;
    mVkPipelineInfo.stageCount          = mVkPipelineShaderStageCount;
    mVkPipelineInfo.renderPass          = *renderpass;
}

void
Pipeline::Bind(const VkCommandBuffer *CmdBuffer) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    vkCmdBindPipeline(*CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mVkPipeline);
}

bool
Pipeline::Create(const VkRenderPass *renderpass)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mUpdateState.Pipeline) {
        SetInfo(renderpass);
        return CreateGraphicsPipeline();
    }

    return true;
}

bool
Pipeline::CreateGraphicsPipeline(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    MoveToCache();

    VkResult err = vkCreateGraphicsPipelines(mVkContext->vkDevice, mVkPipelineCache, 1, &mVkPipelineInfo, nullptr, &mVkPipeline);
    assert(!err);
    
    mUpdateState.Pipeline = false;

    return (err == VK_SUCCESS);
}

}

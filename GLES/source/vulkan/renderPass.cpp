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
 *  @file       renderPass.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Render pass Functionality in Vulkan
 *
 *  @section
 *
 *  A render pass represents a collection of attachments, subpasses, and
 *  dependencies between the subpasses, and describes how the attachments
 *  are used over the course of the subpasses.
 *  Render passes are represented by VkRenderPass handles.
 *
 */

#include "renderPass.h"
#include "utils.h"

namespace vulkanAPI {

RenderPass::RenderPass(const vkContext_t *vkContext)
: mVkContext(vkContext),
  mVkPipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
  mVkRenderPass(VK_NULL_HANDLE),
  mColorClearEnabled(false), mDepthClearEnabled(false), mStencilClearEnabled(false),
  mColorWriteEnabled(true), mDepthWriteEnabled(true), mStencilWriteEnabled(false),
  mStarted(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

    memset((void *)mVkClearValues, 0, sizeof(mVkClearValues));
}

RenderPass::~RenderPass()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
RenderPass::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(mVkContext->vkDevice, mVkRenderPass, nullptr);
        mVkRenderPass = VK_NULL_HANDLE;
    }
}

bool
RenderPass::Create(VkFormat colorFormat, VkFormat depthstencilFormat)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Release();

    VkAttachmentReference           color;
    VkAttachmentReference           depthstencil;
    vector<VkAttachmentDescription> attachments;

    if(colorFormat != VK_FORMAT_UNDEFINED) {

        /// Color attachment
        VkAttachmentDescription attachmentColor;
        attachmentColor.flags           = 0;
        attachmentColor.format          = colorFormat;
        attachmentColor.samples         = VK_SAMPLE_COUNT_1_BIT;
        attachmentColor.loadOp          = (mColorClearEnabled && mColorWriteEnabled) ? VK_ATTACHMENT_LOAD_OP_CLEAR  : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentColor.storeOp         = mColorWriteEnabled ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentColor.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentColor.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentColor.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentColor.finalLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments.push_back(attachmentColor);

        color.attachment           = attachments.size() - 1;
        color.layout               = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    /// Depth/Stencil attachment
    if(depthstencilFormat != VK_FORMAT_UNDEFINED) {

        bool isDepth   = VkFormatIsDepth(depthstencilFormat);
        bool isStencil = VkFormatIsStencil(depthstencilFormat);

        VkAttachmentDescription attachmentDepthStencil;

        attachmentDepthStencil.flags          = 0;
        attachmentDepthStencil.format         = depthstencilFormat;
        attachmentDepthStencil.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachmentDepthStencil.loadOp         = (isDepth   && mDepthClearEnabled) ? VK_ATTACHMENT_LOAD_OP_CLEAR  : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDepthStencil.storeOp        = (isDepth   && mDepthWriteEnabled) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDepthStencil.stencilLoadOp  = (isStencil && mStencilClearEnabled) ? VK_ATTACHMENT_LOAD_OP_CLEAR  : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDepthStencil.stencilStoreOp = (isStencil && mStencilWriteEnabled) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDepthStencil.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDepthStencil.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(attachmentDepthStencil);

        depthstencil.attachment   = attachments.size() - 1;
        depthstencil.layout       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint       = mVkPipelineBindPoint;
    subpass.flags                   = 0;
    subpass.colorAttachmentCount    = colorFormat        != VK_FORMAT_UNDEFINED ? 1             : 0;
    subpass.pColorAttachments       = colorFormat        != VK_FORMAT_UNDEFINED ? &color        : nullptr;
    subpass.pDepthStencilAttachment = depthstencilFormat != VK_FORMAT_UNDEFINED ? &depthstencil : nullptr;
    subpass.pResolveAttachments     = nullptr;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = nullptr;

    VkRenderPassCreateInfo info;
    info.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.pNext            = nullptr;
    info.flags            = 0;
    info.attachmentCount  = static_cast<uint32_t>(attachments.size());
    info.pAttachments     = attachments.data();
    info.subpassCount     = 1;
    info.pSubpasses       = &subpass;
    info.dependencyCount  = 0;
    info.pDependencies    = nullptr;

    VkResult err = vkCreateRenderPass(mVkContext->vkDevice, &info, nullptr, &mVkRenderPass);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}


void
RenderPass::Begin(VkCommandBuffer *activeCmdBuffer, VkFramebuffer *framebuffer, bool hasSecondary)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkRenderPassBeginInfo info;
    info.sType                     = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.pNext                     = nullptr;
    info.framebuffer               = *framebuffer;
    info.renderPass                = mVkRenderPass;
    info.renderArea                = mVkRenderArea;
    info.clearValueCount           = 2;
    info.pClearValues              = mVkClearValues;

    const VkSubpassContents subpassContents = !hasSecondary ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

    vkCmdBeginRenderPass(*activeCmdBuffer, &info, subpassContents);

    mStarted = true;
}

bool
RenderPass::End(VkCommandBuffer *activeCmdBuffer)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (mStarted) {
        mStarted = false;
        vkCmdEndRenderPass(*activeCmdBuffer);
        return true;
    } else {
        return false;
    }
}

void
RenderPass::SetClearArea(const VkRect2D *rect)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mVkRenderArea = *rect;
}

void
RenderPass::SetClearColorValue(const float *value)
{
    FUN_ENTRY(GL_LOG_TRACE);
    mVkClearValues[0].color.float32[0] = value[0];
    mVkClearValues[0].color.float32[1] = value[1];
    mVkClearValues[0].color.float32[2] = value[2];
    mVkClearValues[0].color.float32[3] = value[3];
}
void
RenderPass::SetClearDepthStencilValue(float depth, uint32_t stencil)
{
    FUN_ENTRY(GL_LOG_TRACE);
    mVkClearValues[1].depthStencil.depth   = depth;
    mVkClearValues[1].depthStencil.stencil = stencil;
}

}

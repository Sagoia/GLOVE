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
 *  @file       framebuffer.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Framebuffer Functionality in Vulkan
 *
 *  @section
 *
 *  Framebuffers represent a collection of specific memory attachments that a
 *  render pass instance uses. Framebuffers are represented by VkFramebuffer.
 *
 */

#include "framebuffer.h"

namespace vulkanAPI {

Framebuffer::Framebuffer(const XContext_t *xContext)
: mXContext(xContext),
  mVkFramebuffer(VK_NULL_HANDLE)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

Framebuffer::~Framebuffer()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
Framebuffer::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkFramebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(mXContext->vkDevice, mVkFramebuffer, nullptr);
        mVkFramebuffer = VK_NULL_HANDLE;
    }
}

bool
Framebuffer::Create(std::vector<VkImageView> *imageViews, VkRenderPass *renderpass, uint32_t width, uint32_t height)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Release();

    VkFramebufferCreateInfo info;
    info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext           = nullptr;
    info.flags           = 0;
    info.renderPass      = *renderpass;
    info.width           = width;
    info.height          = height;
    info.layers          = 1;
    info.attachmentCount = static_cast<uint32_t>(imageViews->size());
    info.pAttachments    = imageViews->data();

    VkResult err = vkCreateFramebuffer(mXContext->vkDevice, &info, nullptr, &mVkFramebuffer);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

}

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
 *  @file       renderbuffer.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Renderbuffer Functionality in GLOVE
 *
 *  @scope
 *
 *  Renderbuffer objects contain images (texture). They are created and used
 *  specifically with Framebuffer Objects.
 *
 */

#include "renderbuffer.h"
#include "utils/glUtils.h"
#include "vulkan/utils.h"

Renderbuffer::Renderbuffer(const vulkanAPI::vkContext_t *vkContext, vulkanAPI::CommandBufferManager *cbManager)
: mVkContext(vkContext), mCommandBufferManager(cbManager),
mInternalFormat(GL_RGBA4), mTarget(GL_INVALID_VALUE), mTexture(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

Renderbuffer::~Renderbuffer()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
Renderbuffer::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mTexture != nullptr) {
        delete mTexture;
        mTexture = nullptr;
    }
}

bool
Renderbuffer::Allocate(GLint width, GLint height, GLenum internalformat)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mDims.width     = width;
    mDims.height    = height;
    mInternalFormat = internalformat;

    mTexture->SetTarget(GL_TEXTURE_2D);

    VkFormat vkformat = GlInternalFormatToVkFormat(mInternalFormat);

    if(GlFormatIsColorRenderable(mInternalFormat)) {
        mTexture->SetVkFormat(vkformat);
        mTexture->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));
    } else {
        // convert to supported format
        vkformat = FindSupportedDepthStencilFormat(mVkContext->vkGpus[0], GetVkFormatDepthBits(vkformat), GetVkFormatStencilBits(vkformat));
        mTexture->SetVkFormat(vkformat);
        mTexture->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));
    }
    mTexture->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_2D);
    mTexture->SetVkImageTiling();
    mTexture->SetState(width, height, 0, 0, GlInternalFormatToGlFormat(mInternalFormat),
                       GlInternalFormatToGlType(mInternalFormat), Texture::GetDefaultInternalAlignment(), nullptr);

    return mTexture->Allocate();
}

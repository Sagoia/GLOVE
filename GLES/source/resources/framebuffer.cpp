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
 *  @brief      Framebuffer Functionality in GLOVE
 *
 *  @scope
 *
 *  Framebuffer Objects allow the creation of user-defined framebuffers for
 *  offscreen rendering.
 *
 */

#include "framebuffer.h"
#include "utils/VkToGlConverter.h"
#include "utils/glUtils.h"

Framebuffer::Framebuffer(const vulkanAPI::vkContext_t *vkContext, vulkanAPI::CommandBufferManager *cbManager)
: mVkContext(vkContext), mCommandBufferManager(cbManager),
mTarget(GL_INVALID_VALUE), mWriteBufferIndex(0), mState(IDLE),
mUpdated(true), mSizeUpdated(false), mDepthStencilTexture(nullptr), mIsSystem(false), mBindToTexture(false), mSurfaceType(GLOVE_SURFACE_INVALID)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mRenderPass           = new vulkanAPI::RenderPass(vkContext);
    mAttachmentDepth      = new Attachment();
    mAttachmentStencil    = new Attachment();
}

Framebuffer::~Framebuffer()
{
    FUN_ENTRY(GL_LOG_TRACE);

    delete mRenderPass;
    delete mAttachmentDepth;
    delete mAttachmentStencil;

    if(!mIsSystem && mDepthStencilTexture != nullptr) {
        delete mDepthStencilTexture;
        mDepthStencilTexture = nullptr;
    }

    for(auto color : mAttachmentColors) {
        if(color) {
            delete color;
            color = nullptr;
        }
    }
    mAttachmentColors.clear();

    Release();
}

void
Framebuffer::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(auto fb : mFramebuffers) {
        delete fb;
        fb = nullptr;
    }

    mFramebuffers.clear();
}

Texture *
Framebuffer::GetColorAttachmentTexture(void) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(mIsSystem) {
        return mAttachmentColors[mWriteBufferIndex]->GetTexture();
    }

    Texture * tex   = nullptr;
    GLenum    type  = GetColorAttachmentType();
    uint32_t  index = GetColorAttachmentName();

    if(index) {
        if(type == GL_TEXTURE) {
            tex = mTextureArray->GetObject(index);
        } else if(type == GL_RENDERBUFFER) {
            tex = mRenderbufferArray->GetObject(index)->GetTexture();
        }
    }

    return tex;
}

Texture *
Framebuffer::GetDepthAttachmentTexture(void) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mIsSystem) {
        return mDepthStencilTexture;
    }

    Texture * tex   = nullptr;
    GLenum    type  = GetDepthAttachmentType();
    uint32_t  index = GetDepthAttachmentName();

    if(index) {
        if(type == GL_TEXTURE) {
            tex = mTextureArray->GetObject(index);
        } else if(type == GL_RENDERBUFFER) {
            tex = mRenderbufferArray->GetObject(index)->GetTexture();
        }
    }

    return tex;
}

Texture *
Framebuffer::GetStencilAttachmentTexture(void) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mIsSystem) {
        return mDepthStencilTexture;
    }

    Texture * tex   = nullptr;
    GLenum    type  = GetStencilAttachmentType();
    uint32_t  index = GetStencilAttachmentName();

    if(index) {
        if(type == GL_TEXTURE) {
            tex = mTextureArray->GetObject(index);
        } else if(type == GL_RENDERBUFFER) {
            tex = mRenderbufferArray->GetObject(index)->GetTexture();
        }
    }

    return tex;
}

void
Framebuffer::AddColorAttachment(Texture *texture)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Attachment *att = new Attachment(texture);
    mAttachmentColors.push_back(att);

    SetWidth(texture->GetWidth());
    SetHeight(texture->GetHeight());

    mUpdated = true;
}

void
Framebuffer::SetColorAttachment(int width, int height)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mAttachmentColors.size() == 0) {
        Attachment *att = new Attachment();
        mAttachmentColors.push_back(att);
    }

    SetWidth(width);
    SetHeight(height);

    mUpdated     = true;
    mSizeUpdated = true;
}

GLenum
Framebuffer::CheckStatus(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(GetColorAttachmentType()    == GL_NONE  &&
       GetDepthAttachmentType()    == GL_NONE  &&
       GetStencilAttachmentType()  == GL_NONE) {
        return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    }

    if((GetColorAttachmentType() != GL_NONE   &&
        (!GlFormatIsColorRenderable(GetColorAttachmentTexture()->GetInternalFormat()) ||
         GetColorAttachmentTexture()->GetWidth()  <= 0   ||
         GetColorAttachmentTexture()->GetHeight() <= 0   )
        ) ||
       (GetDepthAttachmentType() != GL_NONE    &&
        (!GlFormatIsDepthRenderable(GetDepthAttachmentTexture()->GetInternalFormat())  ||
         GetDepthAttachmentTexture()->GetWidth()  <= 0 ||
         GetDepthAttachmentTexture()->GetHeight() <= 0)
       ) ||
       (GetStencilAttachmentType() != GL_NONE  &&
        (!GlFormatIsStencilRenderable(GetStencilAttachmentTexture()->GetInternalFormat())  ||
         GetStencilAttachmentTexture()->GetWidth()  <= 0 ||
         GetStencilAttachmentTexture()->GetHeight() <= 0))
       ) {
        return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    }

    if(GetColorAttachmentType() != GL_NONE && GetDepthAttachmentType() != GL_NONE) {
        if(GetColorAttachmentTexture()->GetWidth()  != GetDepthAttachmentTexture()->GetWidth() ||
           GetColorAttachmentTexture()->GetHeight() != GetDepthAttachmentTexture()->GetHeight()) {
            return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
        }
    }

    if(GetColorAttachmentType() != GL_NONE && GetStencilAttachmentType() != GL_NONE) {
        if(GetColorAttachmentTexture()->GetWidth()  != GetStencilAttachmentTexture()->GetWidth() ||
           GetColorAttachmentTexture()->GetHeight() != GetStencilAttachmentTexture()->GetHeight()) {
            return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
        }
    }

    if(GetDepthAttachmentType() != GL_NONE && GetStencilAttachmentType() != GL_NONE) {
        if(GetDepthAttachmentTexture()->GetWidth()  != GetStencilAttachmentTexture()->GetWidth() ||
           GetDepthAttachmentTexture()->GetHeight() != GetStencilAttachmentTexture()->GetHeight()) {
            return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
        }
    }

    // The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.
    //  if() {
    //      return GL_FRAMEBUFFER_UNSUPPORTED;
    //  }

    return GL_FRAMEBUFFER_COMPLETE;
}

bool
Framebuffer::CreateVkRenderPass(bool clearColorEnabled, bool clearDepthEnabled, bool clearStencilEnabled,
                                bool writeColorEnabled, bool writeDepthEnabled, bool writeStencilEnabled)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mRenderPass->SetColorClearEnabled(clearColorEnabled);
    mRenderPass->SetDepthClearEnabled(clearDepthEnabled);
    mRenderPass->SetStencilClearEnabled(clearStencilEnabled);

    mRenderPass->SetColorWriteEnabled(writeColorEnabled);
    mRenderPass->SetDepthWriteEnabled(writeDepthEnabled);
    mRenderPass->SetStencilWriteEnabled(writeStencilEnabled);

    return mRenderPass->Create(GetColorAttachmentTexture() ?
                               GetColorAttachmentTexture()->GetVkFormat() : VK_FORMAT_UNDEFINED,
                               mDepthStencilTexture ?
                               mDepthStencilTexture->GetVkFormat() : VK_FORMAT_UNDEFINED);
}

void
Framebuffer::CreateDepthStencilTexture(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(GetDepthAttachmentTexture() || GetStencilAttachmentTexture()) {

        if(mDepthStencilTexture != nullptr) {
            delete mDepthStencilTexture;
            mDepthStencilTexture = nullptr;
        }

        mDepthStencilTexture = new Texture(mVkContext, mCommandBufferManager);
        mDepthStencilTexture->SetTarget(GL_TEXTURE_2D);

        VkFormat vkformat = GlInternalFormatToVkFormat(
            GetDepthAttachmentTexture()   ? GetDepthAttachmentTexture()->GetInternalFormat()   : GL_INVALID_VALUE,
            GetStencilAttachmentTexture() ? GetStencilAttachmentTexture()->GetInternalFormat() : GL_INVALID_VALUE);
        mDepthStencilTexture->SetVkFormat(vkformat);
        mDepthStencilTexture->SetVkImageUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        mDepthStencilTexture->SetVkImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        mDepthStencilTexture->SetVkImageTiling();
        GLenum glformat = VkFormatToGlInternalformat(mDepthStencilTexture->GetVkFormat());
        mDepthStencilTexture->InitState();
        mDepthStencilTexture->SetState(GetWidth(), GetHeight(), 0, 0, GlInternalFormatToGlFormat(glformat), GlInternalFormatToGlType(glformat), Texture::GetDefaultInternalAlignment(), NULL);
        mDepthStencilTexture->Allocate();
    }
}

void
Framebuffer::IsUpdated()
{
    if(GetColorAttachmentTexture()) {

        mUpdated |= GetColorAttachmentTexture()->GetDataUpdated();
        if( GetColorAttachmentTexture()->GetWidth()  != GetWidth()  ||
            GetColorAttachmentTexture()->GetHeight() != GetHeight() ) {

            SetWidth (GetColorAttachmentTexture()->GetWidth());
            SetHeight(GetColorAttachmentTexture()->GetHeight());

            mSizeUpdated = true;
        }
        GetColorAttachmentTexture()->SetDataUpdated(false);
    }
}

void
Framebuffer::CreateRenderPass(bool clearColorEnabled, bool clearDepthEnabled, bool clearStencilEnabled,
                               bool writeColorEnabled, bool writeDepthEnabled, bool writeStencilEnabled,
                               const float *colorValue, float depthValue, uint32_t stencilValue,
                               const Rect  *clearRect)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mUpdated || mSizeUpdated ||
       mRenderPass->GetColorClearEnabled()   != clearColorEnabled    ||
       mRenderPass->GetDepthClearEnabled()   != clearDepthEnabled    ||
       mRenderPass->GetStencilClearEnabled() != clearStencilEnabled  ||
       mRenderPass->GetColorWriteEnabled()   != writeColorEnabled    ||
       mRenderPass->GetDepthWriteEnabled()   != writeDepthEnabled    ||
       mRenderPass->GetStencilWriteEnabled() != writeStencilEnabled) {

        if(!mIsSystem && mSizeUpdated) {
            CreateDepthStencilTexture();
            mSizeUpdated = false;
        }
        CreateVkRenderPass(clearColorEnabled, clearDepthEnabled, clearStencilEnabled,
                           writeColorEnabled, writeDepthEnabled, writeStencilEnabled);
        Create();

        mUpdated = false;
    }


    const VkRect2D clearRect2D = { {clearRect->x, clearRect->y},
                                   {(uint32_t)clearRect->width, (uint32_t)clearRect->height}};

    mRenderPass->SetClearArea(&clearRect2D);
    mRenderPass->SetClearColorValue(colorValue);
    mRenderPass->SetClearDepthStencilValue(depthValue, stencilValue);
}

void
Framebuffer::BeginVkRenderPass()
{
    VkCommandBuffer activeCmdBuffer = mCommandBufferManager->GetActiveCommandBuffer();
    mRenderPass->Begin(&activeCmdBuffer, mFramebuffers[mWriteBufferIndex]->GetFramebuffer(), true);
}

bool
Framebuffer::EndVkRenderPass(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkCommandBuffer activeCmdBuffer = mCommandBufferManager->GetActiveCommandBuffer();
    return mRenderPass->End(&activeCmdBuffer);
}

void
Framebuffer::PrepareVkImage(VkImageLayout newImageLayout)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(GetColorAttachmentTexture()) {
        GetColorAttachmentTexture()->PrepareVkImageLayout(newImageLayout);
    }
}

bool
Framebuffer::Create(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Release();

    for(uint32_t i = 0; i < mAttachmentColors.size(); ++i) {
        vulkanAPI::Framebuffer *frameBuffer = new vulkanAPI::Framebuffer(mVkContext);

        vector<VkImageView> imageViews;
        if(GetColorAttachmentTexture(i)) {
            imageViews.push_back(GetColorAttachmentTexture(i)->GetVkImageView());
        }
        if(mDepthStencilTexture) {
            imageViews.push_back(mDepthStencilTexture->GetVkImageView());
        }

        if(!frameBuffer->Create(&imageViews, GetVkRenderPass(), GetWidth(), GetHeight())) {
            delete frameBuffer;
            return false;
        }

        mFramebuffers.push_back(frameBuffer);
    }

    return true;
}

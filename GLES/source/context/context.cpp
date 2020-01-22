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
 *  @file       context.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Default Context and Resources Initialization
 *
 */

#include "context.h"
#include "utils/VkToGlConverter.h"

static Context *currentContext = nullptr;

Context *GetCurrentContext()
{
    FUN_ENTRY(GL_LOG_TRACE);

    return currentContext;
}

void SetCurrentContext(Context *ctx)
{
    FUN_ENTRY(GL_LOG_TRACE);

    currentContext = ctx;
}

Context::Context()
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkContext            = vulkanAPI::GetContext();
    mCommandBufferManager = new vulkanAPI::CommandBufferManager(mVkContext);

    mResourceManager = new ResourceManager(mVkContext);
    mShaderCompiler  = new GlslangShaderCompiler();
    mPipeline        = new vulkanAPI::Pipeline(mVkContext);
    mCacheManager    = new CacheManager(mVkContext);

    mStateManager.InitVkPipelineStates(mPipeline);

    InitializeDefaultTextures();

    mPipeline->SetCacheManager(mCacheManager);
    mResourceManager->SetCacheManager(mCacheManager);

    mWriteSurface = nullptr;
    mReadSurface  = nullptr;
    mWriteFBO     = nullptr;
    mSystemFBO    = nullptr;

    //If VK_KHR_maintenance1 is supported, then there is no need to invert the Y
    mIsYInverted        = !(vulkanAPI::GetContext()->mIsMaintenanceExtSupported);
    mIsModeLineLoop     = false;

    mScreenSpacePass = new ScreenSpacePass(mVkContext);
    mScreenSpacePass->SetCacheManager(mCacheManager);
    mStateManager.InitVkPipelineStates(mScreenSpacePass->GetPipeline());
}

Context::~Context()
{
    FUN_ENTRY(GL_LOG_TRACE);

    ReleaseSystemFBO();

    if(mShaderCompiler != nullptr) {
        delete mShaderCompiler;
        mShaderCompiler = nullptr;
    }

    delete mResourceManager;
    delete mCacheManager;

    if(mPipeline != nullptr) {
        delete mPipeline;
        mPipeline = nullptr;
    }

    if(mScreenSpacePass != nullptr) {
        delete mScreenSpacePass;
        mScreenSpacePass = nullptr;
    }

    delete mCommandBufferManager;
}

void
Context::ReleaseSystemFBO(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(uint32_t i = 0; i < mSystemTextures.size(); ++i) {
        if(mSystemTextures[i] != nullptr) {
            delete mSystemTextures[i];
            mSystemTextures[i] = nullptr;
        }
    }

    for(auto iter : mSystemFBOMap){
        delete iter.second;
    }
    mSystemFBOMap.clear();

    mCacheManager->CleanUpCaches();

    if(mCommandBufferManager) {
        mCommandBufferManager->DestroyVkCmdBuffers();
        mCommandBufferManager->AllocateVkCmdBuffers();
    }

    mReadSurface = nullptr;
    mWriteSurface = nullptr;
    mWriteFBO = nullptr;
}

void
Context::InitializeDefaultTextures()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(int i = 0; i < GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS; ++i) {
        mStateManager.GetActiveObjectsState()->SetActiveTexture(GL_TEXTURE_2D      , i, mResourceManager->GetDefaultTexture(GL_TEXTURE_2D));
        mStateManager.GetActiveObjectsState()->SetActiveTexture(GL_TEXTURE_CUBE_MAP, i, mResourceManager->GetDefaultTexture(GL_TEXTURE_CUBE_MAP));
    }
}

Framebuffer *
Context::CreateFBOFromEGLSurface(EGLSurfaceInterface *eglSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // TODO: Pbuffer/pixmaps are not properly supported
    assert(eglSurfaceInterface->type == EGL_WINDOW_BIT);

    Framebuffer *fbo = InitializeFrameBuffer(eglSurfaceInterface);
    fbo->CreateVkRenderPass(false, false, false, true, true, false);
    fbo->Create();
    fbo->SetSurfaceType(GLOVE_SURFACE_WINDOW);

    return fbo;
}

Framebuffer *
Context::InitializeFrameBuffer(EGLSurfaceInterface *eglSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkImage *vkImages = static_cast<VkImage *>(eglSurfaceInterface->images);
    Framebuffer *fbo = new Framebuffer(mVkContext);

    // color images
    for(uint32_t i = 0; i < eglSurfaceInterface->imageCount; ++i) {
        Texture *tex = new Texture(mVkContext);

        VkFormat surfaceColorFormat = static_cast<VkFormat>(eglSurfaceInterface->surfaceColorFormat);
        GLenum glformat = VkFormatToGlInternalformat(surfaceColorFormat);
        GLenum glType = GlInternalFormatToGlType(glformat);

        tex->SetTarget(GL_TEXTURE_2D);
        tex->SetWidth(eglSurfaceInterface->width);
        tex->SetHeight(eglSurfaceInterface->height);
        tex->SetInternalFormat(glformat);
        tex->SetExplicitInternalFormat(glformat);
        tex->SetFormat(GlInternalFormatToGlFormat(glformat));
        tex->SetType(glType);
        tex->SetExplicitType(glType);

        tex->SetVkFormat(surfaceColorFormat);
        tex->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
        tex->SetVkImageTiling();
        tex->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_2D);
        tex->SetVkImage(vkImages[i]);
        tex->CreateVkImageSubResourceRange();
        tex->CreateVkImageView();
        
        fbo->AddColorAttachment(tex);
        mSystemTextures.push_back(tex);
    }

    // depth/stencil images
    Texture *tex = CreateDepthStencil(eglSurfaceInterface);
    fbo->SetDepthStencilAttachmentTexture(tex);
    fbo->SetTarget(GL_FRAMEBUFFER);
    fbo->SetIsSystem();
    fbo->SetEGLSurfaceInterface(eglSurfaceInterface);

    return fbo;
}


Texture *
Context::CreateDepthStencil(EGLSurfaceInterface *eglSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(mVkContext->vkGpus[0], eglSurfaceInterface->depthSize, eglSurfaceInterface->stencilSize);

    if(depthStencilFormat == VK_FORMAT_UNDEFINED) {
        return nullptr;
    }

    Texture *tex = new Texture(mVkContext);
    tex->SetTarget(GL_TEXTURE_2D);
    tex->SetVkFormat(depthStencilFormat);
    tex->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
    tex->SetVkImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    tex->SetVkImageTiling();
    tex->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_2D);

    if(!eglSurfaceInterface->depthBuffer) {
        GLenum glformat = VkFormatToGlInternalformat(depthStencilFormat);
        tex->InitState();
        tex->SetState(eglSurfaceInterface->width, eglSurfaceInterface->height,
                  0, 0,
                  GlInternalFormatToGlFormat(glformat),
                  GlInternalFormatToGlType(glformat),
                  Texture::GetDefaultInternalAlignment(),
                  nullptr);
        if(!tex->Allocate()) {
            delete tex;
            return nullptr;
        }
        VkImage vkImage = tex->GetImage()->GetImage();
        eglSurfaceInterface->depthBuffer = reinterpret_cast<void *>(vkImage);
        tex->SetVkImage(vkImage);
    } else {
        VkImage vkImage = reinterpret_cast<VkImage>(eglSurfaceInterface->depthBuffer);
        tex->SetVkImage(vkImage);
        tex->CreateVkImageSubResourceRange();
        tex->CreateVkImageView();
    }
    mSystemTextures.push_back(tex);
    return tex;
}

void
Context::DestroyAPISurfaceData(const vulkanAPI::vkContext_t *vkContext,
                               EGLSurfaceInterface *eglSurfaceInterface)
{
    // destroy any surface resources allocated implicitly within GLES
    if(eglSurfaceInterface &&
       eglSurfaceInterface->contextRef == 0 &&
       eglSurfaceInterface->depthBuffer != 0) {
        VkImage vkImage = reinterpret_cast<VkImage>(eglSurfaceInterface->depthBuffer);
        if(vkImage != VK_NULL_HANDLE) {
            vkDestroyImage(vkContext->vkDevice, vkImage, nullptr);
        }
        eglSurfaceInterface->depthBuffer = 0;
    }
}

void
Context::SetReadWriteSurfaces(EGLSurfaceInterface *eglReadSurfaceInterface, EGLSurfaceInterface *eglWriteSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_TRACE);

    // TODO:: TBD as we do not take into account read surface!
    if(mWriteSurface && mWriteSurface == eglWriteSurfaceInterface->surface) {
        return;
    }

    FRAMEBUFFER_SURFACES_PAIR readWritePair = {eglReadSurfaceInterface, eglWriteSurfaceInterface};

    auto fboIter = mSystemFBOMap.find(readWritePair);
    if(fboIter != mSystemFBOMap.end()) {
        mWriteSurface = fboIter->first.first;
        mReadSurface = fboIter->first.second;

        // if surface has been invalidated, recreate the FBO (e.g., resized on another context)
        bool surfaceUpdated =
                (eglWriteSurfaceInterface->width != static_cast<uint32_t>(mWriteFBO->GetWidth())) ||
                (eglWriteSurfaceInterface->height != static_cast<uint32_t>(mWriteFBO->GetHeight()));
        if(!surfaceUpdated) {
            mWriteFBO = fboIter->second;
        } else {
            ReleaseSystemFBO();
            mWriteFBO = CreateFBOFromEGLSurface(eglWriteSurfaceInterface);
            mSystemFBOMap[readWritePair] = mWriteFBO;
        }

    } else {
        mWriteSurface = eglWriteSurfaceInterface;
        mReadSurface  = eglReadSurfaceInterface;
        mWriteFBO     = CreateFBOFromEGLSurface(eglWriteSurfaceInterface);
        mSystemFBOMap[readWritePair] = mWriteFBO;
    }

    SetSystemFramebuffer(mWriteFBO);
}

void
Context::SetSystemFramebuffer(Framebuffer *FBO)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mSystemFBO = FBO;

    mStateManager.GetViewportTransformationState()->SetViewportRect(mSystemFBO->GetRect());
    mStateManager.GetFragmentOperationsState()->SetScissorRect(mSystemFBO->GetRect());
    mPipeline->SetUpdatePipeline(true);
    mPipeline->SetUpdateViewportState(true);
}

void
Context::EGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    NOT_IMPLEMENTED();
}

void
Context::EGLImageTargetRenderBufferStorageOES(GLenum target, GLeglImageOES image)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    NOT_IMPLEMENTED();

    return;
}

void
Context::InsertEventMarkerEXT(GLsizei length, const GLchar *marker)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    NOT_IMPLEMENTED();

    return;
}

void
Context::PushGroupMarkerEXT(GLsizei length, const GLchar *marker)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    NOT_IMPLEMENTED();

    return;
}

void
Context::PopGroupMarkerEXT(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    NOT_IMPLEMENTED();

    return;
}

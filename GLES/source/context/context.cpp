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

    InitExtensions();

    mResourceManager = new ResourceManager(mVkContext, mCommandBufferManager);
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

    mScreenSpacePass = new ScreenSpacePass(this, mVkContext, mCommandBufferManager);
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

void 
Context::InitExtensions()
{
    mCompressedTextureFormats.clear();
    mExtensions = "GL_OES_get_program_binary GL_OES_rgb8_rgba8 GL_EXT_texture_format_BGRA8888";
    if (mVkContext->vkDeviceFeatures.textureCompressionBC) { 
        mExtensions += " GL_EXT_texture_compression_dxt1 GL_EXT_texture_compression_s3tc";
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
    }
    if (vulkanAPI::DeviceExtensionEnabled(VK_IMG_FORMAT_PVRTC_EXTENSION_NAME)) {
        mExtensions += " GL_IMG_texture_compression_pvrtc";
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG);
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG);
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG);
        mCompressedTextureFormats.push_back(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG);
    }
}

Framebuffer *
Context::CreateFBOFromEGLSurface(EGLSurfaceInterface *eglSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // TODO: Pbuffer/pixmaps are not properly supported
    assert(eglSurfaceInterface->type == EGL_WINDOW_BIT);

    Framebuffer *fbo = InitializeFrameBuffer(eglSurfaceInterface);
    Texture *tex = CreateDepthStencil(eglSurfaceInterface);
    fbo->SetDepthStencilAttachmentTexture(tex);
    fbo->CreateVkRenderPass(false, false, false, true, true, false);
    fbo->Create();
    fbo->SetSurfaceType(GLOVE_SURFACE_WINDOW);
    mSystemTextures.push_back(tex);

    return fbo;
}

Framebuffer *
Context::InitializeFrameBuffer(EGLSurfaceInterface *eglSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkImage *vkImages = reinterpret_cast<VkImage *>(eglSurfaceInterface->images);

    Framebuffer *fbo = new Framebuffer(mVkContext, mCommandBufferManager);
    fbo->SetCacheManager(mCacheManager);

    for(uint32_t i = 0; i < eglSurfaceInterface->imageCount; ++i) {
        Texture *tex = new Texture(mVkContext, mCommandBufferManager);

        GLenum glformat = VkFormatToGlInternalformat(static_cast<VkFormat>(eglSurfaceInterface->surfaceColorFormat));

        tex->SetTarget(GL_TEXTURE_2D);
        tex->SetWidth(eglSurfaceInterface->width);
        tex->SetHeight(eglSurfaceInterface->height);
        tex->SetInternalFormat(glformat);
        tex->SetExplicitInternalFormat(glformat);
        tex->SetFormat(GlInternalFormatToGlFormat(glformat));
        tex->SetType(GlInternalFormatToGlType(glformat));
        tex->SetExplicitType(GlInternalFormatToGlType(glformat));

        tex->SetVkFormat(static_cast<VkFormat>(eglSurfaceInterface->surfaceColorFormat));
        tex->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
        tex->SetVkImageTiling();
        tex->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_2D);
        tex->SetVkImage(vkImages[i]);
        tex->CreateVkImageSubResourceRange();
        tex->CreateVkImageView();
        tex->PrepareVkImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        fbo->AddColorAttachment(tex);
        mSystemTextures.push_back(tex);
    }

    fbo->SetTarget(GL_FRAMEBUFFER);
    fbo->SetIsSystem();
    fbo->SetWriteBufferIndex(eglSurfaceInterface->nextImageIndex);

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

    Texture *tex = new Texture(mVkContext, mCommandBufferManager);
    tex->SetTarget(GL_TEXTURE_2D);
    tex->SetVkFormat(depthStencilFormat);
    tex->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
    tex->SetVkImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    tex->SetVkImageTiling();
    tex->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_2D);

    GLenum glformat = VkFormatToGlInternalformat(depthStencilFormat);
    tex->InitState();
    tex->SetState(eglSurfaceInterface->width, eglSurfaceInterface->height,
                  0, 0,
                  GlInternalFormatToGlFormat(glformat),
                  GlInternalFormatToGlType(glformat),
                  Texture::GetDefaultInternalAlignment(),
                  nullptr);
    if (!tex->Allocate()) {
        delete tex;
        return nullptr;
    }
    return tex;
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
        mWriteFBO = fboIter->second;
    } else {
        mWriteSurface = eglWriteSurfaceInterface->surface;
        mReadSurface  = eglReadSurfaceInterface->surface;
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

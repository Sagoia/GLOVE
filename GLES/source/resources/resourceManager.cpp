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
 *  @file       resourceManager.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Resource Manager Functionality in GLOVE
 *
 *  @section
 *
 *  OpenGL ES allows developers to allocate, edit and delete a variety of
 *  resources. These include Generic Vertex Attributes, Buffers, Renderbuffers,
 *  Framebuffers, Textures, Shaders, and Shader Programs.
 */

#include "resourceManager.h"

uint32_t ResourceManager::mShadingObjectCount = 1;

ResourceManager::ResourceManager(const vulkanAPI::XContext_t *xContext, vulkanAPI::CommandBufferManager *cbManager):
    mXContext(xContext)
{
    FUN_ENTRY(GL_LOG_TRACE);

    CreateDefaultTextures(cbManager);
}

ResourceManager::~ResourceManager()
{
    FUN_ENTRY(GL_LOG_TRACE);

    delete mDefaultTexture2D;
    delete mDefaultTextureCubeMap;
}

void
ResourceManager::SetCacheManager(CacheManager *cacheManager)
{

}

void
ResourceManager::CreateDefaultTextures(vulkanAPI::CommandBufferManager *cbManager)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mDefaultTexture2D = new Texture(mXContext, cbManager);
    mDefaultTexture2D->SetTarget(GL_TEXTURE_2D);
    mDefaultTexture2D->SetVkFormat(VK_FORMAT_R8G8B8A8_UNORM);
    mDefaultTexture2D->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    mDefaultTexture2D->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_2D);
    mDefaultTexture2D->SetVkImageTiling();
    mDefaultTexture2D->InitState();

    mDefaultTextureCubeMap = new Texture(mXContext, cbManager);
    mDefaultTextureCubeMap->SetTarget(GL_TEXTURE_CUBE_MAP);
    mDefaultTextureCubeMap->SetVkFormat(VK_FORMAT_R8G8B8A8_UNORM);
    mDefaultTextureCubeMap->SetVkImageUsage(static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    mDefaultTextureCubeMap->SetVkImageTarget(vulkanAPI::Image::VK_IMAGE_TARGET_CUBE);
    mDefaultTextureCubeMap->SetVkImageTiling();
    mDefaultTextureCubeMap->InitState();
}

uint32_t
ResourceManager::FindShaderID(const Shader *shader)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(shadingPoolIDs_t::iterator it = mShadingObjectPool.begin(); it != mShadingObjectPool.end(); ++it) {
        if(it->second.type == SHADER_ID && GetShaderID(shader) == it->second.arrayIndex) {
            return it->first;
        }
    }
    return 0;
}

uint32_t
ResourceManager::FindShaderProgramID(const ShaderProgram *program)
{
   FUN_ENTRY(GL_LOG_DEBUG);

   for(shadingPoolIDs_t::iterator it = mShadingObjectPool.begin(); it != mShadingObjectPool.end(); ++it) {
        if(it->second.type == SHADER_PROGRAM_ID && GetShaderProgramID(program) == it->second.arrayIndex) {
            return it->first;
        }
    }
    return 0;
}


void
ResourceManager::UpdateFramebufferObjects(GLuint index, GLenum target)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    auto &objects = mFramebuffers.GetObjects();
    for (uint32_t i = 0; i < objects.Size(); ++i) {
        Framebuffer *fb = objects[i];
        if((fb->GetColorAttachmentType()   == target && index == fb->GetColorAttachmentName()) ||
           (fb->GetDepthAttachmentType()   == target && index == fb->GetDepthAttachmentName()) ||
           (fb->GetStencilAttachmentType() == target && index == fb->GetStencilAttachmentName())) {
            fb->SetUpdated();
        }
    }
}

bool
ResourceManager::IsTextureAttachedToFBO(const Texture *texture)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    auto &objects = mFramebuffers.GetObjects();
    for (uint32_t i = 0; i < objects.Size(); ++i) {
        Framebuffer *fb = objects[i];
        if(fb->GetColorAttachmentType() == GL_TEXTURE && texture == fb->GetColorAttachmentTexture()) {
            return true;
        }
    }

    return false;
}

void
ResourceManager::ResetShaderProgramDescSetsState(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    ObjectArray<ShaderProgram> *shaderProgramArray = GetShaderProgramArray();
    auto &objects = shaderProgramArray->GetObjects();
    for (uint32_t i = 0; i < objects.Size(); ++i) {
        objects[i]->MoveUsingDescriptorSetsToPending();
    }
}

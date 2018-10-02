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
 *  @file       framebuffer.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Framebuffer Functionality in GLOVE
 *
 */

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "attachment.h"
#include "renderbuffer.h"
#include "vulkan/renderPass.h"
#include "vulkan/framebuffer.h"
#include "utils/arrays.hpp"

class Framebuffer {
public:
    enum RenderState {
        IDLE,
        CLEAR,
        CLEAR_DRAW,
        DRAW,
        DELETE
    };

private:

    const
    vulkanAPI::vkContext_t *         mVkContext;
    vulkanAPI::CommandBufferManager *mCommandBufferManager;
    ObjectArray<Texture>            *mTextureArray;
    ObjectArray<Renderbuffer>       *mRenderbufferArray;

    Rect                            mDims;
    GLenum                          mTarget;
    uint32_t                        mWriteBufferIndex;
    RenderState                     mRenderState;
    bool                            mUpdated;
    bool                            mSizeUpdated;

    vulkanAPI::RenderPass*          mRenderPass;
    vector<vulkanAPI::Framebuffer*> mFramebuffers;

    vector<Attachment*>             mAttachmentColors;
    Attachment*                     mAttachmentDepth;
    Attachment*                     mAttachmentStencil;
    Texture*                        mDepthStencilTexture;
    bool                            mIsSystem;

    void                            Release(void);

public:
    Framebuffer(const vulkanAPI::vkContext_t *vkContext = nullptr, vulkanAPI::CommandBufferManager *cbManager = nullptr);
    ~Framebuffer();

// Create Functions
    bool                    Create(void);
    void                    CreateDepthStencilTexture(void);

// RenderPass Functions
    bool                    CreateVkRenderPass(bool clearColorEnabled, bool clearDepthEnabled, bool clearStencilEnabled,
                                               bool writeColorEnabled, bool writeDepthEnabled, bool writeStencilEnabled);
    void                    BeginVkRenderPass (bool clearColorEnabled, bool clearDepthEnabled, bool clearStencilEnabled,
                                               bool writeColorEnabled, bool writeDepthEnabled, bool writeStencilEnabled,
                                               const float *colorValue, float depthValue, uint32_t stencilValue, const VkRect2D *clearRect);
    void                    EndVkRenderPass(void);
    void                    PrepareVkImage(VkImageLayout newImageLayout);

// Add Functions
    void                    AddColorAttachment(Texture *texture);

// Check Functions
    GLenum                  CheckStatus(void);

// Update Functions
    void                    IsUpdated(void);

// Get Functions
    inline VkFramebuffer*   GetActiveVkFramebuffer()                    const   { FUN_ENTRY(GL_LOG_TRACE); return mFramebuffers[mWriteBufferIndex]->GetFramebuffer(); }
    inline RenderState      GetRenderState()                            const   { FUN_ENTRY(GL_LOG_TRACE); return mRenderState; }
    inline Rect *           GetRect(void)                                       { FUN_ENTRY(GL_LOG_TRACE); return &mDims; }
    inline int              GetX(void)                                  const   { FUN_ENTRY(GL_LOG_TRACE); return mDims.x; }
    inline int              GetY(void)                                  const   { FUN_ENTRY(GL_LOG_TRACE); return mDims.y; }
    inline int              GetWidth(void)                              const   { FUN_ENTRY(GL_LOG_TRACE); return mDims.width; }
    inline int              GetHeight(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mDims.height; }
    inline GLenum           GetTarget(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mTarget; }
    inline VkRenderPass *   GetVkRenderPass(void)                       const   { FUN_ENTRY(GL_LOG_TRACE); return mRenderPass->GetRenderPass(); }

    inline uint32_t         GetAttachmentName(GLenum type)              const   { FUN_ENTRY(GL_LOG_TRACE); switch(type) {
                                                                                                           case GL_COLOR_ATTACHMENT0:   return GetColorAttachmentName();
                                                                                                           case GL_DEPTH_ATTACHMENT:    return GetDepthAttachmentName();
                                                                                                           case GL_STENCIL_ATTACHMENT:  return GetStencilAttachmentName();
                                                                                                           default:                     return 0;}}

    inline GLenum           GetColorAttachmentType(void)                const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentColors.size() ? mAttachmentColors[0]->GetType()  : GL_NONE; }
    inline uint32_t         GetColorAttachmentName(void)                const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentColors.size() ? mAttachmentColors[0]->GetName()  : 0; }
    inline GLint            GetColorAttachmentLevel(void)               const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentColors.size() ? mAttachmentColors[0]->GetLevel() : 0; }
    inline GLenum           GetColorAttachmentLayer(void)               const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentColors.size() ? mAttachmentColors[0]->GetLayer() : GL_TEXTURE_CUBE_MAP_POSITIVE_X; }
    inline Texture *        GetColorAttachmentTexture(uint32_t i)       const   { FUN_ENTRY(GL_LOG_TRACE); return mIsSystem ? mAttachmentColors[i]->GetTexture() :
                                                                                                                              GetColorAttachmentTexture(); }
           Texture *        GetColorAttachmentTexture(void)             const;

    inline GLenum           GetDepthAttachmentType(void)                const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentDepth->GetType();     }
    inline uint32_t         GetDepthAttachmentName(void)                const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentDepth->GetName();     }
    inline GLint            GetDepthAttachmentLevel(void)               const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentDepth->GetLevel();    }
    inline GLenum           GetDepthAttachmentLayer(void)               const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentDepth->GetLayer();    }
           Texture *        GetDepthAttachmentTexture(void)             const;

    inline GLenum           GetStencilAttachmentType(void)              const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentStencil->GetType();   }
    inline uint32_t         GetStencilAttachmentName(void)              const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentStencil->GetName();   }
    inline GLint            GetStencilAttachmentLevel(void)             const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentStencil->GetLevel();  }
    inline GLenum           GetStencilAttachmentLayer(void)             const   { FUN_ENTRY(GL_LOG_TRACE); return mAttachmentStencil->GetLayer();  }
           Texture *        GetStencilAttachmentTexture(void)           const;

// Set Functions
    inline void             SetVkContext(const
                                         vulkanAPI::vkContext_t *vkContext)     { FUN_ENTRY(GL_LOG_TRACE); mVkContext   = vkContext; mRenderPass->SetVkContext(vkContext); }
    inline void             SetCommandBufferManager(
                            vulkanAPI::CommandBufferManager     *cbManager)     { FUN_ENTRY(GL_LOG_TRACE); mCommandBufferManager = cbManager; }
    inline void             SetResources(
                            ObjectArray<Texture>            *texArray,
                            ObjectArray<Renderbuffer>       *rbArray)           { FUN_ENTRY(GL_LOG_TRACE); mTextureArray = texArray; mRenderbufferArray = rbArray; }

    inline void             SetUpdated(void)                                    { FUN_ENTRY(GL_LOG_TRACE); mUpdated     = true;   }
    inline void             SetIsSystem(void)                                   { FUN_ENTRY(GL_LOG_TRACE); mIsSystem    = true;   }
    inline void             SetRenderState(RenderState renderState)             { FUN_ENTRY(GL_LOG_TRACE); mRenderState = renderState;}
    inline void             SetTarget(GLenum target)                            { FUN_ENTRY(GL_LOG_TRACE); mTarget      = target; }
    inline void             SetWidth(int32_t width)                             { FUN_ENTRY(GL_LOG_TRACE); mDims.width  = width;  }
    inline void             SetHeight(int32_t height)                           { FUN_ENTRY(GL_LOG_TRACE); mDims.height = height; }

           void             SetColorAttachmentTexture(Texture *texture);
    inline void             SetColorAttachmentType(GLenum type)                 { FUN_ENTRY(GL_LOG_TRACE); mAttachmentColors[0]->SetType(type);   }
    inline void             SetColorAttachmentName(uint32_t name)               { FUN_ENTRY(GL_LOG_TRACE); mAttachmentColors[0]->SetName(name);   }
    inline void             SetColorAttachmentLevel(GLint level)                { FUN_ENTRY(GL_LOG_TRACE); mAttachmentColors[0]->SetLevel(level); }
    inline void             SetColorAttachmentLayer(GLenum layer)               { FUN_ENTRY(GL_LOG_TRACE); mAttachmentColors[0]->SetLayer(layer); }

    inline void             SetDepthAttachmentName(uint32_t name)               { FUN_ENTRY(GL_LOG_TRACE); mAttachmentDepth->SetName(name);   mUpdated = true;}
    inline void             SetDepthAttachmentType(GLenum type)                 { FUN_ENTRY(GL_LOG_TRACE); mAttachmentDepth->SetType(type);   }
    inline void             SetDepthAttachmentLevel(GLint level)                { FUN_ENTRY(GL_LOG_TRACE); mAttachmentDepth->SetLevel(level); }
    inline void             SetDepthAttachmentLayer(GLenum layer)               { FUN_ENTRY(GL_LOG_TRACE); mAttachmentDepth->SetLayer(layer); }

    inline void             SetStencilAttachmentName(uint32_t name)             { FUN_ENTRY(GL_LOG_TRACE); mAttachmentStencil->SetName(name); mUpdated = true;}
    inline void             SetStencilAttachmentType(GLenum type)               { FUN_ENTRY(GL_LOG_TRACE); mAttachmentStencil->SetType(type);   }
    inline void             SetStencilAttachmentLevel(GLint level)              { FUN_ENTRY(GL_LOG_TRACE); mAttachmentStencil->SetLevel(level); }
    inline void             SetStencilAttachmentLayer(GLenum layer)             { FUN_ENTRY(GL_LOG_TRACE); mAttachmentStencil->SetLayer(layer); }

    inline void             SetDepthStencilAttachmentTexture(Texture *texture)  { FUN_ENTRY(GL_LOG_TRACE); mDepthStencilTexture = texture; }
    inline void             SetWriteBufferIndex(uint32_t buffer)                { FUN_ENTRY(GL_LOG_TRACE); if(mAttachmentColors.size() > 1) {mWriteBufferIndex = buffer;} }
};

#endif // __FRAMEBUFFER_H__

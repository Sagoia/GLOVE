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
 *  @file       renderbuffer.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Renderbuffer Functionality in GLOVE
 *
 */

#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include "texture.h"

class Renderbuffer
{
private:
    const
    vulkanAPI::vkContext_t *         mVkContext;

    vulkanAPI::CommandBufferManager *mCommandBufferManager;

    Rect                             mDims;
    GLenum                           mInternalFormat;
    GLenum                           mTarget;
    Texture *                        mTexture;

public:
    Renderbuffer(const vulkanAPI::vkContext_t *vkContext = nullptr, vulkanAPI::CommandBufferManager *cbManager = nullptr);
    ~Renderbuffer();

// Allocate Functions
           bool        Allocate(GLint width, GLint height, GLenum internalformat);

// Release Functions
           void        Release(void);

// Get Functions
    inline int32_t     GetWidth(void)                                     const { FUN_ENTRY(GL_LOG_TRACE); return mDims.width;     }
    inline int32_t     GetHeight(void)                                    const { FUN_ENTRY(GL_LOG_TRACE); return mDims.height;    }
    inline GLenum      GetTarget(void)                                    const { FUN_ENTRY(GL_LOG_TRACE); return mTarget;         }
    inline GLenum      GetInternalFormat(void)                            const { FUN_ENTRY(GL_LOG_TRACE); return mInternalFormat; }
    inline Texture *   GetTexture(void)                                   const { FUN_ENTRY(GL_LOG_TRACE); return mTexture;        }

// Set Functions
    inline void        SetVkContext(const vulkanAPI::vkContext_t *vkContext)    { FUN_ENTRY(GL_LOG_TRACE); mVkContext            = vkContext; }
    inline void        SetCommandBufferManager(
                                   vulkanAPI::CommandBufferManager *cbManager)  { FUN_ENTRY(GL_LOG_TRACE); mCommandBufferManager = cbManager; }
    inline void        SetTarget(GLenum target)                                 { FUN_ENTRY(GL_LOG_TRACE); mTarget               = target;    }

// Init Functions
           void        InitTexture(void);
};

#endif // __RENDERBUFFER_H__

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
 *  @file       stateFramebufferOperations.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      State related to Framebuffer Operations
 *
 */

#ifndef __STATEFRAMEBUFFEROPERATIONS_H__
#define __STATEFRAMEBUFFEROPERATIONS_H__

#include "GLES2/gl2.h"
#include "utils/glLogger.h"
#include "utils/color.hpp"

#include <cmath>

class StateFramebufferOperations {

private:
      GLfloat             mClearColor[4];
      GLfloat             mClearDepth;
      GLint               mClearStencil;

      GLubyte             mColorMask;
      GLboolean           mDepthMask;
      GLuint              mStencilMaskFront;
      GLuint              mStencilMaskBack;

public:
      StateFramebufferOperations();
      ~StateFramebufferOperations();

// Get Functions
             void             GetClearColor(GLboolean *params)          const;
             void             GetClearColor(GLint *params)              const;
             void             GetClearColor(GLfloat *params)            const;
      inline GLfloat          GetClearDepth(void)                       const   { FUN_ENTRY(GL_LOG_TRACE); return mClearDepth; }
      inline GLint            GetClearStencil(void)                     const   { FUN_ENTRY(GL_LOG_TRACE); return mClearStencil; }
      inline GLboolean        GetDepthMask(void)                        const   { FUN_ENTRY(GL_LOG_TRACE); return mDepthMask; }
      inline GLubyte          GetColorMask(void)                        const   { FUN_ENTRY(GL_LOG_TRACE); return mColorMask; }
      inline GLboolean        IsColorWriteEnabled(void)                 const   { FUN_ENTRY(GL_LOG_TRACE); return mColorMask > 0; }
             GLboolean        IsDepthWriteEnabled(void)                 const   { FUN_ENTRY(GL_LOG_TRACE); return mDepthMask == GL_TRUE; }
             GLboolean        IsStencilWriteEnabled(void)               const   { FUN_ENTRY(GL_LOG_TRACE); return mStencilMaskFront != 0u || mStencilMaskBack != 0u; }
             uint32_t         GetClearStencilMasked(void)               const;
             bool             ColorMaskActive(void)                     const;
             bool             StencilMaskActive(void)                   const;
             void             GetColorMask(GLboolean *params)           const;
             void             GetColorMask(GLint     *params)           const;
             void             GetColorMask(GLfloat   *params)           const;
      inline GLuint           GetStencilMaskFront(void)                 const   { FUN_ENTRY(GL_LOG_TRACE); return mStencilMaskFront; }
      inline GLuint           GetStencilMaskBack(void)                  const   { FUN_ENTRY(GL_LOG_TRACE); return mStencilMaskBack;  }

// Set Functions
             void             SetClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
             void             SetClearDepth(GLfloat depth);
      inline void             SetClearStencil(GLint stencil)                    { FUN_ENTRY(GL_LOG_TRACE); mClearStencil = stencil; }

// Update Functions
      bool                    UpdateClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
      bool                    UpdateClearDepth(GLfloat depth);
      bool                    UpdateClearStencil(GLint stencil);
      bool                    UpdateColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
      bool                    UpdateDepthMask(GLenum enable);
      bool                    UpdateStencilMask(GLuint mask);
      bool                    UpdateStencilMaskFront(GLuint mask);
      bool                    UpdateStencilMaskBack(GLuint mask);
};

#endif //__STATEFRAMEBUFFEROPERATIONS_H__

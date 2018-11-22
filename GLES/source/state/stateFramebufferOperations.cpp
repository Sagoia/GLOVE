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
 *  @file       stateFramebufferOperations.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      State related to Framebuffer Operations
 *
 *  @section
 *
 *  Fragment Operations include clearing the (a) Color and (b) Depth/Stencil
 *  buffers.
 *
 */

#include "stateFramebufferOperations.h"
#include "utils/glUtils.h"

StateFramebufferOperations::StateFramebufferOperations()
: mClearDepth(1.0f), mClearStencil(0), mClearStencilOld(0u),
mDepthMask(GL_TRUE), mStencilMaskFront(0xFFFFFFFFu), mStencilMaskBack(0xFFFFFFFFu)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mColorMask = GlColorMaskPack(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    mClearColor[0] = 0.0f;
    mClearColor[1] = 0.0f;
    mClearColor[2] = 0.0f;
    mClearColor[3] = 0.0f;
}

StateFramebufferOperations::~StateFramebufferOperations()
{
    FUN_ENTRY(GL_LOG_TRACE);
}

void
StateFramebufferOperations::GetClearColor(GLboolean *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mClearColor[0] == 0.0f ? GL_FALSE : GL_TRUE;
    params[1] = mClearColor[1] == 0.0f ? GL_FALSE : GL_TRUE;
    params[2] = mClearColor[2] == 0.0f ? GL_FALSE : GL_TRUE;
    params[3] = mClearColor[3] == 0.0f ? GL_FALSE : GL_TRUE;
}

void
StateFramebufferOperations::GetClearColor(GLint *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mClearColor[0] == 1.0f ? 0x7fffffff : static_cast<GLint>(mClearColor[0] * 0x7fffffff);
    params[1] = mClearColor[1] == 1.0f ? 0x7fffffff : static_cast<GLint>(mClearColor[1] * 0x7fffffff);
    params[2] = mClearColor[2] == 1.0f ? 0x7fffffff : static_cast<GLint>(mClearColor[2] * 0x7fffffff);
    params[3] = mClearColor[3] == 1.0f ? 0x7fffffff : static_cast<GLint>(mClearColor[3] * 0x7fffffff);
}

void
StateFramebufferOperations::GetClearColor(GLfloat *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mClearColor[0];
    params[1] = mClearColor[1];
    params[2] = mClearColor[2];
    params[3] = mClearColor[3];
}

uint32_t
StateFramebufferOperations::GetClearStencilMasked() const
{
    FUN_ENTRY(GL_LOG_TRACE);

    uint32_t stencilUint = static_cast<uint32_t>(mClearStencil & 0xFF);
    return stencilUint & mStencilMaskFront;
}

bool
StateFramebufferOperations::StencilMaskActive() const
{
   FUN_ENTRY(GL_LOG_TRACE);

   if((mStencilMaskBack > 0u && mStencilMaskBack < 0xFFFFFFFFu) ||
      (mStencilMaskFront > 0u && mStencilMaskFront < 0xFFFFFFFFu)) {
       return true;
    }
    return false;
}

bool
StateFramebufferOperations::ColorMaskActive() const
{
   FUN_ENTRY(GL_LOG_TRACE);

   if(mColorMask == 0 ||
      (GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_RED) &&
       GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_GREEN) &&
       GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_BLUE) &&
       GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_ALPHA))) {
       return false;
    }
    return true;
}

void
StateFramebufferOperations::GetColorMask(GLboolean *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_RED);
    params[1] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_GREEN);
    params[2] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_BLUE);
    params[3] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_ALPHA);
}

void
StateFramebufferOperations::GetColorMask(GLint     *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_RED);
    params[1] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_GREEN);
    params[2] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_BLUE);
    params[3] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_ALPHA);
}

void
StateFramebufferOperations::GetColorMask(GLfloat   *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_RED) == GL_TRUE ? 1.0f : 0.0f;
    params[1] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_GREEN) == GL_TRUE ? 1.0f : 0.0f;
    params[2] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_BLUE) == GL_TRUE ? 1.0f : 0.0f;
    params[3] = GlColorMaskHasBit(mColorMask, GLColorMaskBit::GLC_ALPHA) == GL_TRUE ? 1.0f : 0.0f;
}

// Set Functions
void
StateFramebufferOperations::SetClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mClearColor[0] = CLAMPF_01(red);
    mClearColor[1] = CLAMPF_01(green);
    mClearColor[2] = CLAMPF_01(blue);
    mClearColor[3] = CLAMPF_01(alpha);
}

void
StateFramebufferOperations::SetClearDepth(GLclampf depth)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mClearDepth   = CLAMPF_01(depth);
}

// Update Functions
bool
StateFramebufferOperations::UpdateClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLfloat r = CLAMPF_01(red);
    GLfloat g = CLAMPF_01(green);
    GLfloat b = CLAMPF_01(blue);
    GLfloat a = CLAMPF_01(alpha);

    bool res = (mClearColor[0] != r) || (mClearColor[1] != g) || (mClearColor[2] != b) || (mClearColor[3] != a);

    mClearColor[0] = r;
    mClearColor[1] = g;
    mClearColor[2] = b;
    mClearColor[3] = a;

    return res;
}

bool
StateFramebufferOperations::UpdateClearDepth(GLclampf depth)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLfloat d = CLAMPF_01(depth);

    bool res = (mClearDepth != d);
    mClearDepth = d;
    return res;
}

bool
StateFramebufferOperations::UpdateColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLubyte mask = GlColorMaskPack(red, green, blue, alpha);

    bool res = (mColorMask != mask);
    mColorMask = mask;
    return res;
}

bool
StateFramebufferOperations::UpdateDepthMask(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mDepthMask != enable);
    mDepthMask = enable;
    return res;
}

bool
StateFramebufferOperations::UpdateClearStencil(GLint stencil)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mClearStencil != stencil);
    mClearStencil = stencil;
    return res;
}

bool
StateFramebufferOperations::UpdateStencilMask(GLuint mask)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mStencilMaskFront != mask) || (mStencilMaskBack != mask);
    mStencilMaskFront = mask;
    mStencilMaskBack  = mask;
    return res;
}

bool
StateFramebufferOperations::UpdateStencilMaskFront(GLuint mask)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mStencilMaskFront != mask);
    mStencilMaskFront = mask;
    return res;
}

bool
StateFramebufferOperations::UpdateStencilMaskBack(GLuint mask)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mStencilMaskBack != mask);
    mStencilMaskBack = mask;
    return res;
}

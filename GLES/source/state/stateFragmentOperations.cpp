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
 *  @file       stateFragmentOperations.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      State related to Fragment Operations
 *
 *  @section
 *
 *  Fragment Operations include (a) Scissor Test, (b) Depth Bounds Test,
 *  (c) Multisampling, (d) Stencil Test, (e) Depth Buffer Test, (f) Blending
 *  Test, (g) Dithering
 *
 */

#include "stateFragmentOperations.h"

StencilOperations::StencilOperations():
        mFuncCompare(GL_ALWAYS),
        mFuncRef(0),
        mFuncMask(0xFFFFFFFFu),
        mOpFail(GL_KEEP),
        mOpZfail(GL_KEEP),
        mOpZpass(GL_KEEP) {
    }

StateFragmentOperations::StateFragmentOperations()
: mScissorTestEnabled(GL_FALSE),
mDepthBoundsTestEnabled(GL_FALSE), mMinDepthBounds(0.0f), mMaxDepthBounds(1.0f),
mMultiSamplingEnabled(GL_FALSE), mSampleCoverageBits(1), mSampleCoverageValue(1.0f), mSampleCoverageInvert(GL_FALSE), mSampleCoverageEnabled(GL_FALSE), mSampleAlphaToCoverageEnabled(GL_FALSE),
mSampleAlphaToOneEnabled(GL_FALSE), mSampleShadingEnabled(GL_FALSE), mMinSampleShading(1.0f),
mStencilTestEnabled(GL_FALSE), mDepthTestFunc(GL_LESS), mDepthTestEnabled(GL_FALSE),
mBlendingEnabled(GL_FALSE), mBlendingEquationRGB(GL_FUNC_ADD), mBlendingEquationAlpha(GL_FUNC_ADD),
mBlendingFactorSourceRGB(GL_ONE), mBlendingFactorSourceAlpha(GL_ONE), mBlendingFactorDestinationRGB(GL_ZERO), mBlendingFactorDestinationAlpha(GL_ZERO), mBlendingColorAttachmentCount(1),
mBlendingLogicOp(GL_CLEAR), mBlendingLogicOpEnabled(GL_FALSE),
mDitheringEnabled(GL_TRUE)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mBlendingColor[0] = 0.0f;
    mBlendingColor[1] = 0.0f;
    mBlendingColor[2] = 0.0f;
    mBlendingColor[3] = 0.0f;
}

StateFragmentOperations::~StateFragmentOperations()
{
    FUN_ENTRY(GL_LOG_TRACE);
}


void
StateFragmentOperations::GetBlendingColor(GLboolean *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mBlendingColor[0] == 0.0f ? GL_FALSE : GL_TRUE;
    params[1] = mBlendingColor[1] == 0.0f ? GL_FALSE : GL_TRUE;
    params[2] = mBlendingColor[2] == 0.0f ? GL_FALSE : GL_TRUE;
    params[3] = mBlendingColor[3] == 0.0f ? GL_FALSE : GL_TRUE;
}

void
StateFragmentOperations::GetBlendingColor(GLint *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = static_cast<GLint>(mBlendingColor[0]);
    params[1] = static_cast<GLint>(mBlendingColor[1]);
    params[2] = static_cast<GLint>(mBlendingColor[2]);
    params[3] = static_cast<GLint>(mBlendingColor[3]);
}

void
StateFragmentOperations::GetBlendingColor(GLfloat *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mBlendingColor[0];
    params[1] = mBlendingColor[1];
    params[2] = mBlendingColor[2];
    params[3] = mBlendingColor[3];
}

void
StateFragmentOperations::GetScissorRect(GLboolean *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mScissorTestRectangle.x      ? GL_TRUE : GL_FALSE;
    params[1] = mScissorTestRectangle.y      ? GL_TRUE : GL_FALSE;
    params[2] = mScissorTestRectangle.width  ? GL_TRUE : GL_FALSE;
    params[3] = mScissorTestRectangle.height ? GL_TRUE : GL_FALSE;
}

void
StateFragmentOperations::GetScissorRect(GLint *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = mScissorTestRectangle.x;
    params[1] = mScissorTestRectangle.y;
    params[2] = mScissorTestRectangle.width;
    params[3] = mScissorTestRectangle.height;
}

void
StateFragmentOperations::GetScissorRect(GLfloat *params) const
{
    FUN_ENTRY(GL_LOG_TRACE);

    params[0] = static_cast<GLfloat>(mScissorTestRectangle.x);
    params[1] = static_cast<GLfloat>(mScissorTestRectangle.y);
    params[2] = static_cast<GLfloat>(mScissorTestRectangle.width);
    params[3] = static_cast<GLfloat>(mScissorTestRectangle.height);
}

void
StateFragmentOperations::SetBlendingColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mBlendingColor[0] = CLAMP_F(red);
    mBlendingColor[1] = CLAMP_F(green);
    mBlendingColor[2] = CLAMP_F(blue);
    mBlendingColor[3] = CLAMP_F(alpha);
}

void
StateFragmentOperations::SetScissorRect(const Rect *fbo)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mScissorTestRectangle.x = fbo->x;
    mScissorTestRectangle.y = fbo->y;
    mScissorTestRectangle.width = fbo->width;
    mScissorTestRectangle.height = fbo->height;
}

bool
StateFragmentOperations::UpdateSampleCoverageEnabled(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = mSampleCoverageEnabled != enable;
    mSampleCoverageEnabled = enable;
    return res;
}

bool
StateFragmentOperations::UpdateSampleAlphaToCoverageEnabled(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = mSampleAlphaToCoverageEnabled != enable;
    mSampleAlphaToCoverageEnabled = enable;
    return res;
}

bool
StateFragmentOperations::UpdateSampleCoverageValue(GLfloat value)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLfloat v = CLAMP_F(value);
    bool res = mSampleCoverageValue != v;
    mSampleCoverageValue = v;
    return res;
}

bool
StateFragmentOperations::UpdateSampleCoverageInvert(GLboolean invert)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = mSampleCoverageInvert != invert;
    mSampleCoverageInvert = invert;
    return res;
}

bool
StateFragmentOperations::UpdateScissorRect(GLint x, GLint y, GLint width, GLint height)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mScissorTestRectangle.x != x)
            || (mScissorTestRectangle.y != y)
            || (mScissorTestRectangle.width != width)
            || (mScissorTestRectangle.height != height);

             mScissorTestRectangle.x = x;
             mScissorTestRectangle.y = y;
             mScissorTestRectangle.width = width;
             mScissorTestRectangle.height = height;
             return res;
}

bool
StateFragmentOperations::UpdateDepthTestFunc(GLenum func)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mDepthTestFunc != func);
    mDepthTestFunc = func;
    return res;
}

bool
StateFragmentOperations::UpdateDepthTestEnabled(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mDepthTestEnabled != enable);
    mDepthTestEnabled = enable;
    return res;
}

bool
StateFragmentOperations::UpdateScissorTestEnabled(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mScissorTestEnabled != enable);
    mScissorTestEnabled = enable;
    return res;
}

bool
StateFragmentOperations::UpdateBlendingEnabled(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mBlendingEnabled != enable);
    mBlendingEnabled = enable;
    return res;
}

bool
StateFragmentOperations::UpdateStencilTestEnabled(GLboolean enable)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mStencilTestEnabled != enable);
    mStencilTestEnabled = enable;
    return res;
}

bool
StateFragmentOperations::UpdateBlendingEquation(GLenum modeRGB, GLenum modeAlpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mBlendingEquationRGB != modeRGB) || (mBlendingEquationAlpha != modeAlpha);
    mBlendingEquationRGB = modeRGB;
    mBlendingEquationAlpha = modeAlpha;
    return res;
}

bool
StateFragmentOperations::UpdateBlendingColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLfloat r = CLAMP_F(red);
    GLfloat g = CLAMP_F(green);
    GLfloat b = CLAMP_F(blue);
    GLfloat a = CLAMP_F(alpha);

    bool res = (mBlendingColor[0] != r) || (mBlendingColor[1] != g) || (mBlendingColor[2] != b) || (mBlendingColor[3] != a);

    mBlendingColor[0] = r;
    mBlendingColor[1] = g;
    mBlendingColor[2] = b;
    mBlendingColor[3] = a;
    return res;
}

bool StateFragmentOperations::UpdateBlendingFactors(GLenum srcRGB, GLenum srcAlpha, GLenum dstRGB, GLenum dstAlpha)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = (mBlendingFactorSourceRGB != srcRGB)
            || (mBlendingFactorSourceAlpha != srcAlpha)
            || (mBlendingFactorDestinationRGB != dstRGB)
            || (mBlendingFactorDestinationAlpha != dstAlpha);

    mBlendingFactorSourceRGB = srcRGB;
    mBlendingFactorSourceAlpha = srcAlpha;
    mBlendingFactorDestinationRGB = dstRGB;
    mBlendingFactorDestinationAlpha = dstAlpha;
    return res;
}

bool StateFragmentOperations::UpdateStencilTestFunc(GLenum face, GLenum func, GLint  ref, GLuint mask)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = true;
    StencilOperations& stencilOperations = (face == GL_FRONT) ? mStencilOperations[SF_FRONT] : mStencilOperations[SF_BACK];
    res = (stencilOperations.GetFuncCompare() != func) || (stencilOperations.GetFuncRef() != ref) || (stencilOperations.GetFuncMask() != mask);
    stencilOperations.SetFuncCompare(func);
    GLint clampedRef = std::min(std::max(ref, 0), 0xFF);
    stencilOperations.SetFuncRef(clampedRef);
    stencilOperations.SetFuncMask(mask);

    return res;
}

bool StateFragmentOperations::UpdateStencilTestOp(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    FUN_ENTRY(GL_LOG_TRACE);

    bool res = true;
    StencilOperations& stencilOperations = (face == GL_FRONT) ? mStencilOperations[SF_FRONT] : mStencilOperations[SF_BACK];
    res = (stencilOperations.GetOpFail() != fail) || (stencilOperations.GetOpZfail() != zfail) || (stencilOperations.GetOpZpass() != zpass);
    stencilOperations.SetOpFail(fail);
    stencilOperations.SetOpZfail(zfail);
    stencilOperations.SetOpZpass(zpass);

    return res;
}

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
 *  @file       glUtils.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      OpengGL ES Utility Functions
 *
 */

#ifndef __GLUTILS_H__
#define __GLUTILS_H__

#include <cstdio>
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include <stdint.h>

enum GLColorMaskBit {
    GLC_RED     = 0,
    GLC_GREEN   = 1,
    GLC_BLUE    = 2,
    GLC_ALPHA   = 3
};

GLboolean               GlColorMaskHasBit(GLubyte colorMask, GLColorMaskBit bit);
GLubyte                 GlColorMaskPack(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
const char *            GlAttribTypeToString(GLenum type);
GLenum                  GlFormatToGlInternalFormat(GLenum format, GLenum type);
GLenum                  GlInternalFormatToGlType(GLenum internalFormat);
GLenum                  GlInternalFormatToGlFormat(GLenum internalFormat);
size_t                  GlInternalFormatTypeToNumElements(GLenum format, GLenum type);
int32_t                 GlAttribTypeToElementSize(GLenum type);
size_t                  GlTypeToElementSize(GLenum type);
void                    GlFormatToStorageBits(GLenum format, GLint     *r_, GLint     *g_, GLint     *b_, GLint     *a_, GLint     *d_, GLint     *s_);
void                    GlFormatToStorageBits(GLenum format, GLfloat   *r_, GLfloat   *g_, GLfloat   *b_, GLfloat   *a_, GLfloat   *d_, GLfloat   *s_);
void                    GlFormatToStorageBits(GLenum format, GLboolean *r_, GLboolean *g_, GLboolean *b_, GLboolean *a_, GLboolean *d_, GLboolean *s_);
bool                    GlFormatIsDepthRenderable(GLenum format);
bool                    GlFormatIsStencilRenderable(GLenum format);
bool                    GlFormatIsColorRenderable(GLenum format);
uint32_t                OccupiedLocationsPerGlType(GLenum type);
bool                    IsGlSampler(GLenum type);
#endif // __GLUTILS_H__

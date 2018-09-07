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
 *  @file       genericVertexAttribute.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Generic Vertex Attribute Functionality in GLOVE
 *
 *  Each vertex is specified with multiple generic vertex attributes. Each
 *  attribute is specified with one, two, three, or four scalar values.
 *  Generic vertex attributes can be accessed from within vertex shaders
 *  and used to compute values for consumption by later processing stages.
 */

#include "genericVertexAttribute.h"

GenericVertexAttribute::GenericVertexAttribute()
: mElements(4), mType(GL_FLOAT), mNormalized(false), mStride(0), mEnabled(false),
mOffset(0), mPtr(0), mVbo(nullptr), mInternalVBO(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mGenericValue[0] = 0.0f;
    mGenericValue[1] = 0.0f;
    mGenericValue[2] = 0.0f;
    mGenericValue[3] = 1.0f;
}

GenericVertexAttribute::~GenericVertexAttribute()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
GenericVertexAttribute::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVbo != nullptr && mInternalVBO) {
        delete mVbo;
        mVbo         = nullptr;
        mInternalVBO = false;
    }
}

void
GenericVertexAttribute::SetVbo(BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Release();

    mVbo = vbo;
}

void
GenericVertexAttribute::Set(GLint nElements, GLenum type, GLboolean normalized, GLsizei stride, const void *ptr, BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    SetType(type);
    SetStride(stride);
    SetNumElements(nElements);
    SetNormalized(normalized);
    SetPointer(reinterpret_cast<uintptr_t>(ptr));
    SetOffset(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)));
    SetVbo(vbo);
}

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
#include "utils/glUtils.h"

GenericVertexAttribute::GenericVertexAttribute()
: mElements(4), mType(GL_FLOAT), mNormalized(false), mStride(0), mEnabled(false),
  mOffset(0), mPtr(0), mVbo(nullptr), mInternalVBO(false), mCacheManager(nullptr)
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
GenericVertexAttribute::MoveToCache(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVbo != nullptr && mInternalVBO) {
        mCacheManager->CacheVBO(mVbo);
        mVbo         = nullptr;
        mInternalVBO = false;
    }
}

void
GenericVertexAttribute::ConvertFixedBufferToFloat(BufferObject* vbo, size_t byteSize,
                                                  const void *srcData, size_t numVertices) {

    FUN_ENTRY(GL_LOG_TRACE);

    const uint8_t* srcBuffer = static_cast<const uint8_t*>(srcData);
    uint8_t* dstBuffer = new uint8_t[byteSize];

    // this is needed to preserve data in case the buffer contains
    // other data as well. For efficiency it can be commented out.
    memcpy(dstBuffer, srcBuffer, byteSize);

    size_t numElements = static_cast<size_t>(GetNumElements());
    size_t stride = static_cast<size_t>(GetStride());
    for(size_t ver = 0; ver < numVertices; ++ver) {
        size_t vertexIndex = ver * stride;
        for(size_t el = 0; el < numElements; ++el) {
            size_t srcIndex = vertexIndex + el * sizeof(GLfixed);
            size_t dstIndex = vertexIndex + el * sizeof(float);
            const GLfixed* val = reinterpret_cast<const GLfixed*>(&srcBuffer[srcIndex]);
            float fval = static_cast<float>(*val) / float(1<<16);
            uint8_t* fvalp = reinterpret_cast<uint8_t*>(&fval);
            for(size_t b = 0; b < sizeof(GLfloat); ++b) {
                dstBuffer[dstIndex + b] = fvalp[b];
            }
        }
    }

    vbo->Allocate(byteSize, dstBuffer);
    delete[] dstBuffer;
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

    MoveToCache();

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

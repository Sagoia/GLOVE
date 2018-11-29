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
  mOffset(0), mPtr(0),
  mInternalVbo(nullptr), mExternalVbo(nullptr),
  mInternalVBOStatus(true), mCacheManager(nullptr)
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

BufferObject*
GenericVertexAttribute::UpdateVertexAttribute(uint32_t numVertices, bool& updatedVBO)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    updatedVBO = false;

    // if disabled, use the generic vertex attribute value registered to that location
    // Otherwise, generate the appropriate vertex data
    if(IsEnabled()) {
        // Calculate stride if not given from user based on the actual data type
        GLsizei stride = GetStride() > 0 ? GetStride() : GetNumElements() * GlAttribTypeToElementSize(GetType());
        SetStride(stride);

        // Create a vbo located on client-space (e.g, glVertexAttribPointer) or
        // attach a vbo lotated on server-space (e.g., glBindBuffer)
        return IsInternalVBO() ? GenerateUserSpaceVBO(numVertices, updatedVBO) : AttachDeviceSpaceVBO(numVertices, updatedVBO);
     } else {
        return UpdateGenericValueVBO(updatedVBO);
    }
}

BufferObject*
GenericVertexAttribute::GenerateUserSpaceVBO(uint32_t numVertices, bool& updatedVBO)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    BufferObject *vbo = new VertexBufferObject(mVkContext);
    void *srcData = reinterpret_cast<void*>(GetPointer());
    size_t byteSize = numVertices * GetStride();

    // explicitly convert GL_FIXED to GL_FLOAT
    if(GetType() != GL_FIXED) {
        vbo->Allocate(byteSize, srcData);
    }
    else {
        ConvertFixedBufferToFloat(vbo, byteSize, srcData, numVertices);
    }

    SetOffset(0);
    SetCurrentVbo(vbo);
    SetInternalVBOStatus(true);
    updatedVBO = true;
    return vbo;
}

BufferObject*
GenericVertexAttribute::AttachDeviceSpaceVBO(uint32_t numVertices, bool& updatedVBO)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    BufferObject *vbo = mExternalVbo;
    updatedVBO = false;
    // explicitly convert GL_FIXED to GL_FLOAT from a buffer object
    // NOTE: this is an inefficient operation and, thus, not a recommended good practice
    if(GetType() == GL_FIXED) {
        size_t byteSize = vbo->GetSize();
        uint8_t *srcData = new uint8_t[byteSize];
        vbo->GetData(byteSize, 0, srcData);
        vbo = new VertexBufferObject(mVkContext);
        ConvertFixedBufferToFloat(vbo, byteSize, srcData, numVertices);
        delete[] srcData;
        mCacheManager->CacheVBO(vbo);
        updatedVBO = true;
    }
    return vbo;
}

BufferObject*
GenericVertexAttribute::UpdateGenericValueVBO(bool& updatedVBO)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // TODO: do not recreate the VBO
    BufferObject *vbo = new VertexBufferObject(mVkContext);
    GLfloat genericValue[4];
    GetGenericValue(genericValue);
    vbo->Allocate(4 * sizeof(float), static_cast<const void *>(genericValue));
    SetNumElements(4);
    SetType(GL_FLOAT);
    SetStride(0);
    SetCurrentVbo(vbo);
    SetInternalVBOStatus(true);
    updatedVBO = true;
    return vbo;
}

void
GenericVertexAttribute::ConvertFixedBufferToFloat(BufferObject* vbo, size_t byteSize,
                                                  void *srcData, size_t numVertices)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const uint8_t* srcBuffer = static_cast<const uint8_t*>(srcData);
    uint8_t* dstBuffer = new uint8_t[byteSize];

    // this is needed to preserve data in case the buffer contains
    // other data as well. For efficiency it can be commented out.
    memcpy(dstBuffer, srcBuffer, byteSize);

    // move the buffers by the corresponding offset (if any)
    const size_t numElements = static_cast<size_t>(GetNumElements());
    const size_t stride = static_cast<size_t>(GetStride());

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

   if(mInternalVbo != nullptr) {
       delete mInternalVbo;
       mInternalVbo        = nullptr;
    }
}

void
GenericVertexAttribute::SetCurrentVbo(BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mInternalVbo != nullptr) {
        mCacheManager->CacheVBO(mInternalVbo);
    }

    mInternalVbo = mInternalVBOStatus ? vbo : nullptr;
    mExternalVbo = mInternalVBOStatus ? nullptr : vbo;
}

void
GenericVertexAttribute::Set(GLint nElements, GLenum type, GLboolean normalized,
                            GLsizei stride, const void *ptr, BufferObject *vbo,
                            bool internalVBO)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    SetType(type);
    SetStride(stride);
    SetNumElements(nElements);
    SetNormalized(normalized);
    SetPointer(internalVBO ? reinterpret_cast<uintptr_t>(ptr) : 0);
    SetOffset(internalVBO ? 0 : reinterpret_cast<uintptr_t>(ptr));
    SetInternalVBOStatus(internalVBO);
    SetCurrentVbo(vbo);
}

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
 *  @file       bufferObject.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Buffer Object Functionality in GLOVE
 *
 */

#ifndef __BUFFEROBJECT_H__
#define __BUFFEROBJECT_H__

#include "GLES2/gl2.h"
#include "vulkan/types.h"
#include "vulkan/memory.h"
#include "vulkan/buffer.h"

class CacheManager;

class BufferObject {
private:
    const
    vulkanAPI::XContext_t*  mXContext;

    GLenum                  mUsage;
    GLenum                  mTarget;
    bool                    mAllocated;

    vulkanAPI::Memory*      mMemory;
    vulkanAPI::Buffer*      mBuffer;

public:
    explicit                BufferObject(const vulkanAPI::XContext_t *xContext = nullptr, XBufferUsageFlags usage = X_BUFFER_USAGE_UNKNOW);
    virtual                ~BufferObject();

// Allocate Functions
    bool                    Allocate(size_t size, const void *data);

// Release Functions
    void                    Release(void);

// Update Functions
    void                    UpdateData(size_t size, size_t offset, const void *data);

// Get Functions
    bool                    GetData(size_t size,
                                    size_t offset, void *data)          const;
    inline GLenum           GetUsage(void)                              const   { FUN_ENTRY(GL_LOG_TRACE); return mUsage;  }
    inline GLenum           GetTarget(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mTarget; }
    inline size_t           GetSize(void)                               const   { FUN_ENTRY(GL_LOG_TRACE); return mBuffer->GetSize(); }
    inline vulkanAPI::Buffer* 
                            GetBuffer(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mBuffer; }

// Set Functions
    void                    SetTarget(GLenum target);
    inline void             SetUsage(GLenum usage)                              { FUN_ENTRY(GL_LOG_TRACE); mUsage     = usage; }
    inline void             SetxContext(const vulkanAPI::XContext_t *xContext)  { FUN_ENTRY(GL_LOG_TRACE); mXContext = xContext;
                                                                                                           mBuffer->SetContext(xContext);
                                                                                                           mMemory->SetContext(xContext); }
    inline void             SetCacheManager(CacheManager *cacheManager)         { FUN_ENTRY(GL_LOG_TRACE); mBuffer->SetCacheManager(cacheManager);
                                                                                                           mMemory->SetCacheManager(cacheManager); }
// Has/Is Functions
    inline bool             HasData(void)                               const   { FUN_ENTRY(GL_LOG_TRACE); return mBuffer->HasBuffer(); }
    inline bool             IsIndexBuffer(void)                         const   { FUN_ENTRY(GL_LOG_TRACE); return (mBuffer->GetFlags() & X_BUFFER_USAGE_INDEX_BUFFER) > 0; }
};

class IndexBufferObject : public BufferObject
{

public:
    explicit                IndexBufferObject(const vulkanAPI::XContext_t *xContext)        : BufferObject(xContext, X_BUFFER_USAGE_INDEX_BUFFER) { FUN_ENTRY(GL_LOG_TRACE); }

};

class TransferSrcBufferObject : public BufferObject
{

public:
    explicit                TransferSrcBufferObject(const vulkanAPI::XContext_t *xContext)  : BufferObject(xContext, X_BUFFER_USAGE_TRANSFER_SRC) { FUN_ENTRY(GL_LOG_TRACE); }

};

class TransferDstBufferObject : public BufferObject
{

public:
    explicit                TransferDstBufferObject(const vulkanAPI::XContext_t *xContext)  : BufferObject(xContext, X_BUFFER_USAGE_TRANSFER_DST) { FUN_ENTRY(GL_LOG_TRACE); }

};

class VertexBufferObject : public BufferObject
{

public:
    explicit                VertexBufferObject(const vulkanAPI::XContext_t *xContext)       : BufferObject(xContext, X_BUFFER_USAGE_VERTEX_BUFFER) { FUN_ENTRY(GL_LOG_TRACE); }

};

#endif // __BUFFEROBJECT_H__

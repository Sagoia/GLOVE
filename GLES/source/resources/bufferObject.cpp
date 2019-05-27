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
 *  @file       bufferObject.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Buffer Object Functionality in GLOVE
 *
 *  @scope
 *
 *  Buffer Objects store an array of unformatted memory allocated by the OpenGL
 *  ES context (GPU). These can be used to store vertex data, pixel data retrieved
 *  from images or the framebuffer, and a variety of other things.
 *
 */

#include "bufferObject.h"

BufferObject::BufferObject(const vulkanAPI::vkContext_t *vkContext, XBufferUsageFlags usage)
: mVkContext(vkContext), mUsage(GL_STATIC_DRAW), mTarget(GL_INVALID_VALUE), mAllocated(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mBuffer = new vulkanAPI::Buffer(vkContext, usage);
    mMemory = new vulkanAPI::Memory(vkContext);
}

BufferObject::~BufferObject()
{
    FUN_ENTRY(GL_LOG_TRACE);

    delete mBuffer;
    delete mMemory;
}

void
BufferObject::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mBuffer->Release();
    mMemory->Release();
    mAllocated = false;
}

bool
BufferObject::Allocate(size_t size, const void *data)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mBuffer->SetSize(size);

    mAllocated = mBuffer->Create()                                            &&
                 mMemory->GetBufferMemoryRequirements(mBuffer->GetVkBuffer()) &&
                 mMemory->Create()                                            &&
                 mMemory->SetData(size, 0, data)                              &&
                 mMemory->BindBufferMemory(mBuffer->GetVkBuffer());
    return mAllocated;
}

bool
BufferObject::GetData(size_t size, size_t offset, void *data) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    return mMemory->GetData(size, offset, data);
}

void
BufferObject::UpdateData(size_t size, size_t offset, const void *data)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mMemory->UpdateData(size, offset, data);
}

void
BufferObject::SetTarget(GLenum target)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // realloc with combined flags in case GL specifies at a later state that an
    // already allocated, e.g., vertex buffer is also an index buffer and vice-versa
    if(mTarget != target && mTarget != GL_INVALID_VALUE) {
        VkBufferUsageFlags combinedBuffers =
                static_cast<VkBufferUsageFlags>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        if(mBuffer->GetFlags() != combinedBuffers && mAllocated == true) {
            size_t size = mBuffer->GetSize();
            uint8_t *srcData = new uint8_t[size];
            this->GetData(size, 0, srcData);
            this->Release();
            mBuffer->SetFlags(combinedBuffers);
            this->Allocate(size, srcData);
            delete[] srcData;
        }
    } else if(target == GL_ARRAY_BUFFER) {
        mBuffer->SetFlags(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    } else if(target == GL_ELEMENT_ARRAY_BUFFER) {
        mBuffer->SetFlags(VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
    mTarget = target;
}

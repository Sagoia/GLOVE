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
 *  @file       contextRendering.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      OpenGL ES API calls related to Rendering
 *
 */

#include "context.h"

void
Context::BeginRendering(bool clearColorEnabled, bool clearDepthEnabled, bool clearStencilEnabled)
{
    FUN_ENTRY(GL_LOG_TRACE);

    StateFramebufferOperations *stateFramebufferOperations = mStateManager.GetFramebufferOperationsState();

    GLfloat clearColorValue[4] = {0.0f,0.0f,0.0f,0.0f};
    if(clearColorEnabled) {
        stateFramebufferOperations->GetClearColor(clearColorValue);
    }
    GLfloat  clearDepthValue   = clearDepthEnabled   ? stateFramebufferOperations->GetClearDepth() : 0.0f;
    uint32_t clearStencilValue = clearStencilEnabled ? stateFramebufferOperations->GetClearStencilMasked() : 0u;

    if(stateFramebufferOperations->StencilMaskActive())
        clearStencilValue |= stateFramebufferOperations->GetClearStencilOld() & (~(stateFramebufferOperations->GetStencilMaskFront() & 0xFF));
    stateFramebufferOperations->SetClearStencilOld(clearStencilValue);

    mCommandBufferManager->BeginVkDrawCommandBuffer();
    mWriteFBO->PrepareVkImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    mWriteFBO->CreateRenderPass(clearColorEnabled, clearDepthEnabled, clearStencilEnabled,
                               stateFramebufferOperations->IsColorWriteEnabled(),
                               stateFramebufferOperations->IsDepthWriteEnabled(),
                               stateFramebufferOperations->IsStencilWriteEnabled(),
                               clearColorValue, clearDepthValue, clearStencilValue,
                               &mClearRect);
    mWriteFBO->BeginVkRenderPass();
}

void
Context::Clear(GLbitfield mask)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    bool clearColorEnabled   = (mask & GL_COLOR_BUFFER_BIT);
    bool clearDepthEnabled   = (mask & GL_DEPTH_BUFFER_BIT);
    bool clearStencilEnabled = (mask & GL_STENCIL_BUFFER_BIT);

    if(!clearColorEnabled && !clearDepthEnabled && !clearStencilEnabled) {
        RecordError(GL_INVALID_VALUE);
        return;
    }

    // color masks are executed implicitly through a screen-space pass (i.e., need a VkPipeline object)
    bool performCustomClear = (mStateManager.GetFramebufferOperationsState()->ColorMaskActive() && clearColorEnabled);
    if(!performCustomClear) {
        ClearSimple(clearColorEnabled, clearDepthEnabled, clearStencilEnabled);
    } else {
        ClearMask(clearDepthEnabled, clearStencilEnabled);
    }
}

void
Context::ClearSimple(bool clearColorEnabled, bool clearDepthEnabled, bool clearStencilEnabled)
{
    if(mWriteFBO->IsInDrawState()) {
        Finish();
    }
    mWriteFBO->SetStateClear();

    SetClearRect();

    BeginRendering(clearColorEnabled, clearDepthEnabled, clearStencilEnabled);
}

void
Context::ClearMask(bool clearDepthEnabled, bool clearStencilEnabled)
{
    if(mWriteFBO->IsInClearState()) {
        Finish();
    }
    SetClearRect();

    StateFramebufferOperations *stateFramebufferOperations = mStateManager.GetFramebufferOperationsState();

    GLfloat clearColorValue[4] = {0.0f,0.0f,0.0f,0.0f};
    GLfloat clearDepthValue    = clearDepthEnabled   ? stateFramebufferOperations->GetClearDepth() : 0.0f;
    uint32_t clearStencilValue = clearStencilEnabled ? stateFramebufferOperations->GetClearStencilMasked() : 0u;

    if(stateFramebufferOperations->StencilMaskActive())
        clearStencilValue |= stateFramebufferOperations->GetClearStencilOld() & (~(stateFramebufferOperations->GetStencilMaskFront() & 0xFF));
    stateFramebufferOperations->SetClearStencilOld(clearStencilValue);

    mWriteFBO->PrepareVkImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    // perform a screen-space pass
    mWriteFBO->CreateRenderPass(false, clearDepthEnabled, clearStencilEnabled,
                                stateFramebufferOperations->IsColorWriteEnabled(),
                                stateFramebufferOperations->IsDepthWriteEnabled(),
                                stateFramebufferOperations->IsStencilWriteEnabled(),
                                 clearColorValue, clearDepthValue, clearStencilValue,
                                 &mClearRect);

    // clearColor is passed as a uniform and masked through VkPipelineColorBlendAttachmentState
    mStateManager.GetFramebufferOperationsState()->GetClearColor(clearColorValue);
    mScreenSpacePass->UpdateUniformBufferColor(clearColorValue[0], clearColorValue[1], clearColorValue[2], clearColorValue[3]);

    vulkanAPI::Pipeline* pipeline = mScreenSpacePass->GetPipeline();

    pipeline->SetColorBlendAttachmentWriteMask(GLColorMaskToVkColorComponentFlags(stateFramebufferOperations->GetColorMask()));
    pipeline->SetDepthWriteEnable(stateFramebufferOperations->IsDepthWriteEnabled());
    pipeline->SetUpdatePipeline(true);
    pipeline->SetViewport(mClearRect.x, mClearRect.y, mClearRect.width, mClearRect.height);
    pipeline->SetScissor(mClearRect.x, mClearRect.y, mClearRect.width, mClearRect.height);

    if(!pipeline->Create(mWriteFBO->GetVkRenderPass())) {
        Finish();
        return;
    }

    mCommandBufferManager->BeginVkDrawCommandBuffer();
    mWriteFBO->BeginVkRenderPass();

    const VkCommandBuffer *secondaryCmdBuffer = mCommandBufferManager->AllocateVkSecondaryCmdBuffers(1);
    mCommandBufferManager->BeginVkSecondaryCommandBuffer(secondaryCmdBuffer, *mWriteFBO->GetVkRenderPass(), *mWriteFBO->GetActiveVkFramebuffer());

    mScreenSpacePass->BindPipeline(secondaryCmdBuffer);
    mScreenSpacePass->BindUniformDescriptors(secondaryCmdBuffer);
    mScreenSpacePass->BindVertexBuffers(secondaryCmdBuffer);

    pipeline->UpdateDynamicState(secondaryCmdBuffer, mStateManager.GetRasterizationState()->GetLineWidth());

    mScreenSpacePass->Draw(secondaryCmdBuffer);
    mCommandBufferManager->EndVkSecondaryCommandBuffer(secondaryCmdBuffer);

    VkCommandBuffer activeCmdBuffer = mCommandBufferManager->GetActiveCommandBuffer();
    vkCmdExecuteCommands(activeCmdBuffer, 1, secondaryCmdBuffer);
    Finish();
}

void
Context::UpdateViewportState(vulkanAPI::Pipeline* pipeline)
{
    if(pipeline->GetUpdateViewportState()) {
        pipeline->ComputeViewport(mWriteFBO->GetWidth(),
                                   mWriteFBO->GetHeight(),
                                   mStateManager.GetViewportTransformationState()->GetViewportRectX(),
                                   mStateManager.GetViewportTransformationState()->GetViewportRectY(),
                                   mStateManager.GetViewportTransformationState()->GetViewportRectWidth(),
                                   mStateManager.GetViewportTransformationState()->GetViewportRectHeight(),
                                   mStateManager.GetViewportTransformationState()->GetMinDepthRange(),
                                   mStateManager.GetViewportTransformationState()->GetMaxDepthRange());

        if(mStateManager.GetFragmentOperationsState()->GetScissorTestEnabled()) {
            pipeline->ComputeScissor(mWriteFBO->GetWidth(),
                                      mWriteFBO->GetHeight(),
                                      mStateManager.GetFragmentOperationsState()->GetScissorRectX(),
                                      mStateManager.GetFragmentOperationsState()->GetScissorRectY(),
                                      mStateManager.GetFragmentOperationsState()->GetScissorRectWidth(),
                                      mStateManager.GetFragmentOperationsState()->GetScissorRectHeight());
        } else {
            pipeline->ComputeScissor(mWriteFBO->GetWidth(),
                                      mWriteFBO->GetHeight(),
                                      mStateManager.GetViewportTransformationState()->GetViewportRectX(),
                                      mStateManager.GetViewportTransformationState()->GetViewportRectY(),
                                      mStateManager.GetViewportTransformationState()->GetViewportRectWidth(),
                                      mStateManager.GetViewportTransformationState()->GetViewportRectHeight());
        }
        pipeline->SetUpdateViewportState(false);
    }
}

void
Context::PushGeometry(uint32_t vertCount, uint32_t firstVertex, bool indexed, GLenum type, const void *indices)
{
    FUN_ENTRY(GL_LOG_TRACE);

    SetClearRect();

    if(mWriteFBO->IsInClearState()) {
        mWriteFBO->SetStateClearDraw();

    } else if(mWriteFBO->IsInIdleState()) {
        mWriteFBO->SetStateDraw();
        BeginRendering(false,false,false);

    } else if(mWriteFBO->IsInClearDrawState()) {
        mWriteFBO->SetStateDraw();
    }

    UpdateVertexAttributes(vertCount, firstVertex);

    //If the primitives are rendered with GL_LINE_LOOP we have to increment the vertCount.
    //TODO: In future this functionality may be better to stay hidden.
    if(mStateManager.GetInputAssemblyState()->GetPrimitiveMode() == GL_LINE_LOOP) {
        vertCount = vertCount + 1;
    }

    if(SetPipelineProgramShaderStages(mStateManager.GetActiveShaderProgram())) {
        if(!mPipeline->Create(mWriteFBO->GetVkRenderPass())) {
            Finish();
            return;
        }
    }

    VkCommandBuffer *secondaryCmdBuffer = mCommandBufferManager->AllocateVkSecondaryCmdBuffers(1);
    mCommandBufferManager->BeginVkSecondaryCommandBuffer(secondaryCmdBuffer, *mWriteFBO->GetVkRenderPass(), *mWriteFBO->GetActiveVkFramebuffer());

    mPipeline->Bind(secondaryCmdBuffer);
    BindUniformDescriptors(secondaryCmdBuffer);
    BindVertexBuffers(secondaryCmdBuffer, indices, type, indexed, vertCount);
    UpdateViewportState(mPipeline);

    mPipeline->UpdateDynamicState(secondaryCmdBuffer, mStateManager.GetRasterizationState()->GetLineWidth());

    DrawGeometry(secondaryCmdBuffer, indexed, firstVertex, vertCount);
    mCommandBufferManager->EndVkSecondaryCommandBuffer(secondaryCmdBuffer);

    VkCommandBuffer activeCmdBuffer = mCommandBufferManager->GetActiveCommandBuffer();
    vkCmdExecuteCommands(activeCmdBuffer, 1, secondaryCmdBuffer);
}

void Context::UpdateVertexAttributes(uint32_t vertCount, uint32_t firstVertex)
{
    FUN_ENTRY(GL_LOG_TRACE);

    /// A glVertexAttrib related function has been called. Check to see if mVkPipelineVertexInput needs to be updated.
    /// If this is true then VkPipeline needs to be updated too.
    /// Otherwise only the buffer that will be bound with vkCmdBindVertexBuffers need to be updated
    if(mPipeline->GetUpdateVertexAttribVBOs() || firstVertex) {
        mStateManager.GetActiveShaderProgram()->PrepareVertexAttribBufferObjects(vertCount, firstVertex, mResourceManager->GetGenericVertexAttributes());
        mPipeline->SetUpdatePipeline(true);
        mPipeline->SetUpdateVertexAttribVBOs(false);
    }
}

void Context::BindUniformDescriptors(VkCommandBuffer *CmdBuffer)
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(*mStateManager.GetActiveShaderProgram()->GetVkDescSet()) {
        mStateManager.GetActiveShaderProgram()->UpdateBuiltInUniformData(mStateManager.GetViewportTransformationState()->GetMinDepthRange(),
                                                                         mStateManager.GetViewportTransformationState()->GetMaxDepthRange());
        mStateManager.GetActiveShaderProgram()->UpdateDescriptorSet();
        vkCmdBindDescriptorSets(*CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mStateManager.GetActiveShaderProgram()->GetVkPipelineLayout(), 0, 1, mStateManager.GetActiveShaderProgram()->GetVkDescSet(), 0, NULL);
    }
}

bool Context::AllocateExplicitIndexBuffer(const void *data, size_t size, BufferObject** ibo) {

    FUN_ENTRY(GL_LOG_TRACE);

    if(mExplicitIbo != nullptr) {
        mCacheManager->CacheVBO(mExplicitIbo);
        mExplicitIbo = nullptr;
    }

    mExplicitIbo = new IndexBufferObject(mVkContext);
    mExplicitIbo->SetTarget(GL_ELEMENT_ARRAY_BUFFER);
    *ibo = mExplicitIbo;

    return mExplicitIbo->Allocate(size, data);
}

bool Context::ConvertIndexBufferToUint16(const void* srcData, size_t elementCount, BufferObject** ibo)
{
    FUN_ENTRY(GL_LOG_TRACE);

    uint16_t *converted_indices_u16 = new uint16_t[elementCount];
    size_t actual_size = elementCount * sizeof(uint16_t);

    bool validatedBuffer = ConvertBuffer<uint8_t, uint16_t>(srcData, converted_indices_u16, elementCount);
    if(validatedBuffer) {
        validatedBuffer = AllocateExplicitIndexBuffer(converted_indices_u16, actual_size, ibo);
    }
    delete[] converted_indices_u16;

    return validatedBuffer;
}

void Context::LineLoopConvertion(void * data, uint32_t vertCount, size_t elementByteSize)
{
    FUN_ENTRY(GL_LOG_TRACE);

    memcpy((uint8_t*)data + (vertCount-1)*elementByteSize, data, elementByteSize);
}

void Context::BindVertexBuffers(VkCommandBuffer *CmdBuffer, const void *indices, GLenum type, bool indexed, uint32_t vertCount)
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(mStateManager.GetActiveShaderProgram()->GetActiveVertexVkBuffersCount()) {
        VkDeviceSize offsets[mStateManager.GetActiveShaderProgram()->GetActiveVertexVkBuffersCount()];
        memset(offsets, 0, sizeof(VkDeviceSize) * mStateManager.GetActiveShaderProgram()->GetActiveVertexVkBuffersCount());
        vkCmdBindVertexBuffers(*CmdBuffer, 0, mStateManager.GetActiveShaderProgram()->GetActiveVertexVkBuffersCount(), mStateManager.GetActiveShaderProgram()->GetActiveVertexVkBuffers(), offsets);
    }

    if(!indexed) {
        return;
    }

    VkDeviceSize offset = 0;
    BufferObject *ibo = nullptr;
    bool validatedBuffer = true;
    size_t actual_size = vertCount * (type == GL_UNSIGNED_INT ? sizeof(GLuint) : sizeof(GLushort));

    // Index buffer requires special handling for passing data and handling unsigned bytes:
    // - If there is a index buffer bound, use the indices parameter as offset.
    // - Otherwise, indices contains the index buffer data. Therefore create a temporary object and store the data there.
    // If the data format is GL_UNSIGNED_BYTE (not supported by Vulkan), convert the data to uint16 and pass this instead.
    if(mStateManager.GetActiveObjectsState()->GetActiveBufferObject(GL_ELEMENT_ARRAY_BUFFER) != nullptr) {
        ibo = mStateManager.GetActiveObjectsState()->GetActiveBufferObject(GL_ELEMENT_ARRAY_BUFFER);
        offset = reinterpret_cast<VkDeviceSize>(indices);

        if(type == GL_UNSIGNED_BYTE) {
            actual_size = ibo->GetSize();
            assert(actual_size > 0);
            uint8_t *srcData     = new uint8_t[actual_size];
            ibo->GetData(actual_size, offset, srcData);
            offset = 0;
            validatedBuffer = ConvertIndexBufferToUint16(srcData, vertCount, &ibo);
            delete[] srcData;
        }

    } else {
        if(type == GL_UNSIGNED_BYTE) {
            validatedBuffer = ConvertIndexBufferToUint16(indices, vertCount, &ibo);
        } else {
            validatedBuffer = AllocateExplicitIndexBuffer(indices, actual_size, &ibo);
        }
    }

    if(mStateManager.GetInputAssemblyState()->GetPrimitiveMode() == GL_LINE_LOOP && ibo->GetTarget() == GL_ELEMENT_ARRAY_BUFFER) {
            size_t sizeOne = (type == GL_UNSIGNED_INT ? sizeof(GLuint) : sizeof(GLushort));
            void *srcData     = new uint8_t[vertCount*sizeOne];

            ibo->GetData(actual_size - sizeOne, offset, srcData);
            LineLoopConvertion(srcData, vertCount, sizeOne);

            validatedBuffer = AllocateExplicitIndexBuffer(srcData, actual_size, &ibo);
            delete[] (uint8_t*)srcData;
        }

    if(validatedBuffer) {
        vkCmdBindIndexBuffer(*CmdBuffer, ibo->GetVkBuffer(), offset, type == GL_UNSIGNED_INT ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
    }
}

void Context::DrawGeometry(VkCommandBuffer *CmdBuffer, bool indexed, uint32_t firstVertex, uint32_t vertCount)
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(indexed == false) {
        vkCmdDraw(*CmdBuffer, vertCount, 1, firstVertex, 0);
    } else {
        vkCmdDrawIndexed(*CmdBuffer, vertCount, 1, 0, 0, 0);
    }
}

void
Context::DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(count < 0) {
        RecordError(GL_INVALID_VALUE);
        return;
    }

    if(mode > GL_TRIANGLE_FAN) {
        RecordError(GL_INVALID_ENUM);
        return;
    }

    if(mode == GL_LINE_LOOP) {
        mIsModeLineLoop = true;
    }

    if(CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        RecordError(GL_INVALID_FRAMEBUFFER_OPERATION);
        return;
    }

    if(mStateManager.GetRasterizationState()->GetCullFace() == GL_FRONT_AND_BACK && IsDrawModeTriangle(mode)) {
        return;
    }

    if(!mStateManager.GetActiveShaderProgram() || !count) {
        return;
    }

    if(mStateManager.GetInputAssemblyState()->UpdatePrimitiveMode(mode)) {
        mPipeline->SetInputAssemblyTopology(GlPrimitiveTopologyToVkPrimitiveTopology(mStateManager.GetInputAssemblyState()->GetPrimitiveMode()));
    }

    PushGeometry(count, first, false, GL_INVALID_ENUM, NULL);

    if(mode == GL_LINE_LOOP) {
        mIsModeLineLoop = false;
    }
}

void
Context::DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if( (mode > GL_TRIANGLE_FAN)  || !(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) ) {
        RecordError(GL_INVALID_ENUM);
        return;
    }

    if(count < 0) {
        RecordError(GL_INVALID_VALUE);
        return;
    }

    if(CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        RecordError(GL_INVALID_FRAMEBUFFER_OPERATION);
        return;
    }

    if(mStateManager.GetRasterizationState()->GetCullFace() == GL_FRONT_AND_BACK && IsDrawModeTriangle(mode)) {
        return;
    }

    if(!mStateManager.GetActiveShaderProgram() || !count) {
        return;
    }

    if(mStateManager.GetInputAssemblyState()->UpdatePrimitiveMode(mode)) {
        mPipeline->SetInputAssemblyTopology(GlPrimitiveTopologyToVkPrimitiveTopology(mStateManager.GetInputAssemblyState()->GetPrimitiveMode()));
    }

    PushGeometry(count, 0, (bool)count, type, indices);
}

void
Context::Finish(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(!Flush()) {
        return;
    }

    mCommandBufferManager->WaitLastSubmition();

    if(!mWriteFBO->IsInDeleteState()) {
        if(mWriteFBO == mSystemFBO) {
            if(mWriteFBO->GetSurfaceType() == GLOVE_SURFACE_WINDOW) {
                mWriteFBO->PrepareVkImage(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            } else if (mWriteFBO->GetSurfaceType() == GLOVE_SURFACE_PBUFFER) {
                if(mSystemFBO->GetBindToTexture()) {
                    mWriteFBO->PrepareVkImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                }
            }
        } else {
            mWriteFBO->PrepareVkImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
    mWriteFBO->SetStateIdle();

    mCacheManager->CleanUpCaches();
}

bool
Context::Flush(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mWriteFBO && mWriteFBO->EndVkRenderPass()) {
        mCommandBufferManager->EndVkDrawCommandBuffer();
        mCommandBufferManager->SubmitVkDrawCommandBuffer();
        return true;
    }
    return false;
}

void
Context::SetClearRect(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mWriteFBO->IsUpdated();

    int x = 0;
    int y = 0;
    int w = mWriteFBO->GetWidth();
    int h = mWriteFBO->GetHeight();

    if(mStateManager.GetFragmentOperationsState()->GetScissorTestEnabled()) {
        x = mStateManager.GetFragmentOperationsState()->GetScissorRectX();
        y = mWriteFBO->GetHeight() - mStateManager.GetFragmentOperationsState()->GetScissorRectY() - mStateManager.GetFragmentOperationsState()->GetScissorRectHeight();

        if(x < mWriteFBO->GetX()) {
            w = mStateManager.GetFragmentOperationsState()->GetScissorRectWidth() + x;
        } else {
            w = mStateManager.GetFragmentOperationsState()->GetScissorRectWidth();
            if(w > mWriteFBO->GetWidth() - x) {
                w = mWriteFBO->GetWidth() - x;
            }
        }

        if(y < mWriteFBO->GetY()) {
            h = mStateManager.GetFragmentOperationsState()->GetScissorRectHeight() + y;
        } else {
            h = mStateManager.GetFragmentOperationsState()->GetScissorRectHeight();
            if(h > mWriteFBO->GetHeight() - y) {
                h = mWriteFBO->GetHeight() - y;
            }
        }
    }

    x = x > mWriteFBO->GetWidth()  ? mWriteFBO->GetX() : x;
    y = y > mWriteFBO->GetHeight() ? mWriteFBO->GetY() : y;
    w = w < mWriteFBO->GetX()      ? mWriteFBO->GetX() : w;
    h = h < mWriteFBO->GetY()      ? mWriteFBO->GetY() : h;

    mClearRect.x      = std::max(mWriteFBO->GetX()     , x);
    mClearRect.y      = std::max(mWriteFBO->GetY()     , y);
    mClearRect.width  = std::min(mWriteFBO->GetWidth() , w);
    mClearRect.height = std::min(mWriteFBO->GetHeight(), h);
}

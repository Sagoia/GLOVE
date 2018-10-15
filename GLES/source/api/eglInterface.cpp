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
 *  @file       eglInterface.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      EGL to OpenGL ES client API connection functionality
 *
 */

#include "rendering_api_interface.h"
#include "context/context.h"
#include "glFunctions.h"

static vkInterface_t  vkInterface;
api_state_t           gles2_state = nullptr;

api_state_t           init_API();
          void        terminate_API();
api_context_t         create_context();
void                  set_read_write_surface(api_context_t api_context, EGLSurfaceInterface *eglReadSurfaceInterface, EGLSurfaceInterface *eglWriteSurfaceInterface);
void                  delete_context(api_context_t api_context);
void                  release_system_fbo(api_context_t api_context);
void                  set_next_image_index(api_context_t api_context, uint32_t index);
GLPROC                get_proc_addr(const char* procname);
void                  flush(api_context_t api_context);
void                  finish(api_context_t api_context);
void                  bind_to_texture(api_context_t api_context, uint32_t bind);

static void           FillInVkInterface(vulkanAPI::vkContext_t* vkContext);

rendering_api_interface_t GLES2Interface = {
    gles2_state,
    init_API,
    terminate_API,
    create_context,
    set_read_write_surface,
    delete_context,
    release_system_fbo,
    set_next_image_index,
    get_proc_addr,
    flush,
    finish,
    bind_to_texture
};

static void FillInVkInterface(vulkanAPI::vkContext_t* vkContext)
{
    vkInterface.vkInstance = vkContext->vkInstance;
    vkInterface.vkGpus = &vkContext->vkGpus[0];
    vkInterface.vkQueue = vkContext->vkQueue;
    vkInterface.vkGraphicsQueueNodeIndex = vkContext->vkGraphicsQueueNodeIndex;
    vkInterface.vkDeviceMemoryProperties = vkContext->vkDeviceMemoryProperties;
    vkInterface.vkDevice = vkContext->vkDevice;
    vkInterface.vkSyncItems = vkContext->vkSyncItems;
}

api_state_t init_API()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    vulkanAPI::InitContext();

    FillInVkInterface(vulkanAPI::GetContext());

    return reinterpret_cast<api_state_t>(&vkInterface);
}

void terminate_API()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    vulkanAPI::TerminateContext();
    GLLogger::Shutdown();
}

api_context_t create_context()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = new Context();
    return ctx;
}

void set_read_write_surface(api_context_t api_context, EGLSurfaceInterface *eglReadSurfaceInterface, EGLSurfaceInterface *eglWriteSurfaceInterface)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    ctx->SetReadWriteSurfaces(eglReadSurfaceInterface, eglWriteSurfaceInterface);

    SetCurrentContext(ctx);
}

void delete_context(api_context_t api_context)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    delete ctx;
}

void release_system_fbo(api_context_t api_context)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    ctx->ReleaseSystemFBO();
}

void set_next_image_index(api_context_t api_context, uint32_t index)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    ctx->SetNextImageIndex(index);
}

GLPROC get_proc_addr(const char* procname)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    return GetGLProcAddr(procname);
}

void flush(api_context_t api_context)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    ctx->Flush();
}

void finish(api_context_t api_context)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    ctx->Finish();
}

void bind_to_texture(api_context_t api_context, uint32_t bind)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Context *ctx = reinterpret_cast<Context *>(api_context);
    ctx->BindToTexture(bind);
}

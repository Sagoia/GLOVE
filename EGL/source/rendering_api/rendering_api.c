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
 *  @file       rendering_api.c
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      It connects EGL to the client (rendering) API at runtime.
 *
 */

#include "utils/egl_defs.h"
#include "rendering_api.h"
#include <string.h>
#include <stdio.h>
#ifndef WIN32
#include <dlfcn.h>
#endif
#include <stdbool.h>

/*
There GLESv2 library is defined under a different name on Android platform
because the dlopen() function gives priority to system libraries and thus loads the
system's GLESv2.
*/
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    #define GLESv2_LIBRARY_NAME "libGLESv2_GLOVE.so"
    #define GLESv2_INTERFACE_NAME "GLES2Interface"
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    #define GLESv2_LIBRARY_NAME "libGLESv2.dll"
    #define GLESv2_INTERFACE_NAME "GetGLES2Interface"
#else
    #define GLESv2_LIBRARY_NAME "libGLESv2.so"
    #define GLESv2_INTERFACE_NAME "GLES2Interface"
#endif //VK_USE_PLATFORM_ANDROID_KHR

typedef struct rendering_api_library_info {
    bool initialized;
    bool loaded;
    void *handle;
    int refCount;
} rendering_api_library_info_t;

static rendering_api_library_info_t gles1_library_info = { false, false, NULL, 0 };
static rendering_api_library_info_t gles2_library_info = { false, false, NULL, 0 };
static rendering_api_library_info_t vg_library_info    = { false, false, NULL, 0 };

static rendering_api_interface_t *gles1_interface = NULL;
static rendering_api_interface_t *gles2_interface = NULL;
static rendering_api_interface_t *vg_interface    = NULL;

static void rendering_api_cache_interface(rendering_api_interface_t* api_interface, uint32_t client_version, EGLenum api);
static rendering_api_return_e rendering_api_get_api_interface(const char *library_name,
                                                              const char *api_interface_name,
                                                              rendering_api_interface_t **api_interface_ret,
                                                              rendering_api_library_info_t *library_info);

static bool rendering_terminate_api(rendering_api_interface_t *api_interface, rendering_api_library_info_t *library_info);

static void rendering_api_cache_interface(rendering_api_interface_t* api_interface, uint32_t client_version, EGLenum api)
{
    switch(api) {
        case EGL_OPENGL_ES_API:
            if(EGL_GL_VERSION_1 == client_version) {
                gles1_interface = api_interface;

            } else {
                gles2_interface = api_interface;

            }
            break;
        case EGL_OPENVG_API:
            vg_interface = api_interface;
            break;
        default:
            NOT_REACHED();
    }
}

static rendering_api_return_e rendering_api_get_api_interface(const char *library_name,
                                                              const char *api_interface_name,
                                                              rendering_api_interface_t **api_interface_ret,
                                                              rendering_api_library_info_t *library_info)
{
    rendering_api_interface_t *api_interface = NULL;
    char *error = NULL;

    if(true == library_info->loaded) {
        library_info->refCount++;
        return RENDERING_API_LOAD_SUCCESS;
    }

#ifndef WIN32
    library_info->handle = dlopen(library_name, RTLD_NOW);
    if(!library_info->handle) {
        fprintf(stderr, "%s\n", dlerror());
        return RENDERING_API_NOT_FOUND;
    }

    dlerror();

    api_interface = (rendering_api_interface_t *) dlsym(library_info->handle, api_interface_name);

    if(!api_interface->init_API_cb) {
        dlclose(library_info->handle);
        char* error = dlerror();
        if(error)  {
            fprintf(stderr, "%s\n", error);
            return false;
        }
        return RENDERING_API_LOAD_ERROR;
    }

    error = dlerror();
    if(error)  {
        fprintf(stderr, "%s\n", error);
        return RENDERING_API_NOT_FOUND;
    }
#else
    EXTERN_C IMAGE_DOS_HEADER __ImageBase;
    CHAR DllPath[MAX_PATH] = { 0 };
    GetModuleFileName((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));
    char *r = strrchr(DllPath, '\\');
    strcpy(r + 1, library_name);
    library_info->handle = LoadLibrary(DllPath);
    if (!library_info->handle) {
        library_info->handle = LoadLibrary(library_name);
        if (!library_info->handle) {
            fprintf(stderr, "0x%x\n", GetLastError());
            return RENDERING_API_NOT_FOUND;
        }
    }

    typedef rendering_api_interface_t * p_api_interface;
    typedef p_api_interface(*func_get_api_interface)();
    func_get_api_interface get_api_interface = (func_get_api_interface)GetProcAddress(library_info->handle, api_interface_name);
    api_interface = get_api_interface();

    if (!api_interface || !api_interface->init_API_cb) {
        FreeLibrary(library_info->handle);
        return RENDERING_API_LOAD_ERROR;
    }
#endif

    library_info->loaded = true;
    *api_interface_ret = api_interface;

    return RENDERING_API_LOAD_SUCCESS;
}

rendering_api_interface_t *RENDERING_API_get_interface(EGLenum api, uint32_t client_version)
{
    rendering_api_interface_t* api_interface = NULL;

    switch(api) {
        case EGL_OPENGL_ES_API: {
                if(EGL_GL_VERSION_1 == client_version) {
                   api_interface = gles1_interface;
                } else {
                   api_interface = gles2_interface;
                }
                break;
            }
        case EGL_OPENVG_API:
			api_interface = vg_interface;
			break;
        default:              { NOT_REACHED(); break; }
    }

    return api_interface;
}

rendering_api_return_e RENDERING_API_get_api_properties(EGLenum api, uint32_t client_version,
                                                rendering_api_interface_t **api_interface, rendering_api_library_info_t **library_info,
                                                const char **api_library_name, const char **api_interface_name)
{
    if(EGL_OPENGL_ES_API == api && (EGL_GL_VERSION_1 != client_version &&
                                    EGL_GL_VERSION_2 != client_version)) {
        return RENDERING_API_INPUT_ERROR;
    }

    switch(api) {
        case EGL_OPENGL_ES_API:
            if(EGL_GL_VERSION_1 == client_version) {
                *api_library_name = "libGLESv1_CM.so";
                *api_interface_name = "GLES1Interface";
                *api_interface = gles1_interface;
                *library_info = &gles1_library_info;
            } else {
                *api_library_name = GLESv2_LIBRARY_NAME;
                *api_interface_name = GLESv2_INTERFACE_NAME;
                *api_interface = gles2_interface;
                *library_info = &gles2_library_info;
            }
            break;
        case EGL_OPENVG_API:
            *api_library_name = "libVG.so";
            *api_interface_name = "VGInterface";
            *api_interface = vg_interface;
            *library_info = &vg_library_info;
            break;
        default:
            {
                NOT_REACHED();
                return RENDERING_API_INPUT_ERROR;
            }
    }

    return RENDERING_API_INPUT_SUCCESS;
}

rendering_api_return_e RENDERING_API_load_api_information(EGLenum api, uint32_t client_version,
                                              rendering_api_interface_t **api_interface_ret,
                                              rendering_api_library_info_t **api_library_info_ret)
{
    rendering_api_interface_t *api_interface = NULL;
    rendering_api_library_info_t *library_info = NULL;
    const char *api_library_name = NULL;
    const char *api_interface_name = NULL;

    rendering_api_return_e ret = RENDERING_API_get_api_properties(api, client_version, &api_interface,
                                                                  &library_info, &api_library_name, &api_interface_name);
    if(ret == RENDERING_API_INPUT_ERROR) {
        return ret;
    }

    ret = rendering_api_get_api_interface(api_library_name, api_interface_name, &api_interface, library_info);

    if(ret != RENDERING_API_LOAD_SUCCESS) {
        return ret;
    }

    rendering_api_cache_interface(api_interface, client_version, api);
    *api_interface_ret = api_interface;
    *api_library_info_ret = library_info;
    return ret;
}

rendering_api_return_e RENDERING_API_load_api(EGLenum api, uint32_t client_version, rendering_api_interface_t **api_interface_ret)
{
    rendering_api_library_info_t *library_info = NULL;
    return RENDERING_API_load_api_information(api, client_version, api_interface_ret, &library_info);
}

rendering_api_return_e RENDERING_API_init_api(EGLenum api, uint32_t client_version, rendering_api_interface_t **api_interface_ret)
{
    rendering_api_library_info_t *library_info = NULL;
    rendering_api_return_e ret = RENDERING_API_load_api_information(api, client_version, api_interface_ret, &library_info);

    if(ret != RENDERING_API_LOAD_SUCCESS) {
        return ret;
    }

    if(library_info->initialized == false) {
        (*api_interface_ret)->state = (*api_interface_ret)->init_API_cb();
        library_info->initialized = true;
    }

    return RENDERING_API_INIT_SUCCESS;
}

static bool rendering_terminate_api(rendering_api_interface_t *api_interface, rendering_api_library_info_t *library_info)
{
    if(library_info->refCount > 0) {
        --library_info->refCount;
        return false;
    }

    if(library_info->initialized && api_interface->terminate_API_cb) {
        api_interface->terminate_API_cb();
    }

#ifndef WIN32
    dlclose(library_info->handle);
    char* error = dlerror();
    if(error)  {
        fprintf(stderr, "%s\n", error);
        return false;
    }
#else
    FreeLibrary(library_info->handle);
#endif

    library_info->loaded = false;
    library_info->initialized = false;
    library_info->handle = NULL;
    return true;
}


rendering_api_interface_t *RENDERING_API_get_gles1_interface()
{
    return gles1_interface;
}

rendering_api_interface_t *RENDERING_API_get_gles2_interface()
{
    return gles2_interface;
}

rendering_api_interface_t *RENDERING_API_get_vg_interface()
{
    return vg_interface;
}

void RENDERING_API_terminate_gles1_api()
{
    if(rendering_terminate_api(gles1_interface, &gles1_library_info)) {
        gles1_interface = NULL;
    }
}

void RENDERING_API_terminate_gles2_api()
{
    if(rendering_terminate_api(gles2_interface, &gles2_library_info)) {
        gles2_interface = NULL;
    }
}

void RENDERING_API_terminate_vg_api()
{
    if(rendering_terminate_api(vg_interface, &vg_library_info)) {
        vg_interface = NULL;
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "WSIWindows.h"

EGLBoolean
WSIWindows::Initialize()
{
    FUN_ENTRY(DEBUG_DEPTH);

    if (VulkanWSI::Initialize() == EGL_FALSE) {
        return EGL_FALSE;
    }

    if (SetPlatformCallbacks() == EGL_FALSE) {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean
WSIWindows::SetPlatformCallbacks(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    memset(&mWsiWin32Callbacks, 0, sizeof(mWsiWin32Callbacks));

    // VK_KHR_xcb_surface functions
    GET_WSI_FUNCTION_PTR(mWsiWin32Callbacks, CreateWin32SurfaceKHR);

    return EGL_TRUE;
}

VkSurfaceKHR
WSIWindows::CreateSurface(EGLDisplay_t* dpy, EGLNativeWindowType win, EGLSurface_t *surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if (!surface) {
        return VK_NULL_HANDLE;
    }

    if (!surface->GetWidth() || !surface->GetHeight()) {
        WINDOWINFO info;
        GetWindowInfo(win, &info);

        surface->SetWidth(info.rcClient.right - info.rcClient.left);
        surface->SetHeight(info.rcClient.bottom - info.rcClient.top);
    }

    VkSurfaceKHR vkSurface;
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    memset(static_cast<void *>(&surfaceCreateInfo), 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
    surfaceCreateInfo.hwnd = win;

    if (VK_SUCCESS != mWsiWin32Callbacks.fpCreateWin32SurfaceKHR(mVkInterface->vkInstance, &surfaceCreateInfo, NULL, &vkSurface)) {
        return VK_NULL_HANDLE;
    }

    return vkSurface;
}

#endif //#ifdef VK_USE_PLATFORM_WIN32_KHR
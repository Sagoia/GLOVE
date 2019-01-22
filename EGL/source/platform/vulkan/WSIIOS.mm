#ifdef VK_USE_PLATFORM_IOS_MVK

#include "WSIIOS.h"
#import <UIKit/UIKit.h>

EGLBoolean
WSIIOS::Initialize()
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
WSIIOS::SetPlatformCallbacks(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    memset(&mWsiIOSCallbacks, 0, sizeof(mWsiIOSCallbacks));

    // VK_KHR_xcb_surface functions
    GET_WSI_FUNCTION_PTR(mWsiIOSCallbacks, CreateIOSSurfaceMVK);

    return EGL_TRUE;
}

VkSurfaceKHR
WSIIOS::CreateSurface(EGLDisplay_t* dpy, EGLNativeWindowType win, EGLSurface_t *surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if (!surface) {
        return VK_NULL_HANDLE;
    }

    if (!surface->GetWidth() || !surface->GetHeight()) {
        UIView *view = (__bridge UIView *)win;
        CGFloat frameScale = view.contentScaleFactor;
        CGSize frameSize = view.frame.size;
        frameScale = 1.0;
        surface->SetWidth(frameSize.width * frameScale);
        surface->SetHeight(frameSize.height * frameScale);
    }

    VkSurfaceKHR vkSurface;
    VkIOSSurfaceCreateInfoMVK surfaceCreateInfo;
    memset(static_cast<void *>(&surfaceCreateInfo), 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.pView = win;

    if (VK_SUCCESS != mWsiIOSCallbacks.fpCreateIOSSurfaceMVK(mVkInterface->vkInstance, &surfaceCreateInfo, NULL, &vkSurface)) {
        return VK_NULL_HANDLE;
    }

    return vkSurface;
}

#endif //#ifdef VK_USE_PLATFORM_IOS_MVK

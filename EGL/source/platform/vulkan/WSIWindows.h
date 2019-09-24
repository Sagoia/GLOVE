#ifndef __WSI_WIN32_H__
#define __WSI_WIN32_H__

#include "vulkanWSI.h"

class WSIWindows : public VulkanWSI
{
protected:
    typedef struct wsiWin32Callbacks {
        // VK_KHR_Win32_surface functions
        PFN_vkCreateWin32SurfaceKHR                     fpCreateWin32SurfaceKHR;
    } wsiWin32Callbacks_t;

    wsiWin32Callbacks_t                                 mWsiWin32Callbacks;

    EGLBoolean         SetPlatformCallbacks() override;

public:
    WSIWindows() {}
    ~WSIWindows() override {}

    EGLBoolean         Initialize() override;
    VkSurfaceKHR       CreateSurface(EGLDisplay_t* dpy,
                                    EGLNativeWindowType win,
                                    EGLSurface_t *surface) override;
};

#endif //__WSI_WIN32_H__

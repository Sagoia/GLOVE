#ifndef __WSI_IOS_H__
#define __WSI_IOS_H__

#include "vulkanWSI.h"

class WSIIOS : public VulkanWSI
{
protected:
    typedef struct wsiIOSCallbacks {
        // VK_KHR_Win32_surface functions
        PFN_vkCreateIOSSurfaceMVK                     fpCreateIOSSurfaceMVK;
    } wsiIOSCallbacks_t;

    wsiIOSCallbacks_t                                 mWsiIOSCallbacks;

    EGLBoolean         SetPlatformCallbacks() override;

public:
    WSIIOS() {}
    ~WSIIOS() override {}

    EGLBoolean         Initialize() override;
    VkSurfaceKHR       CreateSurface(EGLDisplay_t* dpy,
                                    EGLNativeWindowType win,
                                    EGLSurface_t *surface) override;
};

#endif //__WSI_IOS_H__

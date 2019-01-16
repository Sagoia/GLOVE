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
 *  @file       platformFactory.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Abstract Factory to create platform related objects (Childs of PlatformResources and PlatformWindowInterface).
 *
 */

#include "platformFactory.h"
#include "platform/vulkan/vulkanWindowInterface.h"
#include "platform/vulkan/vulkanResources.h"
#ifdef VK_USE_PLATFORM_XCB_KHR
#include "platform/vulkan/WSIXcb.h"
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#include "platform/vulkan/WSIWindows.h"
#endif
#include "platform/vulkan/WSIPlaneDisplay.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "platform/vulkan/WSIAndroid.h"
#endif // VK_USE_PLATFORM_ANDROID_KHR

PlatformFactory *PlatformFactory::mInstance = nullptr;

PlatformFactory::PlatformFactory()
: mPlatformType(UNKNOWN_PLATFORM)
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

PlatformFactory::~PlatformFactory()
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

PlatformFactory *
PlatformFactory::GetInstance()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(!mInstance) {
        mInstance = new PlatformFactory();
    }

    return mInstance;
}

void
PlatformFactory::DestroyInstance()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(mInstance) {
        delete mInstance;
        mInstance = nullptr;
    }
}

void
PlatformFactory::ChoosePlatform()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    PlatformFactory *platformFactory = PlatformFactory::GetInstance();

#ifdef VK_USE_PLATFORM_XCB_KHR
    platformFactory->SetPlatformType(PlatformFactory::WSI_XCB);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    platformFactory->SetPlatformType(PlatformFactory::WSI_WINDOWS);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    platformFactory->SetPlatformType(PlatformFactory::WSI_ANDROID);
#else
    platformFactory->SetPlatformType(PlatformFactory::WSI_PLANE_DISPLAY);
#endif
}

PlatformWindowInterface *
PlatformFactory::GetWindowInterface()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    PlatformFactory *platformFactory = GetInstance();
    PlatformType platformType = platformFactory->GetPlatformType();

    switch(platformType) {
#ifdef VK_USE_PLATFORM_XCB_KHR
        case WSI_XCB: {
            VulkanWindowInterface *windowInterface = new VulkanWindowInterface();
            WSIXcb *vulkanWSI = new WSIXcb();
            windowInterface->SetWSI(vulkanWSI);
            return windowInterface;
        }
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
        case WSI_WINDOWS: {
            VulkanWindowInterface *windowInterface = new VulkanWindowInterface();
            WSIWindows *vulkanWSI = new WSIWindows();
            windowInterface->SetWSI(vulkanWSI);
            return windowInterface;
        }
#endif

        case WSI_PLANE_DISPLAY: {
            VulkanWindowInterface *windowInterface = new VulkanWindowInterface();
            WSIPlaneDisplay *vulkanWSI = new WSIPlaneDisplay();
            windowInterface->SetWSI(vulkanWSI);
            return windowInterface;
        }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case WSI_ANDROID: {
            VulkanWindowInterface *windowInterface = new VulkanWindowInterface();
            WSIAndroid *vulkanWSI = new WSIAndroid();
            windowInterface->SetWSI(vulkanWSI);
            return windowInterface;
        }
#endif

        case UNKNOWN_PLATFORM:
        default:
            NOT_REACHED();
    }

    return nullptr;
}

PlatformResources *
PlatformFactory::GetResources()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    PlatformFactory *platformFactory = GetInstance();
    PlatformType platformType = platformFactory->GetPlatformType();

    switch(platformType) {
        case WSI_XCB:
        case WSI_WINDOWS:
        case WSI_PLANE_DISPLAY:
            return new VulkanResources();

#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case WSI_ANDROID:
            return new VulkanResources();
#endif

        case UNKNOWN_PLATFORM:
        default:
            NOT_REACHED();
    }

    return nullptr;
}

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
 *  @file       context.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Context Functionality via Vulkan
 *
 *  @section
 *
 *  Before using Vulkan, an application must initialize it by loading the Vulkan
 *  commands, and creating a VkInstance object. Once Vulkan is initialized, devices
 *  and queues are the primary objects used to interact with a Vulkan implementation.
 *
 */

#include "context.h"
#include "memoryAllocator.h"
#include "cbManager.h"

namespace vulkanAPI {

#ifdef ENABLE_VK_DEBUG_REPORTER

#define GLOVE_VK_VALIDATION_LAYERS                      true

#ifdef VK_USE_PLATFORM_XCB_KHR
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                    VK_KHR_XCB_SURFACE_EXTENSION_NAME };
#elif defined (VK_USE_PLATFORM_ANDROID_KHR)
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };
#elif defined (VK_USE_PLATFORM_WIN32_KHR)
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#elif defined (VK_USE_PLATFORM_IOS_MVK)
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                    VK_MVK_IOS_SURFACE_EXTENSION_NAME };
#else // native
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                    VK_KHR_DISPLAY_EXTENSION_NAME };
#endif

#else

#define GLOVE_VK_VALIDATION_LAYERS                      false

#ifdef VK_USE_PLATFORM_XCB_KHR
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_KHR_XCB_SURFACE_EXTENSION_NAME};
#elif defined (VK_USE_PLATFORM_ANDROID_KHR)
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
#elif defined (VK_USE_PLATFORM_WIN32_KHR)
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#elif defined (VK_USE_PLATFORM_IOS_MVK)
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_MVK_IOS_SURFACE_EXTENSION_NAME };
#else // native
static const std::vector<const char*> requiredInstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                                    VK_KHR_DISPLAY_EXTENSION_NAME};
#endif

#endif //ENABLE_VK_DEBUG_REPORTER

static const std::vector<const char*>   requiredDeviceExtensions    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static const std::vector<const char*>   usefulDeviceExtensions      = {VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                                                                       VK_IMG_FORMAT_PVRTC_EXTENSION_NAME};

static const std::vector<const char*>   validationLayerNames        = {"VK_LAYER_LUNARG_standard_validation"};

static char **                          enabledInstanceLayers       = nullptr;

static XContext_t                       GloveVkContext;

static bool
InitVkLayers(uint32_t* nLayers)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult res;
    uint32_t layerCount = 0;
    VkLayerProperties *vkLayerProperties = nullptr;

    do {
        res = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        if(!layerCount || res) {
            break;
        }

        vkLayerProperties = (VkLayerProperties *)realloc(vkLayerProperties, layerCount * sizeof(VkLayerProperties));
        if(!vkLayerProperties) {
            return false;
        }

        res = vkEnumerateInstanceLayerProperties(&layerCount, vkLayerProperties);
    } while(res == VK_INCOMPLETE);

    if(layerCount) {
        enabledInstanceLayers = (char**)malloc(layerCount * sizeof(char*));
    }

    uint32_t layers = 0;
    for (uint32_t i = 0; i < layerCount; ++i) {
        bool enable = false;
        for (auto &layerName : validationLayerNames) {
            if (!strcmp((const char *)layerName, vkLayerProperties[i].layerName)) {
                enable = true;
                break;
            }
        }
        if (enable) {
            enabledInstanceLayers[layers] = (char*)malloc(VK_MAX_EXTENSION_NAME_SIZE * sizeof(char));
            strcpy(enabledInstanceLayers[layers], vkLayerProperties[i].layerName);
            ++layers;
        }
    }

    *nLayers = layers;

    if(vkLayerProperties) {
        free(vkLayerProperties);
        vkLayerProperties = nullptr;
    }
    return true;
}

static bool
CheckVkInstanceExtensions(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult res;
    uint32_t extensionCount = 0;
    VkExtensionProperties *vkExtensionProperties = nullptr;

    do {
        res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        if(!extensionCount || res) {
            break;
        }

        vkExtensionProperties = (VkExtensionProperties *)realloc(vkExtensionProperties, extensionCount * sizeof(*vkExtensionProperties));
        if(!vkExtensionProperties) {
            return false;
        }

        res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensionProperties);
    } while(res == VK_INCOMPLETE);

    std::vector<bool> requiredExtensionsAvailable(requiredInstanceExtensions.size(), false);
    for(uint32_t i = 0; i < extensionCount; ++i) {
        for(uint32_t j = 0; j < requiredInstanceExtensions.size(); ++j) {
            if(!strcmp(requiredInstanceExtensions[j], vkExtensionProperties[i].extensionName)) {
                requiredExtensionsAvailable[j] = true;
                break;
            }
        }
    }

    if(vkExtensionProperties) {
        free(vkExtensionProperties);
        vkExtensionProperties = nullptr;
    }

    for(uint32_t j = 0; j < requiredInstanceExtensions.size(); ++j) {
        if(!requiredExtensionsAvailable[j]) {
            return false;
        }
    }

    GloveVkContext.enabledInstanceExtensions = requiredInstanceExtensions;
    
    return true;
}

static bool
CheckVkDeviceExtensions(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult res;
    uint32_t extensionCount = 0;
    VkExtensionProperties *vkExtensionProperties = nullptr;

    do {
        res = vkEnumerateDeviceExtensionProperties(GloveVkContext.vkGpus[0], nullptr, &extensionCount, nullptr);

        if(!extensionCount || res) {
            break;
        }

        vkExtensionProperties = (VkExtensionProperties *)realloc(vkExtensionProperties, extensionCount * sizeof(*vkExtensionProperties));
        if(!vkExtensionProperties) {
            return false;
        }

        res = vkEnumerateDeviceExtensionProperties(GloveVkContext.vkGpus[0], nullptr, &extensionCount, vkExtensionProperties);
    } while(res == VK_INCOMPLETE);

    std::vector<bool> requiredExtensionsAvailable(requiredDeviceExtensions.size(), false);
    for(uint32_t i = 0; i < extensionCount; ++i) {
        for(uint32_t j = 0; j < requiredDeviceExtensions.size(); ++j) {
            if(!strcmp(requiredDeviceExtensions[j], vkExtensionProperties[i].extensionName)) {
                GloveVkContext.enabledDeviceExtensions.push_back(requiredDeviceExtensions[j]);
                requiredExtensionsAvailable[j] = true;
                break;
            }
        }
    }

    GetContext()->mIsMaintenanceExtSupported = false;
    for(uint32_t i = 0; i < extensionCount; ++i) {
        for(uint32_t j = 0; j < usefulDeviceExtensions.size(); ++j) {
            if(!strcmp(usefulDeviceExtensions[j], vkExtensionProperties[i].extensionName)) {
                GloveVkContext.enabledDeviceExtensions.push_back(usefulDeviceExtensions[j]);
                if (!strcmp(usefulDeviceExtensions[j], VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
                    GetContext()->mIsMaintenanceExtSupported = true;
                }
                break;
            }
        }
    }

    if(vkExtensionProperties) {
        free(vkExtensionProperties);
        vkExtensionProperties = nullptr;
    }

    for(uint32_t j = 0; j < requiredDeviceExtensions.size(); ++j) {
        if(!requiredExtensionsAvailable[j]) {
            printf("\n%s extension is mandatory for GLOVE\n", requiredDeviceExtensions[j]);
            printf("Please link GLOVE to a Vulkan driver which supports the latter\n");
            return false;
        }
    }

    return true;
}

static bool
CreateVkInstance(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t enabledLayerCount = 0;
    if(GLOVE_VK_VALIDATION_LAYERS) {
        if(InitVkLayers(&enabledLayerCount) == false) {
            return false;
        }
    }

    VkApplicationInfo applicationInfo;
    applicationInfo.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext             = nullptr;
    applicationInfo.pApplicationName  = "GLOVE (GL Over Vulkan)";
    applicationInfo.applicationVersion= 1;
    applicationInfo.pEngineName       = "GLOVE (GL Over Vulkan)";
    applicationInfo.engineVersion     = 1;
    applicationInfo.apiVersion        = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo;
    memset(static_cast<void *>(&instanceInfo), 0 ,sizeof(instanceInfo));
    instanceInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext                    = nullptr;
    instanceInfo.flags                    = 0;
    instanceInfo.pApplicationInfo         = &applicationInfo;
    instanceInfo.enabledLayerCount        = enabledLayerCount;
    instanceInfo.ppEnabledLayerNames      = enabledInstanceLayers;
    instanceInfo.enabledExtensionCount    = static_cast<uint32_t>(GloveVkContext.enabledInstanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames  = GloveVkContext.enabledInstanceExtensions.data();

    VkResult err = vkCreateInstance(&instanceInfo, nullptr, &GloveVkContext.vkInstance);
    assert(!err);

    for(uint32_t i = 0; i < enabledLayerCount; ++i) {
       free(enabledInstanceLayers[i]);
    }
    free(enabledInstanceLayers);

    return (err == VK_SUCCESS);
}

static bool
EnumerateVkGpus(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t gpuCount;

    VkResult err;
    err = vkEnumeratePhysicalDevices(GloveVkContext.vkInstance, &gpuCount, nullptr);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    GloveVkContext.vkGpus.resize(gpuCount);

    err = vkEnumeratePhysicalDevices(GloveVkContext.vkInstance, &gpuCount, GloveVkContext.vkGpus.data());
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    vkGetPhysicalDeviceMemoryProperties(GloveVkContext.vkGpus[0], &GloveVkContext.vkDeviceMemoryProperties);

    return true;
}

static bool
InitVkQueueFamilyIndex(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(GloveVkContext.vkGpus[0], &queueFamilyCount, nullptr);
    assert(queueFamilyCount >= 1);
    if(!queueFamilyCount) {
        return false;
    }

    VkQueueFamilyProperties *queueProperties = new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(GloveVkContext.vkGpus[0], &queueFamilyCount, queueProperties);

    uint32_t i;
    for(i = 0; i < queueFamilyCount; ++i) {
        if(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            GloveVkContext.vkGraphicsQueueNodeIndex = i;
            break;
        }
    }

    delete[] queueProperties;
    return i < queueFamilyCount ? true : false;
}

static bool
CreateVkDevice(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(GloveVkContext.vkGpus[0], &deviceFeatures);

    VkPhysicalDeviceFeatures &vkDeviceFeatures = GloveVkContext.vkDeviceFeatures;
    if (deviceFeatures.wideLines)               { vkDeviceFeatures.wideLines = VK_TRUE;}
    if (deviceFeatures.textureCompressionBC)    { vkDeviceFeatures.textureCompressionBC = VK_TRUE; }
    if (deviceFeatures.textureCompressionETC2)  { vkDeviceFeatures.textureCompressionETC2 = VK_TRUE; }

    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queueInfo;
    queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext            = nullptr;
    queueInfo.flags            = 0;
    queueInfo.queueCount       = 1;
    queueInfo.pQueuePriorities = queue_priorities;
    queueInfo.queueFamilyIndex = GloveVkContext.vkGraphicsQueueNodeIndex;

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext                   = nullptr;
    deviceInfo.flags                   = 0;
    deviceInfo.queueCreateInfoCount    = 1;
    deviceInfo.pQueueCreateInfos       = &queueInfo;
    deviceInfo.enabledLayerCount       = 0;
    deviceInfo.ppEnabledLayerNames     = nullptr;
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(GloveVkContext.enabledDeviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = GloveVkContext.enabledDeviceExtensions.data();
    deviceInfo.pEnabledFeatures        = &vkDeviceFeatures;

    VkResult err = vkCreateDevice(GloveVkContext.vkGpus[0], &deviceInfo, nullptr, &GloveVkContext.vkDevice);
    assert(!err);

    return (err == VK_SUCCESS);
}

static bool
CreateVkSemaphores(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    VkResult err;

    GloveVkContext.vkSyncItems = new vkSyncItems_t;

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    err = vkCreateSemaphore(GloveVkContext.vkDevice, &semaphoreCreateInfo, nullptr, &GloveVkContext.vkSyncItems->vkDrawSemaphore);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    err = vkCreateSemaphore(GloveVkContext.vkDevice, &semaphoreCreateInfo, nullptr, &GloveVkContext.vkSyncItems->vkAcquireSemaphore);
    assert(!err);

    if(err != VK_SUCCESS) {
        return false;
    }

    GloveVkContext.vkSyncItems->acquireSemaphoreFlag = true;
    GloveVkContext.vkSyncItems->drawSemaphoreFlag = false;

    return true;
}

static void
InitVkQueue(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    vkGetDeviceQueue(GloveVkContext.vkDevice,
                     GloveVkContext.vkGraphicsQueueNodeIndex,
                     0,
                     &GloveVkContext.vkQueue);
}

static void
CreateMemoryAllocator(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    GloveVkContext.memoryAllocator = new MemoryAllocator(GloveVkContext.vkDevice);
}

#ifdef ENABLE_VK_DEBUG_REPORTER

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugLayerCallback(VkDebugReportFlagsEXT flag, VkDebugReportObjectTypeEXT obj_type, uint64_t obj, size_t location, int32_t code, const char *layer_prefix, const char *message, void *user_data)
{
    fprintf(stderr, "Vulkan validation layer log: %s \n", message);
    return VK_FALSE;
}


static bool
CreateVkDebugReporter()
{
    PFN_vkCreateDebugReportCallbackEXT _vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(GloveVkContext.vkInstance, "vkCreateDebugReportCallbackEXT");
    if (!_vkCreateDebugReportCallbackEXT) {
        return false;
    }

    VkDebugReportCallbackCreateInfoEXT debugReportInfo;
    memset(static_cast<void *>(&debugReportInfo), 0, sizeof(debugReportInfo));
    debugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugReportInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debugReportInfo.pfnCallback = DebugLayerCallback;

    VkResult err = _vkCreateDebugReportCallbackEXT(GloveVkContext.vkInstance, &debugReportInfo, nullptr, &(GloveVkContext.vkDebugReporter));
    assert(!err);

    return (err == VK_SUCCESS);
}

#endif

XContext_t *
GetContext()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    return &GloveVkContext;
}

void
ResetContextResources()
{
    GloveVkContext.vkInstance                   = VK_NULL_HANDLE;
#ifdef ENABLE_VK_DEBUG_REPORTER
    GloveVkContext.vkDebugReporter              = VK_NULL_HANDLE;
#endif
    GloveVkContext.vkGpus.clear();
    GloveVkContext.vkQueue                      = VK_NULL_HANDLE;
    GloveVkContext.vkGraphicsQueueNodeIndex     = 0;
    GloveVkContext.vkDevice                     = VK_NULL_HANDLE;
    GloveVkContext.vkSyncItems                  = nullptr;
    GloveVkContext.memoryAllocator              = nullptr;
    GloveVkContext.mIsMaintenanceExtSupported   = false;
    GloveVkContext.mInitialized                 = false;
    GloveVkContext.enabledInstanceExtensions.clear();
    GloveVkContext.enabledDeviceExtensions.clear();
    memset(static_cast<void*>(&GloveVkContext.vkDeviceMemoryProperties), 0,
           sizeof(VkPhysicalDeviceMemoryProperties));
}

bool
InitContext()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (GloveVkContext.mInitialized == true) {
        return true;
    }

    ResetContextResources();

    if( !CheckVkInstanceExtensions() )  { assert(false); return false; }
    if( !CreateVkInstance() )           { assert(false); return false; }
#ifdef ENABLE_VK_DEBUG_REPORTER
    if( !CreateVkDebugReporter() )      { assert(false); return false; }
#endif
    if( !EnumerateVkGpus() )            { assert(false); return false; }
    if( !InitVkQueueFamilyIndex() )     { assert(false); return false; }
    if( !CheckVkDeviceExtensions() )    { assert(false); return false; }
    if( !CreateVkDevice() )             { assert(false); return false; }
    if( !CreateVkSemaphores() )         { assert(false); return false; }

    InitVkQueue();

    CreateMemoryAllocator();

    GloveVkContext.mInitialized = true;

    return GloveVkContext.mInitialized;
}

void
TerminateContext()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(!GloveVkContext.mInitialized) {
        return;
    }

    if (GloveVkContext.memoryAllocator != nullptr) {
        delete GloveVkContext.memoryAllocator;
        GloveVkContext.memoryAllocator = nullptr;
    }

    if(GloveVkContext.vkSyncItems->vkAcquireSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(GloveVkContext.vkDevice, GloveVkContext.vkSyncItems->vkAcquireSemaphore, nullptr);
        GloveVkContext.vkSyncItems->vkAcquireSemaphore = VK_NULL_HANDLE;
    }

    if(GloveVkContext.vkSyncItems->vkDrawSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(GloveVkContext.vkDevice, GloveVkContext.vkSyncItems->vkDrawSemaphore, nullptr);
        GloveVkContext.vkSyncItems->vkDrawSemaphore = VK_NULL_HANDLE;
    }

#ifdef ENABLE_VK_DEBUG_REPORTER
    if(GloveVkContext.vkDebugReporter != VK_NULL_HANDLE) {
        PFN_vkDestroyDebugReportCallbackEXT _vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(GloveVkContext.vkInstance, "vkDestroyDebugReportCallbackEXT");
        if (_vkDestroyDebugReportCallbackEXT) {
            _vkDestroyDebugReportCallbackEXT(GloveVkContext.vkInstance, GloveVkContext.vkDebugReporter, nullptr);
        }
    }
#endif

    if(GloveVkContext.vkDevice != VK_NULL_HANDLE ) {
        vkDeviceWaitIdle(GloveVkContext.vkDevice);
        vkDestroyDevice(GloveVkContext.vkDevice, nullptr);
        vkDestroyInstance(GloveVkContext.vkInstance, nullptr);
    }

    SafeDelete(GloveVkContext.vkSyncItems);

    ResetContextResources();
}

static bool
ExtensionEnabled(const char *name, std::vector<const char*> &extensionList)
{
    if (!name) {
        return false;
    }
    
    for (auto ext : extensionList) {
        if (!strcmp(ext, name)) {
            return true;
        }
    }
    
    return false;
}
    
bool
InstanceExtensionEnabled(const char *name)
{
    return ExtensionEnabled(name, GloveVkContext.enabledInstanceExtensions);
}

bool
DeviceExtensionEnabled(const char *name)
{
    return ExtensionEnabled(name, GloveVkContext.enabledDeviceExtensions);
}

};

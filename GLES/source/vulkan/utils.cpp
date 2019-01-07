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
 *  @file       utils.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Vulkan Utility Functions
 *
 *  @section
 *
 *  A variety of Vulkan utility functions.
 *
 */

#include "utils.h"
#include "utils/glLogger.h"
#include "utils/parser_helpers.h"

uint32_t
GetVkFormatDepthBits(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_S8_UINT:                 return 0;
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:       return 16;
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D24_UNORM_S8_UINT:       return 24;
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:      return 32;
        default: { NOT_REACHED();               return 0; }
    }
}

uint32_t
GetVkFormatStencilBits(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:      return 8;
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:              return 0;
        default: { NOT_REACHED();               return 0; }
    }
}

VkFormat
FindSupportedFormat(VkPhysicalDevice vkPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice, format, &props);

        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

VkFormat
FindSupportedDepthStencilFormat(VkPhysicalDevice dev, uint32_t depthSize, uint32_t stencilSize)
{
    std::vector<VkFormat> acceptableFormats;
    switch(depthSize) {
    case 0:
    {
        stencilSize ?
                    acceptableFormats = {VK_FORMAT_S8_UINT,
                     VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT} :
                    acceptableFormats = { }; break;
    }
    case 16:
    {
        stencilSize ?
                    acceptableFormats = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT} :
                    acceptableFormats = {VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT}; break;
    }
    case 24:
    {
        stencilSize ?
                    acceptableFormats = {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT} :
                    acceptableFormats = {VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT}; break;
    }
    case 32:
    {
        stencilSize ?
                    acceptableFormats = {VK_FORMAT_D32_SFLOAT_S8_UINT} :
                    acceptableFormats = {VK_FORMAT_D32_SFLOAT}; break;
    }
    default: NOT_REACHED(); break;
    }

    return FindSupportedFormat(
        dev,
        acceptableFormats,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool
VkFormatIsDepthStencil(VkFormat format)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

bool
VkFormatIsDepth(VkFormat format)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

bool
VkFormatIsStencil(VkFormat format)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (format) {
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

bool
VkFormatIsColor(VkFormat format)
{
    FUN_ENTRY(GL_LOG_TRACE);

    return (format != VK_FORMAT_UNDEFINED) && !VkFormatIsDepthStencil(format);
}

#define CASE_STR(c)                     case VK_ ##c: return "VK_" STRINGIFY(c);

const char *
VkResultToString(VkResult res)
{
    switch(res) {
    CASE_STR(SUCCESS);
    CASE_STR(NOT_READY);
    CASE_STR(TIMEOUT);
    CASE_STR(EVENT_SET);
    CASE_STR(EVENT_RESET);
    CASE_STR(INCOMPLETE);
    CASE_STR(ERROR_OUT_OF_HOST_MEMORY);
    CASE_STR(ERROR_OUT_OF_DEVICE_MEMORY);
    CASE_STR(ERROR_INITIALIZATION_FAILED);
    CASE_STR(ERROR_DEVICE_LOST);
    CASE_STR(ERROR_MEMORY_MAP_FAILED);
    CASE_STR(ERROR_LAYER_NOT_PRESENT);
    CASE_STR(ERROR_EXTENSION_NOT_PRESENT);
    CASE_STR(ERROR_FEATURE_NOT_PRESENT);
    CASE_STR(ERROR_INCOMPATIBLE_DRIVER);
    CASE_STR(ERROR_TOO_MANY_OBJECTS);
    CASE_STR(ERROR_FORMAT_NOT_SUPPORTED);
    CASE_STR(ERROR_FRAGMENTED_POOL);
    CASE_STR(ERROR_SURFACE_LOST_KHR);
    CASE_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    CASE_STR(SUBOPTIMAL_KHR);
    CASE_STR(ERROR_OUT_OF_DATE_KHR);
    CASE_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    CASE_STR(ERROR_VALIDATION_FAILED_EXT);
    CASE_STR(ERROR_INVALID_SHADER_NV);

    default:  { NOT_REACHED(); return STRINGIFY(GL_INVALID_ENUM); }
    }
}

#undef CASE_STR

const static uint64_t HASH_START = 5381;

static inline uint64_t
HashBuffer(const uint8_t *bytes, size_t size, uint64_t hash)
{
    for (size_t i = 0; i < size; ++i) {
        int32_t c = bytes[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

uint64_t
HashSamplerInfo(VkSamplerCreateInfo &info)
{
    return HashBuffer((const uint8_t *)(&info), sizeof(VkSamplerCreateInfo), HASH_START);
}


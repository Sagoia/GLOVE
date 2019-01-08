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
HashSamplerInfo(const VkSamplerCreateInfo &info)
{
    const static size_t size = sizeof(VkSamplerCreateInfo) - offsetof(VkSamplerCreateInfo, flags);
    return HashBuffer((const uint8_t *)(&info.flags), size, HASH_START);
}

uint64_t
HashRenderPassInfo(const VkRenderPassCreateInfo &info)
{
    uint64_t hash = HASH_START;
    hash = HashBuffer((const uint8_t *)(&info.flags), sizeof(info.flags), hash);

    hash = HashBuffer((const uint8_t *)(&info.attachmentCount), sizeof(uint32_t), hash);
    hash = HashBuffer((const uint8_t *)(info.pAttachments), info.attachmentCount * sizeof(VkAttachmentDescription), hash);

    hash = HashBuffer((const uint8_t *)(&info.subpassCount), sizeof(uint32_t), hash);
    for (uint32_t i = 0; i < info.subpassCount; ++i) {
        const VkSubpassDescription &subpass = info.pSubpasses[i];
        hash = HashBuffer((const uint8_t *)(&subpass.flags), sizeof(subpass.flags), hash);
        hash = HashBuffer((const uint8_t *)(&subpass.pipelineBindPoint), sizeof(subpass.pipelineBindPoint), hash);

        hash = HashBuffer((const uint8_t *)(&subpass.inputAttachmentCount), sizeof(uint32_t), hash);
        hash = HashBuffer((const uint8_t *)(subpass.pInputAttachments), subpass.inputAttachmentCount * sizeof(VkAttachmentReference), hash);

        hash = HashBuffer((const uint8_t *)(&subpass.colorAttachmentCount), sizeof(uint32_t), hash);
        hash = HashBuffer((const uint8_t *)(subpass.pColorAttachments), subpass.colorAttachmentCount * sizeof(VkAttachmentReference), hash);
        if (subpass.pResolveAttachments) {
            hash = HashBuffer((const uint8_t *)(subpass.pResolveAttachments), subpass.colorAttachmentCount * sizeof(VkAttachmentReference), hash);
        }
        if (subpass.pDepthStencilAttachment) {
            hash = HashBuffer((const uint8_t *)(subpass.pDepthStencilAttachment), sizeof(VkAttachmentReference), hash);
        }

        hash = HashBuffer((const uint8_t *)(&subpass.preserveAttachmentCount), sizeof(uint32_t), hash);
        hash = HashBuffer((const uint8_t *)(subpass.pPreserveAttachments), subpass.preserveAttachmentCount * sizeof(uint32_t), hash);
    }

    hash = HashBuffer((const uint8_t *)(&info.dependencyCount), sizeof(uint32_t), hash);
    hash = HashBuffer((const uint8_t *)(info.pDependencies), info.dependencyCount * sizeof(VkSubpassDependency), hash);

    return hash;
}

uint64_t
HashGraphicsPipelineInfo(const VkGraphicsPipelineCreateInfo &info)
{
    uint64_t hash = HASH_START;
    hash = HashBuffer((const uint8_t *)(&info.flags), sizeof(info.flags), hash);

    hash = HashBuffer((const uint8_t *)(&info.stageCount), sizeof(uint32_t), hash);
    hash = HashBuffer((const uint8_t *)(info.pStages), info.stageCount * sizeof(VkPipelineShaderStageCreateInfo), hash);

    const VkPipelineVertexInputStateCreateInfo &vertexInputState = *info.pVertexInputState;
    hash = HashBuffer((const uint8_t *)(&vertexInputState.flags), sizeof(vertexInputState.flags), hash);
    hash = HashBuffer((const uint8_t *)(&vertexInputState.vertexBindingDescriptionCount), sizeof(uint32_t), hash);
    hash = HashBuffer((const uint8_t *)(vertexInputState.pVertexBindingDescriptions), vertexInputState.vertexBindingDescriptionCount * sizeof(VkVertexInputBindingDescription), hash);
    hash = HashBuffer((const uint8_t *)(&vertexInputState.vertexAttributeDescriptionCount), sizeof(uint32_t), hash);
    hash = HashBuffer((const uint8_t *)(vertexInputState.pVertexAttributeDescriptions), vertexInputState.vertexAttributeDescriptionCount * sizeof(VkVertexInputAttributeDescription), hash);

    const static size_t InputAssemblyStateSize = sizeof(VkPipelineInputAssemblyStateCreateInfo) - offsetof(VkPipelineInputAssemblyStateCreateInfo, flags);
    if (info.pInputAssemblyState) {
        hash = HashBuffer((const uint8_t *)(&(info.pInputAssemblyState->flags)), InputAssemblyStateSize, hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pInputAssemblyState), sizeof(intptr_t), hash);
    }

    const static size_t TessellationStateSize = sizeof(VkPipelineTessellationStateCreateInfo) - offsetof(VkPipelineTessellationStateCreateInfo, flags);
    if (info.pTessellationState) {
        hash = HashBuffer((const uint8_t *)(&(info.pTessellationState->flags)), TessellationStateSize, hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pTessellationState), sizeof(intptr_t), hash);
    }

    if (info.pViewportState) {
        const VkPipelineViewportStateCreateInfo &viewportState = *info.pViewportState;
        hash = HashBuffer((const uint8_t *)(&viewportState.flags), sizeof(viewportState.flags), hash);
        hash = HashBuffer((const uint8_t *)(&viewportState.viewportCount), sizeof(uint32_t), hash);
        if (viewportState.pViewports) {
            hash = HashBuffer((const uint8_t *)(viewportState.pViewports), viewportState.viewportCount * sizeof(VkViewport), hash);
        } else {
            hash = HashBuffer((const uint8_t *)(&viewportState.pViewports), sizeof(intptr_t), hash);
        }
        hash = HashBuffer((const uint8_t *)(&viewportState.scissorCount), sizeof(uint32_t), hash);
        if (viewportState.pScissors) {
            hash = HashBuffer((const uint8_t *)(viewportState.pScissors), viewportState.scissorCount * sizeof(VkRect2D), hash);
        } else {
            hash = HashBuffer((const uint8_t *)(&viewportState.pScissors), sizeof(intptr_t), hash);
        }
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pViewportState), sizeof(intptr_t), hash);
    }

    const static size_t RasterizationStateSize = sizeof(VkPipelineRasterizationStateCreateInfo) - offsetof(VkPipelineRasterizationStateCreateInfo, flags);
    if (info.pRasterizationState) {
        hash = HashBuffer((const uint8_t *)(&(info.pRasterizationState->flags)), RasterizationStateSize, hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pRasterizationState), sizeof(intptr_t), hash);
    }

    const static size_t MultisampleStateSize = sizeof(VkPipelineMultisampleStateCreateInfo) - offsetof(VkPipelineMultisampleStateCreateInfo, flags);
    if (info.pMultisampleState) {
        hash = HashBuffer((const uint8_t *)(&(info.pMultisampleState->flags)), MultisampleStateSize, hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pMultisampleState), sizeof(intptr_t), hash);
    }

    const static size_t DepthStencilStateSize = sizeof(VkPipelineDepthStencilStateCreateInfo) - offsetof(VkPipelineDepthStencilStateCreateInfo, flags);
    if (info.pDepthStencilState) {
        hash = HashBuffer((const uint8_t *)(&(info.pDepthStencilState->flags)), DepthStencilStateSize, hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pDepthStencilState), sizeof(intptr_t), hash);
    }

    if (info.pColorBlendState) {
        const VkPipelineColorBlendStateCreateInfo &colorBlendState = *info.pColorBlendState;
        hash = HashBuffer((const uint8_t *)(&colorBlendState.flags), sizeof(colorBlendState.flags), hash);
        hash = HashBuffer((const uint8_t *)(&colorBlendState.logicOpEnable), sizeof(colorBlendState.logicOpEnable), hash);
        hash = HashBuffer((const uint8_t *)(&colorBlendState.logicOp), sizeof(colorBlendState.logicOp), hash);
        hash = HashBuffer((const uint8_t *)(&colorBlendState.attachmentCount), sizeof(uint32_t), hash);
        hash = HashBuffer((const uint8_t *)(colorBlendState.pAttachments), colorBlendState.attachmentCount * sizeof(VkPipelineColorBlendAttachmentState), hash);
        hash = HashBuffer((const uint8_t *)(colorBlendState.blendConstants), 4 * sizeof(float), hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pColorBlendState), sizeof(intptr_t), hash);
    }

    if (info.pDynamicState) {
        const VkPipelineDynamicStateCreateInfo &dynamicState = *info.pDynamicState;
        hash = HashBuffer((const uint8_t *)(&dynamicState.flags), sizeof(dynamicState.flags), hash);
        hash = HashBuffer((const uint8_t *)(&dynamicState.dynamicStateCount), sizeof(uint32_t), hash);
        hash = HashBuffer((const uint8_t *)(dynamicState.pDynamicStates), dynamicState.dynamicStateCount * sizeof(VkDynamicState), hash);
    } else {
        hash = HashBuffer((const uint8_t *)(&info.pDynamicState), sizeof(intptr_t), hash);
    }

    hash = HashBuffer((const uint8_t *)(&info.layout), sizeof(info.layout), hash);
    hash = HashBuffer((const uint8_t *)(&info.renderPass), sizeof(info.renderPass), hash);
    hash = HashBuffer((const uint8_t *)(&info.subpass), sizeof(info.subpass), hash);
    
    // basePipelineHandle and basePipelineIndex is always 0

    return hash;
}


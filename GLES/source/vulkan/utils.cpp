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
#include "utils/glUtils.h"
#include "utils/glLogger.h"
#include "utils/parser_helpers.h"

namespace vulkanAPI {

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

GLenum
VkFormatToGlInternalformat(VkFormat format)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (format) {
    case VK_FORMAT_R5G6B5_UNORM_PACK16:     return GL_RGB565;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:   return GL_RGBA4;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:   return GL_RGB5_A1;
    case VK_FORMAT_R8G8B8_UNORM:            return GL_RGB8_OES;

    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_R8G8B8A8_UNORM:          return GL_RGBA8_OES;

    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:          return GL_BGRA8_EXT;

    case VK_FORMAT_D16_UNORM:               return GL_DEPTH_COMPONENT16;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:      return GL_DEPTH24_STENCIL8_OES;
    case VK_FORMAT_X8_D24_UNORM_PACK32:     return GL_DEPTH_COMPONENT24_OES;
    case VK_FORMAT_D32_SFLOAT:              return GL_DEPTH_COMPONENT32_OES;
    case VK_FORMAT_S8_UINT:                 return GL_STENCIL_INDEX8;

    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:         return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:        return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case VK_FORMAT_BC2_UNORM_BLOCK:             return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case VK_FORMAT_BC3_UNORM_BLOCK:             return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:     return GL_ETC1_RGB8_OES;
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;

    case VK_FORMAT_UNDEFINED:
    default: { NOT_FOUND_ENUM(format);      return GL_INVALID_VALUE; }
    }
}


VkColorComponentFlags
GLColorMaskToVkColorComponentFlags(GLubyte colorMask)
{
    FUN_ENTRY(GL_LOG_TRACE);

    //assert(colorMask > 0);
    VkColorComponentFlags bits = 0;
    if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_RED)) {
        bits |= VK_COLOR_COMPONENT_R_BIT;
    }
    if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_GREEN)) {
        bits |= VK_COLOR_COMPONENT_G_BIT;
    }
    if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_BLUE)) {
        bits |= VK_COLOR_COMPONENT_B_BIT;
    }
    if (GlColorMaskHasBit(colorMask, GLColorMaskBit::GLC_ALPHA)) {
        bits |= VK_COLOR_COMPONENT_A_BIT;
    }
    return static_cast<VkColorComponentFlags>(bits);
}

VkPolygonMode
GLPrimitiveModeToVkPolygonMode(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_POINTS:                return VK_POLYGON_MODE_POINT;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    case GL_LINES:                 return VK_POLYGON_MODE_LINE;
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLES:             return VK_POLYGON_MODE_FILL;
    default: NOT_FOUND_ENUM(mode); return VK_POLYGON_MODE_FILL;
    }
}

VkSampleCountFlagBits
GlSampleCoverageBitsToVkSampleCountFlagBits(GLint bits)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (bits) {
    case 1:                return VK_SAMPLE_COUNT_1_BIT;
    case 2:                return VK_SAMPLE_COUNT_2_BIT;
    case 4:                return VK_SAMPLE_COUNT_4_BIT;
    case 8:                return VK_SAMPLE_COUNT_8_BIT;
    case 16:                return VK_SAMPLE_COUNT_16_BIT;
    case 32:                return VK_SAMPLE_COUNT_32_BIT;
    case 64:                return VK_SAMPLE_COUNT_64_BIT;
    default: NOT_REACHED(); return VK_SAMPLE_COUNT_1_BIT;
    }
}

VkStencilOp
GlStencilFuncToVkStencilOp(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_KEEP:                     return VK_STENCIL_OP_KEEP;
    case GL_ZERO:                     return VK_STENCIL_OP_ZERO;
    case GL_REPLACE:                  return VK_STENCIL_OP_REPLACE;
    case GL_INCR:                     return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case GL_INCR_WRAP:                return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case GL_DECR:                     return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case GL_DECR_WRAP:                return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    case GL_INVERT:                   return VK_STENCIL_OP_INVERT;
    default: NOT_FOUND_ENUM(mode);    return VK_STENCIL_OP_KEEP;
    }
}

VkLogicOp
GlLogicOpToVkLogicOp(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_CLEAR:                          return VK_LOGIC_OP_CLEAR;
    case GL_AND:                            return VK_LOGIC_OP_AND;
    case GL_AND_REVERSE:                    return VK_LOGIC_OP_AND_REVERSE;
    case GL_COPY:                           return VK_LOGIC_OP_COPY;
    case GL_AND_INVERTED:                   return VK_LOGIC_OP_AND_INVERTED;
    case GL_NOOP:                           return VK_LOGIC_OP_NO_OP;
    case GL_XOR:                            return VK_LOGIC_OP_XOR;
    case GL_OR:                             return VK_LOGIC_OP_OR;
    case GL_NOR:                            return VK_LOGIC_OP_NOR;
    case GL_EQUIV:                          return VK_LOGIC_OP_EQUIVALENT;
    case GL_INVERT:                         return VK_LOGIC_OP_INVERT;
    case GL_OR_REVERSE:                     return VK_LOGIC_OP_OR_REVERSE;
    case GL_COPY_INVERTED:                  return VK_LOGIC_OP_COPY_INVERTED;
    case GL_OR_INVERTED:                    return VK_LOGIC_OP_OR_INVERTED;
    case GL_NAND:                           return VK_LOGIC_OP_NAND;
    case GL_SET:                            return VK_LOGIC_OP_SET;
    default: NOT_FOUND_ENUM(mode);          return VK_LOGIC_OP_CLEAR;
    }
}

VkCompareOp
GlCompareFuncToVkCompareOp(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_NEVER:                          return VK_COMPARE_OP_NEVER;
    case GL_EQUAL:                          return VK_COMPARE_OP_EQUAL;
    case GL_LEQUAL:                         return VK_COMPARE_OP_LESS_OR_EQUAL;
    case GL_GREATER:                        return VK_COMPARE_OP_GREATER;
    case GL_NOTEQUAL:                       return VK_COMPARE_OP_NOT_EQUAL;
    case GL_GEQUAL:                         return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case GL_ALWAYS:                         return VK_COMPARE_OP_ALWAYS;
    case GL_LESS:                           return VK_COMPARE_OP_LESS;
    default: NOT_FOUND_ENUM(mode);          return VK_COMPARE_OP_LESS;
    }
}

VkCullModeFlagBits
GlCullModeToVkCullMode(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_BACK:                           return VK_CULL_MODE_BACK_BIT;
    case GL_FRONT:                          return VK_CULL_MODE_FRONT_BIT;
    case GL_FRONT_AND_BACK:                 return VK_CULL_MODE_FRONT_AND_BACK;
    default: NOT_FOUND_ENUM(mode);          return VK_CULL_MODE_NONE;
    }
}

VkFrontFace
GlFrontFaceToVkFrontFace(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_CW:                             return VK_FRONT_FACE_CLOCKWISE;
    case GL_CCW:                            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    default: NOT_FOUND_ENUM(mode);          return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

VkSamplerAddressMode
GlTexAddressToVkTexAddress(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_CLAMP_TO_EDGE:                  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case GL_REPEAT:                         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case GL_MIRRORED_REPEAT:                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    default: NOT_REACHED();                 return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
}

VkFilter
GlTexFilterToVkTexFilter(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_NEAREST:
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:          return VK_FILTER_NEAREST;

    case GL_LINEAR:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:           return VK_FILTER_LINEAR;
    default: NOT_FOUND_ENUM(mode);          return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode
GlTexMipMapModeToVkMipMapMode(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_LINEAR:
    case GL_NEAREST:
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_NEAREST:          return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:           return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default: NOT_FOUND_ENUM(mode);          return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}

VkPrimitiveTopology
GlPrimitiveTopologyToVkPrimitiveTopology(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_POINTS:                         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case GL_LINES:                          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case GL_LINE_LOOP:
    case GL_LINE_STRIP:                     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case GL_TRIANGLES:                      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case GL_TRIANGLE_STRIP:                 return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case GL_TRIANGLE_FAN:                   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    default: NOT_FOUND_ENUM(mode);          return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

VkBlendFactor
GlBlendFactorToVkBlendFactor(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_ONE:                            return VK_BLEND_FACTOR_ONE;
    case GL_ZERO:                           return VK_BLEND_FACTOR_ZERO;
    case GL_SRC_COLOR:                      return VK_BLEND_FACTOR_SRC_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case GL_DST_COLOR:                      return VK_BLEND_FACTOR_DST_COLOR;
    case GL_ONE_MINUS_DST_COLOR:            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case GL_SRC_ALPHA:                      return VK_BLEND_FACTOR_SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:                      return VK_BLEND_FACTOR_DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case GL_CONSTANT_COLOR:                 return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case GL_ONE_MINUS_CONSTANT_COLOR:       return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case GL_CONSTANT_ALPHA:                 return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case GL_ONE_MINUS_CONSTANT_ALPHA:       return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case GL_SRC_ALPHA_SATURATE:             return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    default: NOT_FOUND_ENUM(mode);          return VK_BLEND_FACTOR_ZERO;
    }
}

VkBlendOp
GlBlendEquationToVkBlendOp(GLenum mode)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (mode) {
    case GL_FUNC_ADD:                       return VK_BLEND_OP_ADD;
    case GL_FUNC_SUBTRACT:                  return VK_BLEND_OP_SUBTRACT;
    case GL_FUNC_REVERSE_SUBTRACT:          return VK_BLEND_OP_REVERSE_SUBTRACT;
    default: NOT_FOUND_ENUM(mode);          return VK_BLEND_OP_ADD;
    }
}

VkFormat
GlInternalFormatToVkFormat(GLenum internalformatDepth, GLenum internalformatStencil)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (internalformatDepth) {
    case GL_DEPTH24_STENCIL8_OES:
    case GL_UNSIGNED_INT_24_8_OES:          return VK_FORMAT_D24_UNORM_S8_UINT;
    case GL_DEPTH_COMPONENT16:              return internalformatStencil == GL_STENCIL_INDEX8 ? VK_FORMAT_D16_UNORM_S8_UINT : VK_FORMAT_D16_UNORM;
    case GL_DEPTH_COMPONENT24_OES:          return internalformatStencil == GL_STENCIL_INDEX8 ? VK_FORMAT_D24_UNORM_S8_UINT : VK_FORMAT_X8_D24_UNORM_PACK32;
    case GL_DEPTH_COMPONENT32_OES:          return internalformatStencil == GL_STENCIL_INDEX8 ? VK_FORMAT_D32_SFLOAT_S8_UINT : VK_FORMAT_D32_SFLOAT;
    default:
        switch (internalformatStencil) {
        case GL_STENCIL_INDEX1_OES:
        case GL_STENCIL_INDEX4_OES:
        case GL_STENCIL_INDEX8:             return VK_FORMAT_S8_UINT;
        default: {                          NOT_FOUND_ENUM(internalformatDepth); NOT_FOUND_ENUM(internalformatStencil); }
        }
    }

    return VK_FORMAT_UNDEFINED;
}

VkFormat
GlInternalFormatToVkFormat(GLenum internalformat)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (internalformat) {
    case GL_RGB565:                           return VK_FORMAT_R5G6B5_UNORM_PACK16;
    case GL_RGBA4:                            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    case GL_RGB5_A1:                          return VK_FORMAT_R5G5B5A1_UNORM_PACK16;

    case GL_RGB8_OES:                         return VK_FORMAT_R8G8B8_UNORM;

    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_RGBA:
    case GL_RGBA8_OES:                        return VK_FORMAT_R8G8B8A8_UNORM;

    case GL_BGRA8_EXT:
    case GL_BGRA_EXT:                         return VK_FORMAT_B8G8R8A8_UNORM;

    case GL_DEPTH_COMPONENT16:                return VK_FORMAT_D16_UNORM;
    case GL_DEPTH24_STENCIL8_OES:
    case GL_UNSIGNED_INT_24_8_OES:            return VK_FORMAT_D24_UNORM_S8_UINT;
    case GL_DEPTH_COMPONENT24_OES:            return VK_FORMAT_X8_D24_UNORM_PACK32;
    case GL_DEPTH_COMPONENT32_OES:            return VK_FORMAT_D32_SFLOAT;

    case GL_STENCIL_INDEX1_OES:
    case GL_STENCIL_INDEX4_OES:
    case GL_STENCIL_INDEX8:                   return VK_FORMAT_S8_UINT;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:     return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:    return VK_FORMAT_BC2_UNORM_BLOCK;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:    return VK_FORMAT_BC3_UNORM_BLOCK;

    case GL_ETC1_RGB8_OES:                    return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;

    case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
    case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
    case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
    case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;

    default: { NOT_FOUND_ENUM(internalformat); return VK_FORMAT_UNDEFINED; }
    }
}

VkFormat GlColorFormatToVkColorFormat(GLenum format, GLenum type)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (type) {
    case GL_UNSIGNED_BYTE: {
        switch (format) {
        case GL_RGB:                        return VK_FORMAT_R8G8B8_UNORM;
        case GL_LUMINANCE:
        case GL_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_RGBA:                       return VK_FORMAT_R8G8B8A8_UNORM;
        case GL_BGRA_EXT:                   return VK_FORMAT_B8G8R8A8_UNORM;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }
    }
    case GL_UNSIGNED_SHORT_5_6_5: {
        assert(format == GL_RGB);
        return          VK_FORMAT_R5G6B5_UNORM_PACK16;
    }
    case GL_UNSIGNED_SHORT_4_4_4_4: {
        assert(format == GL_RGBA);
        return          VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    }
    case GL_UNSIGNED_SHORT_5_5_5_1: {
        assert(format == GL_RGBA);
        return          VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    }
    default: {
        switch (format) {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:       return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:      return VK_FORMAT_BC2_UNORM_BLOCK;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:      return VK_FORMAT_BC3_UNORM_BLOCK;
        case GL_ETC1_RGB8_OES:                      return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:   return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:   return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
        default:                                    return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }
    }
}

VkFormat
GlAttribPointerToVkFormat(GLint nElements, GLenum type, GLboolean normalized)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (type) {
    case GL_FLOAT:
    case GL_FIXED:
        switch (nElements) {
        case 1:                             return VK_FORMAT_R32_SFLOAT;
        case 2:                             return VK_FORMAT_R32G32_SFLOAT;
        case 3:                             return VK_FORMAT_R32G32B32_SFLOAT;
        case 4:                             return VK_FORMAT_R32G32B32A32_SFLOAT;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    case GL_INT:
        switch (nElements) {
        case 1:                             return VK_FORMAT_R32_SINT;
        case 2:                             return VK_FORMAT_R32G32_SINT;
        case 3:                             return VK_FORMAT_R32G32B32_SINT;
        case 4:                             return VK_FORMAT_R32G32B32A32_SINT;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    case GL_UNSIGNED_INT:
        switch (nElements) {
        case 1:                             return VK_FORMAT_R32_UINT;
        case 2:                             return VK_FORMAT_R32G32_UINT;
        case 3:                             return VK_FORMAT_R32G32B32_UINT;
        case 4:                             return VK_FORMAT_R32G32B32A32_UINT;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    case GL_BYTE:
        switch (nElements) {
        case 1:                             return normalized ? VK_FORMAT_R8_SNORM : VK_FORMAT_R8_SSCALED;
        case 2:                             return normalized ? VK_FORMAT_R8G8_SNORM : VK_FORMAT_R8G8_SSCALED;
        case 3:                             return normalized ? VK_FORMAT_R8G8B8_SNORM : VK_FORMAT_R8G8B8_SSCALED;
        case 4:                             return normalized ? VK_FORMAT_R8G8B8A8_SNORM : VK_FORMAT_R8G8B8A8_SSCALED;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    case GL_UNSIGNED_BYTE:
        switch (nElements) {
        case 1:                             return normalized ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_USCALED;
        case 2:                             return normalized ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8G8_USCALED;
        case 3:                             return normalized ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8_USCALED;
        case 4:                             return normalized ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_USCALED;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    case GL_SHORT:
        switch (nElements) {
        case 1:                             return normalized ? VK_FORMAT_R16_SNORM : VK_FORMAT_R16_SSCALED;
        case 2:                             return normalized ? VK_FORMAT_R16G16_SNORM : VK_FORMAT_R16G16_SSCALED;
        case 3:                             return normalized ? VK_FORMAT_R16G16B16_SNORM : VK_FORMAT_R16G16B16_SSCALED;
        case 4:                             return normalized ? VK_FORMAT_R16G16B16A16_SNORM : VK_FORMAT_R16G16B16A16_SSCALED;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    case GL_UNSIGNED_SHORT:
        switch (nElements) {
        case 1:                             return normalized ? VK_FORMAT_R16_UNORM : VK_FORMAT_R16_USCALED;
        case 2:                             return normalized ? VK_FORMAT_R16G16_UNORM : VK_FORMAT_R16G16_USCALED;
        case 3:                             return normalized ? VK_FORMAT_R16G16B16_UNORM : VK_FORMAT_R16G16B16_USCALED;
        case 4:                             return normalized ? VK_FORMAT_R16G16B16A16_UNORM : VK_FORMAT_R16G16B16A16_USCALED;
        default: { NOT_REACHED();           return VK_FORMAT_UNDEFINED; }
        }

    default: { NOT_REACHED();               return VK_FORMAT_UNDEFINED; }
    }
}

VkIndexType
GlToVkIndexType(GLenum type)
{
    FUN_ENTRY(GL_LOG_TRACE);

    switch (type) {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:                 return VK_INDEX_TYPE_UINT16;
    case GL_UNSIGNED_INT:                   return VK_INDEX_TYPE_UINT32;
    case GL_INVALID_ENUM:                   return VK_INDEX_TYPE_MAX_ENUM;
    default: NOT_FOUND_ENUM(type);          return VK_INDEX_TYPE_MAX_ENUM;
    }
}


}

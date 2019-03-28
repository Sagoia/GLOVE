
#ifndef __VKTYPES_H__
#define __VKTYPES_H__

#include "vulkan/vulkan.h"

typedef VkFlags XFlags;
typedef VkBufferUsageFlags XBufferUsageFlags;

// enumerates begin

enum XImageTarget {
    X_IMAGE_TARGET_2D   = 0,
    X_IMAGE_TARGET_CUBE = 1,
    X_IMAGE_TARGET_MAX  = 2
};

enum XBufferUsageFlagBits {
    X_BUFFER_USAGE_UNKNOW           = 0x00,
    X_BUFFER_USAGE_TRANSFER_SRC     = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    X_BUFFER_USAGE_TRANSFER_DST     = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    X_BUFFER_USAGE_UNIFORM_BUFFER   = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    X_BUFFER_USAGE_INDEX_BUFFER     = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    X_BUFFER_USAGE_VERTEX_BUFFER    = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
};

enum XMemoryPropertyFlagBits {
    X_MEMORY_PROPERTY_FLAG_DEFAULT_BIT = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    X_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    X_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM,
};

enum XImageLayout {
    X_IMAGE_LAYOUT_GENERAL                  = VK_IMAGE_LAYOUT_GENERAL,
    X_IMAGE_LAYOUT_COLOR_ATTACHMENT         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    X_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    X_IMAGE_LAYOUT_SHADER_READ              = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    X_IMAGE_LAYOUT_PRESENT                  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    X_IMAGE_LAYOUT_MAX_ENUM                 = 0x7FFFFFFF
};

typedef VkFormat XFormat;

typedef VkColorComponentFlags XColorComponentFlags;

typedef VkBlendOp XBlendOp;

typedef VkBlendFactor XBlendFactor;

typedef VkLogicOp XLogicOp;

typedef VkCompareOp XCompareOp;

typedef VkStencilOp XStencilOp;

typedef VkCullModeFlagBits XCullModeFlagBits;

typedef VkFrontFace XFrontFace;

typedef VkPolygonMode XPolygonMode;

typedef VkPrimitiveTopology XPrimitiveTopology;

typedef VkIndexType XIndexType;

typedef VkFilter XFilter;

typedef VkSampleCountFlagBits XSampleCountFlagBits;

typedef VkSamplerAddressMode XSamplerAddressMode;

typedef VkSamplerMipmapMode XSamplerMipmapMode;

// enumerates end

#endif //__VKTYPES_H__

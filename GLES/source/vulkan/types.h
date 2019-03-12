
#ifndef __VKTYPES_H__
#define __VKTYPES_H__

#include "vulkan/vulkan.h"

enum XBufferUsageFlagBits {
    X_BUFFER_USAGE_UNKNOW           = 0x00,
    X_BUFFER_USAGE_TRANSFER_SRC     = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    X_BUFFER_USAGE_TRANSFER_DST     = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    X_BUFFER_USAGE_UNIFORM_BUFFER   = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    X_BUFFER_USAGE_INDEX_BUFFER     = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    X_BUFFER_USAGE_VERTEX_BUFFER    = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
};

typedef uint32_t XBufferUsageFlags; 

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

#endif //__VKTYPES_H__
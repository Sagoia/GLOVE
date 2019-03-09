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
 *  @file       utils.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Vulkan Utility Functions
 *
 */

#ifndef __VKUTILS_H__
#define __VKUTILS_H__

#include "GLES/gl.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "vulkan/vulkan.h"
#include <vector>
#include "types.h"

namespace vulkanAPI {

uint32_t                GetVkFormatStencilBits(VkFormat format);
uint32_t                GetVkFormatDepthBits(VkFormat format);
VkFormat                FindSupportedDepthStencilFormat(VkPhysicalDevice dev, uint32_t depthSize, uint32_t stencilSize);
VkFormat                FindSupportedFormat(VkPhysicalDevice vkPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
bool                    VkFormatIsDepthStencil(VkFormat format);
bool                    VkFormatIsDepth(VkFormat format);
bool                    VkFormatIsStencil(VkFormat format);
bool                    VkFormatIsColor(VkFormat format);
const char *            VkResultToString(VkResult res);

uint64_t                HashSamplerInfo(const VkSamplerCreateInfo &info);
uint64_t                HashRenderPassInfo(const VkRenderPassCreateInfo &info);
uint64_t                HashGraphicsPipelineInfo(const VkGraphicsPipelineCreateInfo &info);

// type convert
GLenum                  VkFormatToGlInternalformat(VkFormat format);

VkColorComponentFlags   GLColorMaskToVkColorComponentFlags(GLubyte colorMask);
VkBlendFactor           GlBlendFactorToVkBlendFactor(GLenum mode);
VkLogicOp           	GlLogicOpToVkLogicOp(GLenum mode);
VkBlendOp               GlBlendEquationToVkBlendOp(GLenum mode);
VkCompareOp             GlCompareFuncToVkCompareOp(GLenum mode);
VkCullModeFlagBits      GlCullModeToVkCullMode(GLenum mode);
VkFrontFace             GlFrontFaceToVkFrontFace(GLenum mode);
VkPolygonMode           GLPrimitiveModeToVkPolygonMode(GLenum mode);
VkPrimitiveTopology     GlPrimitiveTopologyToVkPrimitiveTopology(GLenum mode);
VkSampleCountFlagBits   GlSampleCoverageBitsToVkSampleCountFlagBits(GLint bits);
VkStencilOp             GlStencilFuncToVkStencilOp(GLenum mode);
VkSamplerAddressMode    GlTexAddressToVkTexAddress(GLenum mode);
VkFilter                GlTexFilterToVkTexFilter(GLenum mode);
VkSamplerMipmapMode     GlTexMipMapModeToVkMipMapMode(GLenum mode);
VkFormat                GlInternalFormatToVkFormat(GLenum internalformat);
VkFormat                GlInternalFormatToVkFormat(GLenum internalformatDepth, GLenum internalformatStencil);
VkFormat                GlAttribPointerToVkFormat(GLint nElements, GLenum type, GLboolean normalized);
VkIndexType             GlToVkIndexType(GLenum type);
VkFormat                GlColorFormatToVkColorFormat(GLenum format, GLenum type);

}

inline GLenum                   XFormatToGlInternalformat(XFormat format) { return vulkanAPI::VkFormatToGlInternalformat((VkFormat)format); }

inline XColorComponentFlags     GLColorMaskToXColorComponentFlags(GLubyte colorMask) { return vulkanAPI::GLColorMaskToVkColorComponentFlags(colorMask); }
inline XBlendFactor             GlBlendFactorToXBlendFactor(GLenum mode) { return vulkanAPI::GlBlendFactorToVkBlendFactor(mode); }
inline XLogicOp                 GlLogicOpToXLogicOp(GLenum mode) { return vulkanAPI::GlLogicOpToVkLogicOp(mode); }
inline XBlendOp                 GlBlendEquationToXBlendOp(GLenum mode) { return vulkanAPI::GlBlendEquationToVkBlendOp(mode); }
inline XCompareOp               GlCompareFuncToXCompareOp(GLenum mode) { return vulkanAPI::GlCompareFuncToVkCompareOp(mode); }
inline XCullModeFlagBits        GlCullModeToXCullMode(GLenum mode) { return vulkanAPI::GlCullModeToVkCullMode(mode); }
inline XFrontFace               GlFrontFaceToXFrontFace(GLenum mode) { return vulkanAPI::GlFrontFaceToVkFrontFace(mode); }
inline XPolygonMode             GLPrimitiveModeToXPolygonMode(GLenum mode) { return vulkanAPI::GLPrimitiveModeToVkPolygonMode(mode); }
inline XPrimitiveTopology       GlPrimitiveTopologyToXPrimitiveTopology(GLenum mode) { return vulkanAPI::GlPrimitiveTopologyToVkPrimitiveTopology(mode); }
inline XSampleCountFlagBits     GlSampleCoverageBitsToXSampleCountFlagBits(GLint bits) { return vulkanAPI::GlSampleCoverageBitsToVkSampleCountFlagBits(bits); }
inline XStencilOp               GlStencilFuncToXStencilOp(GLenum mode) { return vulkanAPI::GlStencilFuncToVkStencilOp(mode); }
inline XSamplerAddressMode      GlTexAddressToXTexAddress(GLenum mode) { return vulkanAPI::GlTexAddressToVkTexAddress(mode); }
inline XFilter                  GlTexFilterToXTexFilter(GLenum mode) { return vulkanAPI::GlTexFilterToVkTexFilter(mode); }
inline XSamplerMipmapMode       GlTexMipMapModeToXMipMapMode(GLenum mode) { return vulkanAPI::GlTexMipMapModeToVkMipMapMode(mode); }
inline XFormat                  GlInternalFormatToXFormat(GLenum internalformat) { return vulkanAPI::GlInternalFormatToVkFormat(internalformat); }
inline XFormat                  GlInternalFormatToXFormat(GLenum internalformatDepth, GLenum internalformatStencil) { return vulkanAPI::GlInternalFormatToVkFormat(internalformatDepth, internalformatStencil); }
inline XFormat                  GlAttribPointerToXFormat(GLint nElements, GLenum type, GLboolean normalized) { return vulkanAPI::GlAttribPointerToVkFormat(nElements, type, normalized); }
inline XIndexType               GlToXIndexType(GLenum type) { return vulkanAPI::GlToVkIndexType(type); }
inline XFormat                  GlColorFormatToXColorFormat(GLenum format, GLenum type) { return vulkanAPI::GlColorFormatToVkColorFormat(format, type); }

#endif // __VKUTILS_H__

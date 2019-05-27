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
 *  @file       stateManager.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      State Manager Functionality
 *
 *  @section
 *
 *  OpenGL ES maintains a considerable amount of information that can be
 *  divided into state parameteres for (a) Active Objects, (b) Framebuffer
 *  Operations, (c) Fragment Operations, (d) Input Assembly, (e) Pixel Storage,
 *  (f) Rasterization, (g) Viewport Transformation and (h) implementation-specific
 *  hints.
 *
 */

#include "stateManager.h"

StateManager::StateManager()
: mError(GL_NO_ERROR)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

StateManager::~StateManager()
{
    FUN_ENTRY(GL_LOG_TRACE);
}

GLboolean
StateManager::IsCapabilityEnabled(GLenum cap)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLboolean res = GL_FALSE;

    switch(cap) {
    case GL_BLEND:                    res = mFragmentOperationsState.GetBlendingEnabled();              break;
    case GL_CULL_FACE:                res = mRasterizationState.GetCullEnabled();                       break;
    case GL_DEPTH_TEST:               res = mFragmentOperationsState.GetDepthTestEnabled();             break;
    case GL_DITHER:                   res = mFragmentOperationsState.GetDitheringEnabled();             break;
    case GL_POLYGON_OFFSET_FILL:      res = mRasterizationState.GetPolygonOffsetFillEnabled();          break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE: res = mFragmentOperationsState.GetSampleAlphaToCoverageEnabled(); break;
    case GL_SAMPLE_COVERAGE:          res = mFragmentOperationsState.GetSampleCoverageEnabled();        break;
    case GL_SCISSOR_TEST:             res = mFragmentOperationsState.GetScissorTestEnabled();           break;
    case GL_STENCIL_TEST:             res = mFragmentOperationsState.GetStencilTestEnabled();           break;
    }

    return res;
}

void
StateManager::InitVkPipelineStates(vulkanAPI::Pipeline *pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);

    VkBool32            primitiveRestartEnable = GetInputAssemblyState()->GetPrimitiveRestartEnabled();
    VkPrimitiveTopology topology               = GlPrimitiveTopologyToXPrimitiveTopology(GetInputAssemblyState()->GetPrimitiveMode());

    VkPolygonMode      polygonMode             = GLPrimitiveModeToXPolygonMode(GetInputAssemblyState()->GetPrimitiveMode());
    VkCullModeFlagBits cullMode                = GlCullModeToXCullMode(GetRasterizationState()->GetCullFace());
    VkFrontFace        frontFace               = GlFrontFaceToXFrontFace(GetRasterizationState()->GetFrontFace());
    VkBool32           depthClampEnable        = GetRasterizationState()->GetDepthClampEnabled();
    VkBool32           rasterizerDiscardEnable = GetRasterizationState()->GetRasterizerDiscardEnabled();
    VkBool32           depthBiasEnable         = GetRasterizationState()->GetPolygonOffsetFillEnabled();
    float              depthBiasConstantFactor = GetRasterizationState()->GetPolygonOffsetFactor();
    float              depthBiasSlopeFactor    = GetRasterizationState()->GetPolygonOffsetUnits();
    float              depthBiasClamp          = GetRasterizationState()->GetPolygonOffsetClamp();
    VkBool32           blendEnable             = GetFragmentOperationsState()->GetBlendingEnabled();
    VkColorComponentFlags colorWriteMask       = GLColorMaskToXColorComponentFlags(GetFramebufferOperationsState()->GetColorMask());

    VkBlendFactor srcColorBlendFactor = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorSourceRGB());
    VkBlendFactor dstColorBlendFactor = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorDestinationRGB());
    VkBlendFactor srcAlphaBlendFactor = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorSourceAlpha());
    VkBlendFactor dstAlphaBlendFactor = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorDestinationAlpha());

    VkBlendOp   colorBlendOp         = GlBlendEquationToXBlendOp(GetFragmentOperationsState()->GetBlendingEquationRGB());
    VkBlendOp   alphaBlendOp         = GlBlendEquationToXBlendOp(GetFragmentOperationsState()->GetBlendingEquationAlpha());
    VkLogicOp   logicOp              = GlLogicOpToXLogicOp(GetFragmentOperationsState()->GetBlendingLogicOp());
    VkBool32    logicOpEnable        = GetFragmentOperationsState()->GetBlendingLogicOpEnabled();
    uint32_t    colorAttachmentCount = GetFragmentOperationsState()->GetBlendingColorAttachmentCount();

    GLfloat     blendcolor[4];
    GetFragmentOperationsState()->GetBlendingColor(blendcolor);

    uint32_t    viewportCount = GetViewportTransformationState()->GetViewportCount();
    uint32_t    scissorCount  = GetViewportTransformationState()->GetScissorCount();

    VkBool32              alphaToOneEnable      = GetFragmentOperationsState()->GetSampleAlphaToOneEnabled();
    VkBool32              alphaToCoverageEnable = GetFragmentOperationsState()->GetSampleAlphaToCoverageEnabled();
    VkSampleCountFlagBits rasterizationSamples  = GlSampleCoverageBitsToXSampleCountFlagBits(GetFragmentOperationsState()->GetSampleCoverageBits());
    VkBool32              sampleShadingEnable   = GetFragmentOperationsState()->GetSampleShadingEnabled();
    float                 minSampleShading      = GetFragmentOperationsState()->GetMinSampleShading();

    VkBool32    depthTestEnable       = GetFragmentOperationsState()->GetDepthTestEnabled();
    VkBool32    depthWriteEnable      = GetFramebufferOperationsState()->GetDepthMask();
    VkCompareOp depthCompareOp        = GlCompareFuncToXCompareOp(GetFragmentOperationsState()->GetDepthTestFunc());
    VkBool32    depthBoundsTestEnable = GetFragmentOperationsState()->GetDepthBoundsTestEnabled();
    float       minDepthBounds        = GetFragmentOperationsState()->GetMinDepthBounds();
    float       maxDepthBounds        = GetFragmentOperationsState()->GetMaxDepthBounds();

    VkBool32    stencilTestEnable     = GetFragmentOperationsState()->GetStencilTestEnabled();
    VkStencilOp backfailOp            = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpFailBack());
    VkStencilOp backpassOp            = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZpassBack());
    VkStencilOp backdepthFailOp       = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZfailBack());
    uint32_t    backwriteMask         = GetFramebufferOperationsState()->GetStencilMaskBack();
    VkCompareOp backcompareOp         = GlCompareFuncToXCompareOp(GetFragmentOperationsState()->GetStencilTestFuncCompareBack());
    uint32_t    backcompareMask       = GetFragmentOperationsState()->GetStencilTestFuncMaskBack();
    uint32_t    backreference         = GetFragmentOperationsState()->GetStencilTestFuncRefBack();
    VkStencilOp frontfailOp           = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpFailFront());
    VkStencilOp frontpassOp           = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZpassFront());
    VkStencilOp frontdepthFailOp      = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZfailFront());
    uint32_t    frontwriteMask        = GetFramebufferOperationsState()->GetStencilMaskFront();
    VkCompareOp frontcompareOp        = GlCompareFuncToXCompareOp(GetFragmentOperationsState()->GetStencilTestFuncCompareFront());
    uint32_t    frontcompareMask      = GetFragmentOperationsState()->GetStencilTestFuncMaskFront();
    uint32_t    frontreference        = GetFragmentOperationsState()->GetStencilTestFuncRefFront();

    pipeline->CreateInputAssemblyState(primitiveRestartEnable, topology);
    pipeline->CreateRasterizationState(polygonMode, cullMode, frontFace, depthBiasEnable, depthBiasConstantFactor, depthBiasSlopeFactor, depthBiasClamp, depthClampEnable, rasterizerDiscardEnable );
    pipeline->CreateColorBlendState(blendEnable, colorWriteMask, srcColorBlendFactor, dstColorBlendFactor, srcAlphaBlendFactor, dstAlphaBlendFactor, colorBlendOp, alphaBlendOp, logicOp, logicOpEnable, colorAttachmentCount, blendcolor);
    pipeline->CreateDepthStencilState(depthTestEnable, depthWriteEnable, depthCompareOp, depthBoundsTestEnable, minDepthBounds, maxDepthBounds, stencilTestEnable, backfailOp, backpassOp,
                            backdepthFailOp, backwriteMask, backcompareOp, backcompareMask, backreference, frontfailOp, frontpassOp, frontdepthFailOp, frontwriteMask, frontcompareOp, frontcompareMask, frontreference );
    pipeline->CreateViewportState(viewportCount, scissorCount);
    pipeline->CreateMultisampleState(alphaToOneEnable, alphaToCoverageEnable, rasterizationSamples, sampleShadingEnable, minSampleShading);
    std::vector<VkDynamicState> states = {VK_DYNAMIC_STATE_VIEWPORT,
                                          VK_DYNAMIC_STATE_SCISSOR,
                                          VK_DYNAMIC_STATE_LINE_WIDTH};
    pipeline->CreateDynamicState(states);
    pipeline->CreateInfo();
}

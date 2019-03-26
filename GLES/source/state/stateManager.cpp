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
StateManager::InitPipelineStates(vulkanAPI::Pipeline *pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);

    GLboolean               primitiveRestartEnable  = GetInputAssemblyState()->GetPrimitiveRestartEnabled();
    XPrimitiveTopology      topology                = GlPrimitiveTopologyToXPrimitiveTopology(GetInputAssemblyState()->GetPrimitiveMode());

    XPolygonMode            polygonMode             = GLPrimitiveModeToXPolygonMode(GetInputAssemblyState()->GetPrimitiveMode());
    XCullModeFlagBits       cullMode                = GlCullModeToXCullMode(GetRasterizationState()->GetCullFace());
    XFrontFace              frontFace               = GlFrontFaceToXFrontFace(GetRasterizationState()->GetFrontFace());
    GLboolean               depthClampEnable        = GetRasterizationState()->GetDepthClampEnabled();
    GLboolean               rasterizerDiscardEnable = GetRasterizationState()->GetRasterizerDiscardEnabled();
    GLboolean               depthBiasEnable         = GetRasterizationState()->GetPolygonOffsetFillEnabled();
    float                   depthBiasConstantFactor = GetRasterizationState()->GetPolygonOffsetFactor();
    float                   depthBiasSlopeFactor    = GetRasterizationState()->GetPolygonOffsetUnits();
    float                   depthBiasClamp          = GetRasterizationState()->GetPolygonOffsetClamp();
    GLboolean               blendEnable             = GetFragmentOperationsState()->GetBlendingEnabled();
    XColorComponentFlags    colorWriteMask          = GLColorMaskToXColorComponentFlags(GetFramebufferOperationsState()->GetColorMask());

    XBlendFactor            srcColorBlendFactor     = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorSourceRGB());
    XBlendFactor            dstColorBlendFactor     = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorDestinationRGB());
    XBlendFactor            srcAlphaBlendFactor     = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorSourceAlpha());
    XBlendFactor            dstAlphaBlendFactor     = GlBlendFactorToXBlendFactor(GetFragmentOperationsState()->GetBlendingFactorDestinationAlpha());

    XBlendOp                colorBlendOp            = GlBlendEquationToXBlendOp(GetFragmentOperationsState()->GetBlendingEquationRGB());
    XBlendOp                alphaBlendOp            = GlBlendEquationToXBlendOp(GetFragmentOperationsState()->GetBlendingEquationAlpha());
    XLogicOp                logicOp                 = GlLogicOpToXLogicOp(GetFragmentOperationsState()->GetBlendingLogicOp());
    GLboolean               logicOpEnable           = GetFragmentOperationsState()->GetBlendingLogicOpEnabled();
    uint32_t                colorAttachmentCount    = GetFragmentOperationsState()->GetBlendingColorAttachmentCount();

    GLfloat                 blendcolor[4];            GetFragmentOperationsState()->GetBlendingColor(blendcolor);

    uint32_t                viewportCount           = GetViewportTransformationState()->GetViewportCount();
    uint32_t                scissorCount            = GetViewportTransformationState()->GetScissorCount();

    GLboolean               alphaToOneEnable        = GetFragmentOperationsState()->GetSampleAlphaToOneEnabled();
    GLboolean               alphaToCoverageEnable   = GetFragmentOperationsState()->GetSampleAlphaToCoverageEnabled();
    XSampleCountFlagBits    rasterizationSamples    = GlSampleCoverageBitsToXSampleCountFlagBits(GetFragmentOperationsState()->GetSampleCoverageBits());
    GLboolean               sampleShadingEnable     = GetFragmentOperationsState()->GetSampleShadingEnabled();
    float                   minSampleShading        = GetFragmentOperationsState()->GetMinSampleShading();

    GLboolean               depthTestEnable         = GetFragmentOperationsState()->GetDepthTestEnabled();
    GLboolean               depthWriteEnable        = GetFramebufferOperationsState()->GetDepthMask();
    XCompareOp              depthCompareOp          = GlCompareFuncToXCompareOp(GetFragmentOperationsState()->GetDepthTestFunc());
    GLboolean               depthBoundsTestEnable   = GetFragmentOperationsState()->GetDepthBoundsTestEnabled();
    float                   minDepthBounds          = GetFragmentOperationsState()->GetMinDepthBounds();
    float                   maxDepthBounds          = GetFragmentOperationsState()->GetMaxDepthBounds();

    GLboolean               stencilTestEnable       = GetFragmentOperationsState()->GetStencilTestEnabled();
    XStencilOp              backfailOp              = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpFailBack());
    XStencilOp              backpassOp              = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZpassBack());
    XStencilOp              backdepthFailOp         = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZfailBack());
    uint32_t                backwriteMask           = GetFramebufferOperationsState()->GetStencilMaskBack();
    XCompareOp              backcompareOp           = GlCompareFuncToXCompareOp(GetFragmentOperationsState()->GetStencilTestFuncCompareBack());
    uint32_t                backcompareMask         = GetFragmentOperationsState()->GetStencilTestFuncMaskBack();
    uint32_t                backreference           = GetFragmentOperationsState()->GetStencilTestFuncRefBack();
    XStencilOp              frontfailOp             = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpFailFront());
    XStencilOp              frontpassOp             = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZpassFront());
    XStencilOp              frontdepthFailOp        = GlStencilFuncToXStencilOp(GetFragmentOperationsState()->GetStencilTestOpZfailFront());
    uint32_t                frontwriteMask          = GetFramebufferOperationsState()->GetStencilMaskFront();
    XCompareOp              frontcompareOp          = GlCompareFuncToXCompareOp(GetFragmentOperationsState()->GetStencilTestFuncCompareFront());
    uint32_t                frontcompareMask        = GetFragmentOperationsState()->GetStencilTestFuncMaskFront();
    uint32_t                frontreference          = GetFragmentOperationsState()->GetStencilTestFuncRefFront();

    pipeline->CreateInputAssemblyState(primitiveRestartEnable, topology);
    pipeline->CreateRasterizationState(polygonMode, cullMode, frontFace, depthBiasEnable, depthBiasConstantFactor, depthBiasSlopeFactor, depthBiasClamp, depthClampEnable, rasterizerDiscardEnable );
    pipeline->CreateColorBlendState(blendEnable, colorWriteMask, srcColorBlendFactor, dstColorBlendFactor, srcAlphaBlendFactor, dstAlphaBlendFactor, colorBlendOp, alphaBlendOp, logicOp, logicOpEnable, colorAttachmentCount, blendcolor);
    pipeline->CreateDepthStencilState(depthTestEnable, depthWriteEnable, depthCompareOp, depthBoundsTestEnable, minDepthBounds, maxDepthBounds, stencilTestEnable, backfailOp, backpassOp,
                            backdepthFailOp, backwriteMask, backcompareOp, backcompareMask, backreference, frontfailOp, frontpassOp, frontdepthFailOp, frontwriteMask, frontcompareOp, frontcompareMask, frontreference );
    pipeline->CreateViewportState(viewportCount, scissorCount);
    pipeline->CreateMultisampleState(alphaToOneEnable, alphaToCoverageEnable, rasterizationSamples, sampleShadingEnable, minSampleShading);
    pipeline->CreateCommonDynamicState(true);
    pipeline->CreateInfo();
}

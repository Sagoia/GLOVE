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
 *  @file       screenSpacePass.cpp
 *  @author     Think Silicon
 *  @date       26/10/2018
 *  @version    1.0
 *
 *  @brief      Screen Space Vulkan Pass used for various operations (e.g., clear with ColorMask)
 *
 */

#include "screenSpacePass.h"
#include "utils/VkToGlConverter.h"
#include "shaderProgram.h"
#include "glslang/glslangShaderCompiler.h"
#include "context/context.h"

struct ScreenSpaceVertex {
    float pos[2];
};

struct UniformBufferObject_ScreenSpace {
    float color[4];
};

ScreenSpacePass::ScreenSpacePass(const vulkanAPI::vkContext_t *vkContext):
    mVkContext(vkContext), mCacheManager(nullptr),
    mNumElements(0), mVertexBuffer(nullptr),
    mVertexVkBuffer(VK_NULL_HANDLE), mVertexVkBufferOffset(0),
    mVertexInputInfo(), mPipelineCache(new vulkanAPI::PipelineCache(mVkContext)),
    mPipeline(new vulkanAPI::Pipeline(vkContext)),
    mInitialized(false), mValid(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

}

ScreenSpacePass::~ScreenSpacePass()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Destroy();
}

bool
ScreenSpacePass::CreateMeshData()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // the rate to load data from memory throughout the vertices
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ScreenSpaceVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    mVkBindingDescriptions.push_back(bindingDescription);

    // vertex attribute information
    const int num_attributes = 1;
    std::array<VkVertexInputAttributeDescription, num_attributes> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(ScreenSpaceVertex, pos);
    mVkAttributeDescriptions.push_back(attributeDescriptions[0]);

    float pos = 1.0f;
    std::vector<ScreenSpaceVertex> vertices =  {
     // bottom left
     {{-pos, -pos}},
     // bottom right
     {{pos,  -pos}},
     // top right
      {{pos,  pos}},
     // top right
      {{pos,  pos}},
     // top left
      {{-pos, pos}},
     // bottom left
      {{-pos, -pos}}
     };

    mNumElements = static_cast<uint32_t>(vertices.size());

    mVertexBuffer = new VertexBufferObject(mVkContext);
    if(!mVertexBuffer->Allocate(mNumElements * sizeof(ScreenSpaceVertex), vertices.data())) {
        return false;
    }

    mVertexVkBuffer = mVertexBuffer->GetVkBuffer();
    mVertexVkBufferOffset = 0;

    return true;
}

bool
ScreenSpacePass::DestroyMeshData()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVertexBuffer) {
        mVertexBuffer->Release();
    }
    delete mVertexBuffer;
    mVertexBuffer = nullptr;
    mVertexVkBuffer = VK_NULL_HANDLE;
    mVertexVkBufferOffset = 0;
    return true;
}

void
ScreenSpacePass::ShaderData::Destroy()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(shaderCompiler) {
        delete shaderCompiler;
        shaderCompiler = nullptr;
    }
    if(vertShader) {
        delete vertShader;
        vertShader = nullptr;
    }
    if(fragShader) {
        delete fragShader;
        fragShader = nullptr;
    }
    if(shaderProgram) {
        delete shaderProgram;
        shaderProgram = nullptr;
    }
}

void
ScreenSpacePass::ShaderData::InitResources(CacheManager* cacheManager, const vulkanAPI::vkContext_t* mVkContext)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Destroy();
    shaderCompiler = new GlslangShaderCompiler();
    vertShader = new Shader();
    vertShader->SetShaderCompiler(shaderCompiler);
    vertShader->SetVkContext(mVkContext);
    fragShader = new Shader();
    fragShader->SetShaderCompiler(shaderCompiler);
    fragShader->SetVkContext(mVkContext);
    shaderProgram = new ShaderProgram(mVkContext);
    shaderProgram->SetShaderCompiler(shaderCompiler);
    shaderProgram->SetCacheManager(cacheManager);
}

bool
ScreenSpacePass::ShaderData::Generate(const std::string& vertexSource, const std::string& fragmentSource)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const char *vertexSourcec100Str = vertexSource.c_str();
    const char *fragmentSourcec100Str = fragmentSource.c_str();

    vertShader->SetShaderType(SHADER_TYPE_VERTEX);
    GLint vertexLength = static_cast<GLint>(vertexSource.length());
    vertShader->SetShaderSource(1, &vertexSourcec100Str, &vertexLength);
    if(!vertShader->CompileShader()) {
        GLOVE_PRINT_ERR("Could not compile vertex shader for screen-space pass\n");
        return false;
    }

    fragShader->SetShaderType(SHADER_TYPE_FRAGMENT);
    GLint fragmentLength = static_cast<GLint>(fragmentSource.length());
    fragShader->SetShaderSource(1, &fragmentSourcec100Str, &fragmentLength);
    if(!fragShader->CompileShader()) {
        GLOVE_PRINT_ERR("Could not compile fragment shader for screen-space pass\n");
        return false;
    }

    shaderProgram->AttachShader(vertShader);
    shaderProgram->AttachShader(fragShader);
    if(!shaderProgram->LinkProgram()) {
        GLOVE_PRINT_ERR("Could not link shader program for screen-space pass\n");
        return false;
    }
    shaderProgram->SetShaderModules();
    return true;
}

bool
ScreenSpacePass::CreateShaderData()
{
    FUN_ENTRY(GL_LOG_DEBUG);

const std::string vertexSource100 = "#version 100\n\
#ifdef GL_ES\n\
    precision mediump float;\n\
#endif\n\
attribute vec2 v_position;\n\
void main() {\n\
    gl_Position = vec4(v_position, 1.0, 1.0);\n\
}\n\
";

const std::string fragmentSource100 = "#version 100\n\
#ifdef GL_ES\n\
    precision mediump float;\n\
#endif\n\
uniform  vec4    clearColor;\n\
void main() {\n\
    gl_FragColor = clearColor;\n\
}\n\
";

    mShaderData.InitResources(mCacheManager, mVkContext);
    mShaderData.Generate(vertexSource100, fragmentSource100);

    if(!mShaderData.shaderProgram->SetPipelineShaderStage(mPipeline->GetShaderStageCountRef(), mPipeline->GetShaderStageIDsRef(), mPipeline->GetShaderStages())) {
        return false;
    }
    mPipeline->SetCache(mShaderData.shaderProgram->GetVkPipelineCache());
    mPipeline->SetLayout(mShaderData.shaderProgram->GetVkPipelineLayout());

    return true;
}

bool
ScreenSpacePass::DestroyShaderData()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mShaderData.Destroy();

    return true;
}

bool
ScreenSpacePass::Initialize()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mInitialized) {
        return true;
    }

    mInitialized = true;

    if(!CreateShaderData()) {
        return false;
    }

    if(!CreateMeshData()) {
        return false;
    }

    if(!CreateDefaultPipelineStates()) {
       return false;
    }

    mValid = true;
    return mValid;
}

bool
ScreenSpacePass::CreateDefaultPipelineStates()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // the format of the vertex data that will be passed to the vertex shader
    mVertexInputInfo = {};
    mVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    mVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(mVkBindingDescriptions.size());
    mVertexInputInfo.pVertexBindingDescriptions = mVkBindingDescriptions.data();
    mVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(mVkAttributeDescriptions.size());
    mVertexInputInfo.pVertexAttributeDescriptions = mVkAttributeDescriptions.data();

    mPipeline->SetVertexInputState(&mVertexInputInfo);

    std::vector<VkDynamicState> states = {VK_DYNAMIC_STATE_VIEWPORT,
                                          VK_DYNAMIC_STATE_SCISSOR};
    mPipeline->CreateDynamicState(states);

    mPipeline->SetDepthTestEnable(false);

    return true;
}

bool
ScreenSpacePass::Destroy()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    DestroyShaderData();
    DestroyMeshData();
    if(mPipeline != nullptr) {
       delete mPipeline;
       mPipeline = nullptr;
    }
    if(mPipelineCache) {
       delete mPipelineCache;
       mPipelineCache = nullptr;
    }
    return true;
}

bool
ScreenSpacePass::UpdateUniformBufferColor(float r, float g, float b, float a)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    UniformBufferObject_ScreenSpace ubo = { r,g,b,a };
    GLint loc = mShaderData.shaderProgram->GetUniformLocation("clearColor");
    if(loc >= 0) {
        mShaderData.shaderProgram->SetUniformData(static_cast<uint32_t>(loc),
                                                  sizeof(UniformBufferObject_ScreenSpace),
                                                  static_cast<void *>(&ubo.color));
    }

    return true;
}

void
ScreenSpacePass::BindPipeline(const VkCommandBuffer *cmdBuffer) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mPipeline->Bind(cmdBuffer);
}

void
ScreenSpacePass::BindVertexBuffers(const VkCommandBuffer *cmdBuffer) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // bind vertex buffers
    vkCmdBindVertexBuffers(*cmdBuffer, 0, 1, &mVertexVkBuffer, &mVertexVkBufferOffset);
}

void
ScreenSpacePass::BindUniformDescriptors(const VkCommandBuffer *cmdBuffer) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mShaderData.shaderProgram->UpdateDescriptorSet();
    mShaderData.shaderProgram->UpdateBuiltInUniformData(0.0f, 1.0f);
    vkCmdBindDescriptorSets(*cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mShaderData.shaderProgram->GetVkPipelineLayout(), 0, 1,
                            mShaderData.shaderProgram->GetVkDescSet(), 0, nullptr);
}

void
ScreenSpacePass::Draw(const VkCommandBuffer *cmdBuffer) const
{
    FUN_ENTRY(GL_LOG_DEBUG);


    vkCmdDraw(*cmdBuffer, mNumElements, 1, 0, 0);
}

void
ScreenSpacePass::SetCacheManager(CacheManager* cacheManager)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mCacheManager = cacheManager;
    mPipeline->SetCacheManager(cacheManager);
}

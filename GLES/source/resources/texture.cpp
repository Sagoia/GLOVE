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
 *  @file       texture.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Texture Functionality in GLOVE
 *
 *  @scope
 *
 *  A Texture contains one or more images that all have the same image format.
 *  A texture can be used in two ways: (a) it can be the source of a texture
 *  access from a Shader or (b) it can be used as an attachment.
 *
 */

#include "texture.h"
#include "utils/glUtils.h"
#include "utils/cacheManager.h"

#define NUMBER_OF_MIP_LEVELS(w, h)                      (std::floor(std::log2(std::max((w),(h)))) + 1)

// TODO:: this needs to be further discussed
int Texture::mDefaultInternalAlignment = 1;

Texture::Texture(const vulkanAPI::XContext_t *xContext, vulkanAPI::CommandBufferManager *cbManager, const VkFlags vkFlags)
: mXContext(xContext), mCommandBufferManager(cbManager), mCacheManager(nullptr),
mFormat(GL_INVALID_VALUE), mTarget(GL_INVALID_VALUE), mType(GL_INVALID_VALUE), mInternalFormat(GL_INVALID_VALUE),
mExplicitType(GL_INVALID_VALUE), mExplicitInternalFormat(GL_INVALID_VALUE),
mMipLevelsCount(1), mLayersCount(1), mState(nullptr), mDataUpdated(false), mDataNoInvertion(false), mFboColorAttached(false), mIsNPOT(false), mIsNPOTAccessCompleted(false),
mDepthStencilTexture(nullptr), mDepthStencilTextureRefCount(0u), mDirty(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mImage     = new vulkanAPI::Image(xContext);
    mImageView = new vulkanAPI::ImageView(xContext);
    mMemory    = new vulkanAPI::Memory(xContext, vkFlags);
    mSampler   = new vulkanAPI::Sampler(xContext);
}

Texture::~Texture()
{
    FUN_ENTRY(GL_LOG_TRACE);

    delete mSampler;
    delete mImageView;
    delete mImage;
    delete mMemory;

    if(mState != nullptr) {
        for (GLint layer = 0; layer < mLayersCount; ++layer) {
            for (GLint level = 0; level < (GLint)mState[layer].Size(); ++level) {
                delete mState[layer][level];
            }
        }
        delete [] mState;
        mState = nullptr;
    }
}

void
Texture::UpdateNPOTAccessCompleted(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mIsNPOTAccessCompleted = !(IsNPOT() && ((GetMinFilter() != GL_LINEAR         && GetMinFilter() != GL_NEAREST)    ||
                                            (GetWrapS()     != GL_CLAMP_TO_EDGE  || GetWrapT()     != GL_CLAMP_TO_EDGE)));
}

bool
Texture::IsCompleted(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // A two-dimensional texture is complete if the following conditions all hold true:
    // 1) The set of mipmap arrays are specified with the same format, internal format, and type.
    // 2) The dimensions of the arrays follow the correct sequence
    // 3) Each dimension of the level zero array is positive.

    // A texture is cube complete if the following conditions all hold true:
    // 1) The level zero arrays of each of the six texture images making up the cube map have identical, positive, and square dimensions.
    // 2) The level zero arrays were each specified with the same format, internal format, and type.

    if(mState == nullptr) {
        return false;
    }

    if (!mDirty) {
        return true;
    }

    State_t *state = mState[0][0];
    if(state->format == GL_INVALID_VALUE || state->width <= 0 || state->height <= 0) {
        return false;
    }

    GLenum format = state->format;
    GLenum type   = state->type;
    GLint  width  = state->width;
    GLint  height = state->height;
    GLint  levels = (GLint)NUMBER_OF_MIP_LEVELS(state->width, state->height);

    GLint count = 0;
    for(GLint layer = 0; layer < mLayersCount; ++layer) {
        count = 0;

        for(GLint level = 0; level < levels; ++level) {

            state = mState[layer][level];

            if (!GlInternalFormatIsCompressed(state->format) &&
                (state->format == GL_INVALID_VALUE || state->type == GL_INVALID_VALUE)) {
                ++count;
            } else if (state->width != static_cast<GLint>(std::max(floor(width >> level), 1.0)) ||
                state->height != static_cast<GLint>(std::max(floor(height >> level), 1.0))) {
                ++count;
            } else if(state->format != format || state->type != type) {
                return false;
            }
        }

        if(count > 0 && count < levels - 1)
            return false;
    }

    mMipLevelsCount = levels - count;

    mDirty = false;

    return true;
}

bool
Texture::IsValid(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (mState == nullptr) {
        return false;
    }

    State_t *state = mState[0][0];
    if (state->format == GL_INVALID_VALUE || state->width <= 0 || state->height <= 0) {
        return false;
    }

    GLenum format = state->format;
    GLenum type = state->type;
    GLint  width = state->width;
    GLint  height = state->height;
    GLint  levels = (GLint)NUMBER_OF_MIP_LEVELS(state->width, state->height);

    GLint levelsCount = INT32_MAX;
    GLint count = 0;
    for (GLint layer = 0; layer < mLayersCount; ++layer) {
        count = 0;

        for (GLint level = 0; level < levels; ++level) {

            state = mState[layer][level];

            if (!GlInternalFormatIsCompressed(state->format) &&
                (state->format == GL_INVALID_VALUE || state->type == GL_INVALID_VALUE)) {
                break;
            } else if (state->width != static_cast<GLint>(std::max(floor(width >> level), 1.0)) ||
                       state->height != static_cast<GLint>(std::max(floor(height >> level), 1.0))) {
                break;
            } else if (state->format != format || state->type != type) {
                break;
            }

            ++count;
        }

        levelsCount = std::min(levelsCount, count);

    }

    mMipLevelsCount = levelsCount;

    mDirty = false;

    return mMipLevelsCount > 0;
}

void
Texture::ReleaseVkResources(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mSampler->Release();
    mImageView->Release();
    mImage->Release();
    mMemory->Release();
}

bool
Texture::CreateVkImage(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mImage->SetWidth(GetWidth());
    mImage->SetHeight(GetHeight());
    mImage->SetMipLevels(mMipLevelsCount);
    mImage->SetImageLayout(VK_IMAGE_LAYOUT_UNDEFINED);

    mSampler->SetMaxLod((mParameters.GetMinFilter() == GL_NEAREST || mParameters.GetMinFilter() == GL_LINEAR) ? 0.25f : static_cast<float>(mMipLevelsCount-1));
    return mImage->Create();
}

bool
Texture::AllocateVkMemory(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mMemory->GetImageMemoryRequirements(mImage->GetImage());

    return mMemory->Create() && mMemory->BindImageMemory(mImage->GetImage());
}

bool
Texture::CreateVkTexture(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    ReleaseVkResources();

    if(!CreateVkImage()) {
        return false;
    }

    if(!AllocateVkMemory()) {
        mImage->Release();
        return false;
    }

    if(!CreateVkImageView()) {
        mImage->Release();
        mMemory->Release();
        return false;
    }

    PrepareVkImageLayout(VK_IMAGE_LAYOUT_GENERAL);

    return true;
}

bool
Texture::Allocate(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    State_t *state = mState[0][0];

    SetWidth (state->width);
    SetHeight(state->height);
    SetFormat(state->format);
    SetType  (state->type);
    SetInternalFormat(GlFormatToGlInternalFormat(state->format, state->type));

    if (mImage->GetFormat() == VK_FORMAT_UNDEFINED) {
        SetVkFormat(FindSupportedVkColorFormat(GlColorFormatToXColorFormat(state->format, state->type)));
    }
    mExplicitInternalFormat = XFormatToGlInternalformat(mImage->GetFormat());
    mExplicitType           = GlInternalFormatToGlType(mExplicitInternalFormat);

    if(!CreateVkTexture()) {
        return false;
    }

    bool isCompressed = GlInternalFormatIsCompressed(mExplicitInternalFormat);

    // NOTE:: there is an implicit conversion of all textures to GL_RGBA
    // TODO:: this should definitely NOT be the case
    GLenum srcInternalFormat = mInternalFormat;
    GLenum dstInternalFormat = mExplicitInternalFormat;
    GLenum dstType = mExplicitType;
    for(GLint layer = 0; layer < mLayersCount; ++layer) {
        for(GLint level = 0; level < mMipLevelsCount; ++level) {
            state = mState[layer][level];
            if(!state->data) {
                continue;
            }
            if (isCompressed) {
                Rect srcRect(0, 0, state->width, state->height);
                CpoyCompressedPixelFromHost(&srcRect, level, layer, srcInternalFormat, state->data, state->size);
            } else {
                ImageRect srcRect(0, 0, state->width, state->height,
                                  (int)(GlInternalFormatTypeToNumElements(srcInternalFormat, state->type)),
                                  (int)(GlTypeToElementSize(state->type)),
                                  Texture::GetDefaultInternalAlignment());
                ImageRect dstRect(0, 0, state->width, state->height,
                                  (int)(GlInternalFormatTypeToNumElements(dstInternalFormat, dstType)),
                                  (int)(GlTypeToElementSize(dstType)),
                                  Texture::GetDefaultInternalAlignment());
                CopyPixelsFromHost(&srcRect, &dstRect, level, layer, srcInternalFormat, state->data);
            }
        }
    }

    return true;
}

void
Texture::SetState(GLsizei width, GLsizei height, GLint level, GLint layer, GLenum format, GLenum type, GLint unpackAlignment, const void *pixels)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (level == 0) {
        uint32_t mipLevel = (uint32_t)NUMBER_OF_MIP_LEVELS(width, height);
        if (mState[layer].Capacity() < mipLevel) {
            mState[layer].Reserve(mipLevel);
        }
        uint32_t count = mipLevel - mState[layer].Size();
        if (count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                mState[layer].PushBack(new State_t());
            }
        }
    }

    mState[layer][level]->width  = width;
    mState[layer][level]->height = height;
    mState[layer][level]->format = format;
    mState[layer][level]->type   = type;

    if (layer == 0 && level == 0) {
        mIsNPOT = format == GL_INVALID_VALUE ? false : (!ISPOWEROFTWO(width) || !ISPOWEROFTWO(height));
        UpdateNPOTAccessCompleted();
    }

    if(mState[layer][level]->data) {
        delete [] (uint8_t *)mState[layer][level]->data;
        mState[layer][level]->data = nullptr;
    }

    if(pixels) {
        // convert the pixel buffers to the internal alignment
        // so that they can cooperate with any subsequent subimage calls
        // even if a texture is not complete
        GLenum srcInternalFormat = GlFormatToGlInternalFormat(format, type);
        ImageRect srcRect(0, 0, width, height,
                          (int)(GlInternalFormatTypeToNumElements(srcInternalFormat, type)),
                          (int)(GlTypeToElementSize(type)),
                          unpackAlignment);
        ImageRect dstRect(0, 0, width, height,
                          (int)(GlInternalFormatTypeToNumElements(srcInternalFormat, type)),
                          (int)(GlTypeToElementSize(type)),
                          Texture::GetDefaultInternalAlignment());
        size_t size       = dstRect.GetRectBufferSize();
        mState[layer][level]->data = new uint8_t[size];
        void *data = mState[layer][level]->data;
        ConvertPixels(srcInternalFormat, srcInternalFormat,
                      &srcRect, pixels,
                      &dstRect, data);
    }

    mDirty = true;
}

void
Texture::SetSubState(ImageRect *srcRect, ImageRect *dstRect, GLint level, GLint layer, GLenum srcFormat, const void *srcData)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mState[layer][level]->data == nullptr) {
        ImageRect srcRect(0, 0, mState[layer][level]->width, mState[layer][level]->height,
                          (int)(GlInternalFormatTypeToNumElements(GetInternalFormat(), GetType())),
                          (int)(GlTypeToElementSize(GetType())),
                          Texture::GetDefaultInternalAlignment());
        unsigned int size = srcRect.GetRectBufferSize();
        mState[layer][level]->data = new uint8_t[size];
    }

    if(srcData) {
        const GLenum dstFormat = mInternalFormat;

        // create a buffer at the size of the requested subrectangle
        const size_t dstSize = dstRect->GetRectBufferSize();
        uint8_t *dstData = new uint8_t[dstSize];

        // convert the source buffer to the internal format and alignment
        // both buffers here are in the subtexture dimensions but may differ
        // in format and alignment
        ImageRect tmp_srcRect = *srcRect;
        ImageRect tmp_dstRect = *dstRect;
        tmp_srcRect.x = 0; tmp_srcRect.y = 0;
        tmp_dstRect.x = 0; tmp_dstRect.y = 0;
        ConvertPixels(srcFormat, dstFormat,
                      &tmp_srcRect, srcData,
                      &tmp_dstRect, dstData);

        if(mFboColorAttached) {
            InvertImageYAxis(static_cast<uint8_t *>(dstData), &tmp_dstRect);
        }
        mFboColorAttached = false;

        // copy the converted buffer (containing the subtexture) to the target texture
        // both buffers are now in the same format and alignment
        tmp_srcRect = *srcRect;
        tmp_dstRect = *dstRect;
        tmp_srcRect.mAlignment = Texture::GetDefaultInternalAlignment();
        tmp_dstRect.width = mState[layer][level]->width;
        tmp_dstRect.height = mState[layer][level]->height;

        CopyPixelsNoConversion(&tmp_srcRect, dstData,
                              &tmp_dstRect, mState[layer][level]->data);
        delete[] dstData;
    }

    SetDataUpdated(true);
}

void
Texture::SetCompressedState(GLsizei width, GLsizei height, GLint level, GLint layer, GLenum internalformat, GLsizei size, const void *imageData)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (level == 0) {
        uint32_t mipLevel = (uint32_t)NUMBER_OF_MIP_LEVELS(width, height);
        if (mState[layer].Capacity() < mipLevel) {
            mState[layer].Reserve(mipLevel);
        }
        uint32_t count = mipLevel - mState[layer].Size();
        if (count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                mState[layer].PushBack(new State_t());
            }
        }
    }

    mState[layer][level]->width = width;
    mState[layer][level]->height = height;
    mState[layer][level]->format = internalformat;
    mState[layer][level]->type = GL_INVALID_VALUE;
    mState[layer][level]->size = size;

    if (layer == 0 && level == 0) {
        mIsNPOT = internalformat == GL_INVALID_VALUE ? false : (!ISPOWEROFTWO(width) || !ISPOWEROFTWO(height));
        UpdateNPOTAccessCompleted();
    }

    if (mState[layer][level]->data) {
        delete[](uint8_t *)mState[layer][level]->data;
        mState[layer][level]->data = nullptr;
    }

    if (imageData) {
        uint8_t *data = new uint8_t[size];
        if (data) {
            memcpy(data, imageData, size);
            mState[layer][level]->data = data;
        }
    }

    mDirty = true;
}

void 
Texture::CopyPixelsToHost(ImageRect *srcRect, ImageRect *dstRect, GLint miplevel, GLint layer, GLenum dstFormat, void *dstData)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const GLenum srcFormat = mExplicitInternalFormat;

    // create a buffer at the size of the requested subrectangle
    const size_t srcSize   = srcRect->GetRectBufferSize();
    BufferObject *tbo = new TransferDstBufferObject(mXContext);
    tbo->Allocate(srcSize, nullptr);

    // use the global rect offsets for transfering the subpixels from Vulkan
    SubmitCopyPixels(srcRect, tbo, miplevel, layer, dstFormat, false);

    // convert the destination buffer (both are similar dimensions) to the internal format
    uint8_t *srcData = new uint8_t[srcSize];
    tbo->GetData(srcSize, 0, srcData);

    ImageRect tmp_srcRect = *srcRect;
    ImageRect tmp_dstRect = *dstRect;
    tmp_srcRect.x = 0; tmp_srcRect.y = 0;
    tmp_dstRect.x = 0; tmp_dstRect.y = 0;
    ConvertPixels(srcFormat, dstFormat,
                  &tmp_srcRect, srcData,
                  &tmp_dstRect, dstData);

    if(!mDataNoInvertion) {
        InvertImageYAxis(static_cast<uint8_t *>(dstData), &tmp_dstRect);
    }
    mDataNoInvertion = false;

    delete    tbo;
    delete[]  srcData;
}

void 
Texture::CopyPixelsFromHost(ImageRect *srcRect, ImageRect *dstRect, GLint miplevel, GLint layer, GLenum srcFormat, const void *srcData)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const GLenum dstFormat = mExplicitInternalFormat;

    // create a buffer at the size of the requested subrectangle
    const size_t dstSize   = dstRect->GetRectBufferSize();
    uint8_t *dstData = new uint8_t[dstSize];

    // convert the destination buffer (both are similar dimensions) to the internal format
    ImageRect tmp_srcRect = *srcRect;
    ImageRect tmp_dstRect = *dstRect;
    tmp_srcRect.x = 0; tmp_srcRect.y = 0;
    tmp_dstRect.x = 0; tmp_dstRect.y = 0;
    ConvertPixels(srcFormat, dstFormat,
                  &tmp_srcRect, srcData,
                  &tmp_dstRect, dstData);

    BufferObject *tbo = new TransferSrcBufferObject(mXContext);
    tbo->Allocate(dstSize, dstData);

    // use the global rect offsets for transfering the subpixels to Vulkan
    SubmitCopyPixels(dstRect, tbo, miplevel, layer, dstFormat, true);

    delete    tbo;
    delete[]  dstData;

#if GLOVE_SAVE_TEXTURES_TO_FILE == true
    // TODO:: adjust for lod levels
    ImageRect _srcRect(0, 0, GetWidth(), GetHeight(),
                      GlInternalFormatTypeToNumElements(GetExplicitInternalFormat(), GetExplicitType()),
                      GlTypeToElementSize(GetExplicitType()),
                      Texture::GetDefaultInternalAlignment());
    const size_t texSize   = _srcRect.GetRectBufferSize();
    uint8_t *_writtenData = new uint8_t[texSize];
    CopyPixelsToHost(&_srcRect, &_srcRect, miplevel, layer, GetExplicitInternalFormat(), _writtenData);
    static int calls = 0;
    char fileName[64];
    snprintf(fileName, 64, "texture%d_%dx%d.rgba", calls++, GetWidth(), GetHeight());
    FILE *fp = fopen(fileName, "w");
    if(fp) {
        fwrite(_writtenData, texSize, 1, fp);
        fclose(fp);
    }
    delete[] _writtenData;
 #endif
}

void
Texture::CpoyCompressedPixelFromHost(Rect *srcRect, GLint miplevel, GLint layer, GLenum format, const void *srcData, GLsizei dataSize)
{
    BufferObject *tbo = new TransferSrcBufferObject(mXContext);
    tbo->Allocate(dataSize, srcData);

    // use the global rect offsets for transfering the subpixels to Vulkan
    SubmitCopyPixels(srcRect, tbo, miplevel, layer, format, true);
}

void 
Texture::SubmitCopyPixels(const Rect *rect, BufferObject *tbo, GLint miplevel, GLint layer, GLenum srcFormat, bool copyToImage)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mImage->CreateBufferImageCopy(rect->x, rect->y, rect->width, rect->height, miplevel, layer, 1);
    mImage->ModifyImageSubresourceRange(miplevel, 1, layer, 1);

    VkImageLayout oldImageLayout = mImage->GetImageLayout();
    oldImageLayout = (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED &&
                      oldImageLayout != VK_IMAGE_LAYOUT_PREINITIALIZED) ? oldImageLayout : VK_IMAGE_LAYOUT_GENERAL;
    VkImageLayout newImageLayout = copyToImage ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    mCommandBufferManager->BeginVkAuxCommandBuffer();
    VkCommandBuffer activeCmdBuffer = mCommandBufferManager->GetAuxCommandBuffer();
    {
        mImage->ModifyImageLayout(&activeCmdBuffer, newImageLayout);
        if(copyToImage) {
            mImage->CopyBufferToImage(&activeCmdBuffer, tbo->GetVkBuffer());
        } else {
            mImage->CopyImageToBuffer(&activeCmdBuffer, tbo->GetVkBuffer());
        }
        mImage->ModifyImageLayout(&activeCmdBuffer, oldImageLayout);
    }
    mCommandBufferManager->EndVkAuxCommandBuffer();
    mCommandBufferManager->SubmitVkAuxCommandBuffer();
    mCommandBufferManager->WaitVkAuxCommandBuffer();
}

void
Texture::PrepareVkImageLayout(VkImageLayout newImageLayout)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if (newImageLayout == mImage->GetImageLayout()) {
        return;
    }

    mCommandBufferManager->BeginVkAuxCommandBuffer();
    VkCommandBuffer cmdBuffer = mCommandBufferManager->GetAuxCommandBuffer();

    mImage->ModifyImageSubresourceRange(0, mMipLevelsCount, 0, mLayersCount);
    mImage->ModifyImageLayout(&cmdBuffer, newImageLayout);

    mCommandBufferManager->EndVkAuxCommandBuffer();
    mCommandBufferManager->SubmitVkAuxCommandBuffer();
    mCommandBufferManager->WaitVkAuxCommandBuffer();
}

void
Texture::InvertPixels()
{
    int numElements = (int)GlInternalFormatTypeToNumElements(GetExplicitInternalFormat(), GetExplicitType());
    int sizeElement = (int)GlTypeToElementSize(GetExplicitType());
    int alignment   = Texture::GetDefaultInternalAlignment();
    ImageRect srcRect(0, 0, GetWidth(), GetHeight(), numElements, sizeElement, alignment);
    ImageRect dstRect(0, 0, GetWidth(), GetHeight(), numElements, sizeElement, alignment);

    const size_t     baseLevel  = 0;
    const size_t     baseSize   = dstRect.GetRectBufferSize();
          uint8_t   *basePixels[1];
    for(GLint layer = 0; layer < 1; ++layer) {
        basePixels[layer] = new uint8_t[baseSize];
        CopyPixelsToHost(&srcRect, &dstRect, baseLevel, layer, GetExplicitInternalFormat(), basePixels[layer]);
    }

    for(GLint layer = 0; layer < 1; ++layer) {
        CopyPixelsFromHost(&srcRect, &dstRect, baseLevel, layer, GetExplicitInternalFormat(), basePixels[layer]);
        delete[] basePixels[layer];
    }
}

void
Texture::GenerateMipmaps(GLenum hintMipmapMode)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    int numElements = (int)GlInternalFormatTypeToNumElements(GetExplicitInternalFormat(), GetExplicitType());
    int sizeElement = (int)GlTypeToElementSize(GetExplicitType());
    int alignment   = Texture::GetDefaultInternalAlignment();
    ImageRect srcRect(0, 0, GetWidth(), GetHeight(), numElements, sizeElement, alignment);
    ImageRect dstRect(0, 0, GetWidth(), GetHeight(), numElements, sizeElement, alignment);

    const size_t     baseLevel  = 0;
    const size_t     baseSize   = dstRect.GetRectBufferSize();
          uint8_t  **basePixels = new uint8_t*[mLayersCount];
    for(GLint layer = 0; layer < mLayersCount; ++layer) {
        basePixels[layer] = new uint8_t[baseSize];
        CopyPixelsToHost(&srcRect, &dstRect, baseLevel, layer, GetExplicitInternalFormat(), basePixels[layer]);
    }

    // create Mipmapped Texture
    mMipLevelsCount = (GLint)NUMBER_OF_MIP_LEVELS(GetWidth(), GetHeight());
    CreateVkTexture();

    // set back base mipLevel for all layers
    for(GLint layer = 0; layer < mLayersCount; ++layer) {
        InvertImageYAxis(static_cast<uint8_t *>(basePixels[layer]), &srcRect);
        CopyPixelsFromHost(&srcRect, &dstRect, baseLevel, layer, GetExplicitInternalFormat(), basePixels[layer]);
        delete[] basePixels[layer];
    }

    delete [] basePixels;

    // Blit LoD Level '0' to rest layers
    VkImageBlit imageBlit;
    memset(static_cast<void *>(&imageBlit), 0, sizeof(imageBlit));
    imageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.mipLevel       = 0;
    imageBlit.srcSubresource.baseArrayLayer = 0;
    imageBlit.srcSubresource.layerCount     = mLayersCount;
    imageBlit.srcOffsets[1].x               = GetWidth();
    imageBlit.srcOffsets[1].y               = GetHeight();
    imageBlit.srcOffsets[1].z               = 1;

    imageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.mipLevel       = 1;
    imageBlit.dstSubresource.baseArrayLayer = 0;
    imageBlit.dstSubresource.layerCount     = mLayersCount;
    imageBlit.dstOffsets[1].x               = static_cast<int32_t>(std::max(std::floor(imageBlit.srcOffsets[1].x >> 1), 1.0));
    imageBlit.dstOffsets[1].y               = static_cast<int32_t>(std::max(std::floor(imageBlit.srcOffsets[1].y >> 1), 1.0));
    imageBlit.dstOffsets[1].z               = 1;

    mCommandBufferManager->BeginVkAuxCommandBuffer();
    VkCommandBuffer activeCmdBuffer = mCommandBufferManager->GetAuxCommandBuffer();
    {
        VkFilter      filter         = hintMipmapMode == GL_FASTEST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        VkImageLayout oldImageLayout = mImage->GetImageLayout();

        mImage->ModifyImageSubresourceRange(0, mMipLevelsCount, 0, mLayersCount);
        mImage->ModifyImageLayout(&activeCmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        for(GLint mipLevel = 1; mipLevel < mMipLevelsCount; ++mipLevel) {
            mImage->ModifyImageSubresourceRange(mipLevel, 1, 0, mLayersCount);
            mImage->ModifyImageLayout(&activeCmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            mImage->BlitImage        (&activeCmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                        mImage->GetImage(),
                                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                        &imageBlit, filter);
            mImage->ModifyImageLayout(&activeCmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            imageBlit.srcSubresource.mipLevel = imageBlit.dstSubresource.mipLevel;
            imageBlit.srcOffsets[1].x         = imageBlit.dstOffsets[1].x;
            imageBlit.srcOffsets[1].y         = imageBlit.dstOffsets[1].y;

            imageBlit.dstSubresource.mipLevel++;
            imageBlit.dstOffsets[1].x = static_cast<int32_t>(std::max(std::floor(imageBlit.srcOffsets[1].x >> 1), 1.0));
            imageBlit.dstOffsets[1].y = static_cast<int32_t>(std::max(std::floor(imageBlit.srcOffsets[1].y >> 1), 1.0));
        }
        mImage->ModifyImageSubresourceRange(0, mMipLevelsCount, 0, mLayersCount);
        mImage->ModifyImageLayout(&activeCmdBuffer, oldImageLayout);
    }
    mCommandBufferManager->EndVkAuxCommandBuffer();
    mCommandBufferManager->SubmitVkAuxCommandBuffer();
    mCommandBufferManager->WaitVkAuxCommandBuffer();

    // TODO: Fill the 'State_t' with the rest mipLevels
}

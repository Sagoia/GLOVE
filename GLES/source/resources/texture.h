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
 *  @file       texture.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Texture Functionality in GLOVE
 *
 */

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "rect.h"
#include "sampler.h"
#include "bufferObject.h"
#include "vulkan/sampler.h"
#include "vulkan/imageView.h"
#include "vulkan/cbManager.h"
#include "arrays.hpp"

#define ISPOWEROFTWO(x)           ((x != 0) && !(x & (x - 1)))

class CacheManager;

class Texture {

public:
    const static int TEXTURE_2D_LAYERS          = 1;
    const static int TEXTURE_CUBE_MAP_LAYERS    = 6;

private:
    struct State {
        GLint                      width;
        GLint                      height;
        GLenum                     format;
        GLenum                     type;
        void                       *data;
        GLsizei                    size;

        State() : width(-1), height(-1), format(GL_INVALID_VALUE), type(GL_INVALID_VALUE),
            data(nullptr), size(0){ FUN_ENTRY(GL_LOG_TRACE); }
        ~State() { FUN_ENTRY(GL_LOG_TRACE); if(data) {delete [] (uint8_t *)data; data = nullptr;}}
    };
    typedef State               State_t;
    typedef Array<State_t *>    States_t;

    const
    vulkanAPI::XContext_t *    mXContext;

    vulkanAPI::CommandBufferManager *mCommandBufferManager;

    CacheManager *              mCacheManager;

    GLenum                      mFormat;
    GLenum                      mTarget;
    GLenum                      mType;
    GLenum                      mInternalFormat;

    // NOTE: Formats are forced into GL_RGBA8_OES. Keep here the original format requested by the user
    GLenum                      mExplicitType;
    GLenum                      mExplicitInternalFormat;

    GLint                       mMipLevelsCount;
    GLint                       mLayersCount;

    Rect                        mDims;
    Sampler                     mParameters;
    States_t*                   mState;
    bool                        mDataUpdated;
    bool                        mDataNoInvertion;
    bool                        mFboColorAttached;
    bool                        mIsNPOT;
    bool                        mIsNPOTAccessCompleted;
    
    Texture                    *mDepthStencilTexture;
    uint32_t                    mDepthStencilTextureRefCount;

    bool                        mDirty;

    vulkanAPI::Image*           mImage;
    vulkanAPI::Memory*          mMemory;
    vulkanAPI::Sampler*         mSampler;
    vulkanAPI::ImageView*       mImageView;

    static int                  mDefaultInternalAlignment;

    bool                        AllocateVkMemory(void);
    void                        ReleaseVkResources(void);

public:
    Texture(const vulkanAPI::XContext_t  *xContext = nullptr, vulkanAPI::CommandBufferManager *cbManager = nullptr,
            const VkFlags       vkFlags   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ~Texture();

// Generate Functions
    bool                    Allocate();
    void                    SetState(GLsizei width, GLsizei height, GLint level, GLint layer, GLenum format, GLenum type, GLint unpackAlignment, const void *pixels);
    void                    SetSubState(ImageRect *srcRect, ImageRect *dstRect, GLint miplevel, GLint layer, GLenum srcFormat, const void *srcData);
    void                    SetCompressedState(GLsizei width, GLsizei height, GLint level, GLint layer, GLenum internalformat, GLsizei size, const void *imageData);
    void                    GenerateMipmaps(GLenum hintMipmapMode);

// Init Functions
    inline void             InitState(void)                                     { FUN_ENTRY(GL_LOG_TRACE); mLayersCount  = mTarget == GL_TEXTURE_2D ? TEXTURE_2D_LAYERS : TEXTURE_CUBE_MAP_LAYERS;
                                                                                                           mState        = new States_t[mLayersCount]; }

// Helper Functions
    static int              GetDefaultInternalAlignment()                       { FUN_ENTRY(GL_LOG_TRACE); return mDefaultInternalAlignment; }
    inline int              GetInvertedYOrigin(const Rect* rect)                { FUN_ENTRY(GL_LOG_TRACE); return mDims.height - rect->height - rect->y; }
    void                    PrepareVkImageLayout(VkImageLayout newImageLayout);

// Create Functions
    bool                    CreateVkTexture(void);
    bool                    CreateVkImage(void);
    bool                    CreateVkImageView(void)                             { FUN_ENTRY(GL_LOG_TRACE); return mImageView->Create(mImage); }
    bool                    CreateVkSampler(void)                               { FUN_ENTRY(GL_LOG_TRACE); return mSampler->Create(); }
    void                    CreateVkImageSubResourceRange(void)                 { FUN_ENTRY(GL_LOG_TRACE); return mImage->CreateImageSubresourceRange(); }

// Copy Functions
     void                   CopyPixelsFromHost (ImageRect *srcRect, ImageRect *dstRect, GLint miplevel, GLint layer, GLenum srcFormat, const void *srcData);
     void                   CpoyCompressedPixelFromHost(Rect *srcRect, GLint miplevel, GLint layer, GLenum format, const void *srcData, GLsizei dataSize);
     void                   CopyPixelsToHost   (ImageRect *srcRect, ImageRect *dstRect, GLint miplevel, GLint layer, GLenum dstFormat, void *dstData);
     void                   SubmitCopyPixels   (const Rect *rect, BufferObject *tbo, GLint miplevel, GLint layer, GLenum dstFormat, bool copyToImage);
     void                   InvertPixels       (void);

// Get Functions
    inline GLenum           GetWrapS(void)                              const   { FUN_ENTRY(GL_LOG_TRACE); return mParameters.GetWrapS(); }
    inline GLenum           GetWrapT(void)                              const   { FUN_ENTRY(GL_LOG_TRACE); return mParameters.GetWrapT(); }
    inline GLenum           GetMinFilter(void)                          const   { FUN_ENTRY(GL_LOG_TRACE); return mParameters.GetMinFilter(); }
    inline GLenum           GetMagFilter(void)                          const   { FUN_ENTRY(GL_LOG_TRACE); return mParameters.GetMagFilter(); }
    inline int              GetWidth(void)                              const   { FUN_ENTRY(GL_LOG_TRACE); return mDims.width; }
    inline int              GetHeight(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mDims.height; }
    inline GLenum           GetType(void)                               const   { FUN_ENTRY(GL_LOG_TRACE); return mType; }
    inline GLenum           GetExplicitType(void)                       const   { FUN_ENTRY(GL_LOG_TRACE); return mExplicitType; }
    inline GLenum           GetFormat(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mFormat; }
    inline GLenum           GetTarget(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mTarget; }
    inline GLenum           GetInternalFormat(void)                     const   { FUN_ENTRY(GL_LOG_TRACE); return mInternalFormat; }
    inline GLenum           GetExplicitInternalFormat(void)             const   { FUN_ENTRY(GL_LOG_TRACE); return mExplicitInternalFormat; }
    inline GLint            GetLayersCount(void)                        const   { FUN_ENTRY(GL_LOG_TRACE); return mLayersCount; }
    inline GLint            GetMipLevelsCount(void)                     const   { FUN_ENTRY(GL_LOG_TRACE); return mMipLevelsCount; }
    inline bool             GetDataUpdated(void)                        const   { FUN_ENTRY(GL_LOG_TRACE); return mDataUpdated; }
    
    inline Texture         *GetDepthStencilTexture(void)                const   { FUN_ENTRY(GL_LOG_TRACE); return mDepthStencilTexture;}
    inline uint32_t         GetDepthStencilTextureRefCount(void)        const   { FUN_ENTRY(GL_LOG_TRACE); return mDepthStencilTextureRefCount; }

    inline vulkanAPI::Image* GetImage(void)                                     { FUN_ENTRY(GL_LOG_TRACE); return mImage; }

    inline VkSampler        GetVkSampler(void)                          const   { FUN_ENTRY(GL_LOG_TRACE); return mSampler->GetSampler(); }
    inline VkFormat         GetVkFormat(void)                           const   { FUN_ENTRY(GL_LOG_TRACE); return mImage->GetFormat(); }
    inline VkFlags          GetVkImageUsage(void)                       const   { FUN_ENTRY(GL_LOG_TRACE); return mImage->GetImageUsage(); }
    inline VkImageLayout    GetVkImageLayout(void)                      const   { FUN_ENTRY(GL_LOG_TRACE); return mImage->GetImageLayout(); }
    inline VkImageView      GetVkImageView(void)                        const   { FUN_ENTRY(GL_LOG_TRACE); return mImageView->GetImageView(); }
    VkFormat                FindSupportedVkColorFormat(VkFormat format)         { FUN_ENTRY(GL_LOG_TRACE); return mImage->FindSupportedVkColorFormat(format); }

// Set Functions
    inline void             SetCommandBufferManager(
                                vulkanAPI::CommandBufferManager *cbManager)     { FUN_ENTRY(GL_LOG_TRACE); mCommandBufferManager = cbManager; }
    inline void             SetCacheManager(CacheManager *cacheManager)         { FUN_ENTRY(GL_LOG_TRACE); mCacheManager = cacheManager;
                                                                                                           mSampler->SetCacheManager(cacheManager);
                                                                                                           mImageView->SetCacheManager(cacheManager);
                                                                                                           mImage->SetCacheManager(cacheManager);
                                                                                                           mMemory->SetCacheManager(cacheManager); }
    inline void             SetxContext(const vulkanAPI::XContext_t *xContext)  { FUN_ENTRY(GL_LOG_TRACE); mXContext = xContext;
                                                                                                           mMemory->SetContext(xContext);
                                                                                                           mSampler->SetContext(xContext);
                                                                                                           mImageView->SetContext(xContext);
                                                                                                           mImage->SetContext(xContext); }
    inline void             SetWrapS(GLenum mode)                               { FUN_ENTRY(GL_LOG_TRACE); if(mParameters.UpdateWrapS(mode)) { \
                                                                                                           mSampler->SetAddressModeU(GlTexAddressToXTexAddress(mode)); \
                                                                                                           UpdateNPOTAccessCompleted();}}
    inline void             SetWrapT(GLenum mode)                               { FUN_ENTRY(GL_LOG_TRACE); if(mParameters.UpdateWrapT(mode)) { \
                                                                                                           mSampler->SetAddressModeV(GlTexAddressToXTexAddress(mode)); \
                                                                                                           UpdateNPOTAccessCompleted();}}
    inline void             SetMinFilter(GLenum mode)                           { FUN_ENTRY(GL_LOG_TRACE); if(mParameters.UpdateMinFilter(mode)){ \
                                                                                                           mSampler->SetMinFilter(GlTexFilterToXTexFilter(mode)); \
                                                                                                           mSampler->SetMipmapMode(GlTexMipMapModeToXMipMapMode(mode)); \
                                                                                                           mSampler->SetMaxLod((mode == GL_NEAREST || mode == GL_LINEAR) ? 0.25f : static_cast<float>(mMipLevelsCount-1)); \
                                                                                                           UpdateNPOTAccessCompleted();}}
    inline void             SetMagFilter(GLenum mode)                           { FUN_ENTRY(GL_LOG_TRACE); if(mParameters.UpdateMagFilter(mode)) { \
                                                                                                           mSampler->SetMagFilter(GlTexFilterToXTexFilter(mode)); \
                                                                                                           UpdateNPOTAccessCompleted();}}
    inline void             SetWidth(int width)                                 { FUN_ENTRY(GL_LOG_TRACE); mDims.width  = width;  }
    inline void             SetHeight(int height)                               { FUN_ENTRY(GL_LOG_TRACE); mDims.height = height; }
    inline void             SetTarget(GLenum target)                            { FUN_ENTRY(GL_LOG_TRACE); mTarget      = target; }
    inline void             SetFormat(GLenum format)                            { FUN_ENTRY(GL_LOG_TRACE); mFormat      = format; }
    inline void             SetType(GLenum type)                                { FUN_ENTRY(GL_LOG_TRACE); mType        = type;   }
    inline void             SetExplicitType(GLenum type)                        { FUN_ENTRY(GL_LOG_TRACE); mExplicitType = type;  }
    inline void             SetInternalFormat(GLenum format)                    { FUN_ENTRY(GL_LOG_TRACE); mInternalFormat         = format;  }
    inline void             SetExplicitInternalFormat(GLenum format)            { FUN_ENTRY(GL_LOG_TRACE); mExplicitInternalFormat = format;  }
    inline void             SetDataUpdated(bool updated)                        { FUN_ENTRY(GL_LOG_TRACE); mDataUpdated = updated; }
    inline void             SetDataNoInvertion(bool updated)                    { FUN_ENTRY(GL_LOG_TRACE); mDataNoInvertion = updated; }
    inline void             SetFboColorAttached(bool updated)                   { FUN_ENTRY(GL_LOG_TRACE); mFboColorAttached = updated; }
    inline void             SetDepthStencilTexture(Texture *tex)                { FUN_ENTRY(GL_LOG_TRACE); mDepthStencilTexture = tex;}

    inline void             SetImageBufferCopyStencil(bool copy)                { FUN_ENTRY(GL_LOG_TRACE); mImage->SetCopyStencil(copy);   }
    inline void             SetVkFormat(VkFormat format)                        { FUN_ENTRY(GL_LOG_TRACE); mImage->SetFormat(format);      }
    inline void             SetVkImage(VkImage image)                           { FUN_ENTRY(GL_LOG_TRACE); mImage->SetImage(image);        }
    inline void             SetVkImageUsage(VkImageUsageFlags usage)            { FUN_ENTRY(GL_LOG_TRACE); mImage->SetImageUsage(usage);   }
    inline void             SetVkImageLayout(VkImageLayout layout)              { FUN_ENTRY(GL_LOG_TRACE); mImage->SetImageLayout(layout); }
    inline void             SetVkImageTiling(VkImageTiling tiling)              { FUN_ENTRY(GL_LOG_TRACE); mImage->SetImageTiling(tiling); }
    inline void             SetVkImageTiling(void)                              { FUN_ENTRY(GL_LOG_TRACE); mImage->SetImageTiling();       }
    inline void             SetXImageTarget(XImageTarget target)                { FUN_ENTRY(GL_LOG_TRACE); mImage->SetImageTarget(target); }

// Increase/Decrease Functions
    inline void             IncreaseDepthStencilTextureRefCount(void)                              { FUN_ENTRY(GL_LOG_TRACE); ++mDepthStencilTextureRefCount; }
    inline void             DecreaseDepthStencilTextureRefCount(void)                              { FUN_ENTRY(GL_LOG_TRACE); --mDepthStencilTextureRefCount; }

// Is Functions
    inline bool             IsCubeMap(void)                             const   { FUN_ENTRY(GL_LOG_TRACE); return mTarget  == GL_TEXTURE_CUBE_MAP; }
    inline bool             IsCompressed(void)                          const   { FUN_ENTRY(GL_LOG_TRACE); return (mFormat != GL_ALPHA           &&
                                                                                                                   mFormat != GL_RGB             &&
                                                                                                                   mFormat != GL_RGBA            &&
                                                                                                                   mFormat != GL_LUMINANCE       &&
                                                                                                                   mFormat != GL_LUMINANCE_ALPHA &&
                                                                                                                   mFormat != GL_BGRA_EXT); }
           void             UpdateNPOTAccessCompleted(void);
    inline bool             IsNPOT(void)                                const   { FUN_ENTRY(GL_LOG_TRACE); return mIsNPOT; }
    inline bool             IsNPOTAccessCompleted(void)                 const   { FUN_ENTRY(GL_LOG_TRACE); return mIsNPOTAccessCompleted; }
           bool             IsCompleted(void);
           bool             IsValid(void);
};

#endif // __TEXTURE_H__

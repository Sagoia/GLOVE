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
 *  @file       image.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Resource Creation Functionality via Images in Vulkan
 *
 */

#ifndef __VKIMAGE_H__
#define __VKIMAGE_H__

#include "utils.h"
#include "context.h"
#include "buffer.h"

class CacheManager;

namespace vulkanAPI {

class Image {

public:
    const static int TEXTURE_2D_LAYERS          = 1;
    const static int TEXTURE_CUBE_MAP_LAYERS    = 6;

private:

    const
    XContext_t *                      mXContext;

    VkImage                           mVkImage;
    VkFormat                          mVkFormat;
    VkImageType                       mVkImageType;
    VkImageUsageFlags                 mVkImageUsage;
    VkImageLayout                     mVkImageLayout;
    VkPipelineStageFlags              mVkPipelineStage;
    VkImageTiling                     mVkImageTiling;
    VkImageSubresourceRange           mVkImageSubresourceRange;
    XImageTarget                      mXImageTarget;
    VkSampleCountFlagBits             mVkSampleCount;
    VkSharingMode                     mVkSharingMode;
    VkBufferImageCopy                 mVkBufferImageCopy;

    uint32_t                          mWidth;
    uint32_t                          mHeight;
    uint32_t                          mMipLevels;
    uint32_t                          mLayers;
    VkBool32                          mDelete;

    bool                              mCopyStencil;

    CacheManager *                    mCacheManager;

public:
// Constructor
    Image(const XContext_t *xContext = nullptr);

// Destructor
    ~Image();

// Blit Functions
    void                              BlitImage(GLenum hintMipmapMode, VkCommandBuffer activeCmdBuffer);

// Create Functions
    bool                              Create(void);
    void                              CreateImageSubresourceRange(void);
    void                              CreateBufferImageCopy(int32_t offsetX, int32_t offsetY, uint32_t extentWidth, uint32_t extentHeight, uint32_t miplevel, uint32_t layer, uint32_t layerCount);

// Copy Functions
    void                              DoCopy(VkCommandBuffer activeCmdBuffer, Buffer *srcBuffer, bool copyToImage);
    void                              CopyBufferToImage(VkCommandBuffer activeCmdBuffer, Buffer *srcBuffer);
    void                              CopyImageToBuffer(VkCommandBuffer activeCmdBuffer, Buffer *srcBuffer);

// Modify Functions
    void                              ModifyImageSubresourceRange(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);
    inline bool                       CanModifyImageLayout(XImageLayout newImageLayout)
                                                                                { FUN_ENTRY(GL_LOG_DEBUG); return newImageLayout != mVkImageLayout; }
    inline void                       ModifyImageLayout(VkCommandBuffer activeCmdBuffer, XImageLayout newImageLayout) 
                                                                                { FUN_ENTRY(GL_LOG_DEBUG); ModifyImageLayout(activeCmdBuffer, (VkImageLayout)newImageLayout); }
    void                              ModifyImageLayout(VkCommandBuffer activeCmdBuffer, VkImageLayout newImageLayout);

// Release Functions
    void                              Release(void);

// Get Functions
    inline VkImage &                  GetImage(void)                            { FUN_ENTRY(GL_LOG_TRACE); return mVkImage;          }
    inline VkFormat                   GetFormat(void)                     const { FUN_ENTRY(GL_LOG_TRACE); return mVkFormat;         }
    inline XImageTarget               GetImageTarget(void)                const { FUN_ENTRY(GL_LOG_TRACE); return mXImageTarget;     }
    inline VkImageUsageFlags          GetImageUsage(void)                 const { FUN_ENTRY(GL_LOG_TRACE); return mVkImageUsage;     }
    inline VkImageLayout              GetImageLayout(void)                const { FUN_ENTRY(GL_LOG_TRACE); return mVkImageLayout;    }
    inline VkBufferImageCopy *        GetBufferImageCopy(void)                  { FUN_ENTRY(GL_LOG_TRACE); return &mVkBufferImageCopy;      }
    inline VkImageSubresourceRange    GetImageSubresourceRange(void)      const { FUN_ENTRY(GL_LOG_TRACE); return mVkImageSubresourceRange; }
    inline uint32_t                   GetMipLevels(void)                  const { FUN_ENTRY(GL_LOG_TRACE); return mMipLevels;        }
    inline uint32_t                   GetLayers(void)                     const { FUN_ENTRY(GL_LOG_TRACE); return mLayers;           }

// Set Functions
    inline void                       SetContext(const XContext_t *xContext)    { FUN_ENTRY(GL_LOG_TRACE); mXContext      = xContext; }
    inline void                       SetFormat(VkFormat format)                { FUN_ENTRY(GL_LOG_TRACE); mVkFormat      = format;    }
    inline void                       SetCopyStencil(bool copy)                 { FUN_ENTRY(GL_LOG_TRACE); mCopyStencil   = copy;      }
    inline void                       SetImage(VkImage image)                   { FUN_ENTRY(GL_LOG_TRACE); mVkImage       = image;
                                                                                                           mDelete        = false;     }
    inline void                       SetImageUsage(VkImageUsageFlags usage)    { FUN_ENTRY(GL_LOG_TRACE); mVkImageUsage  = usage;     }
           void                       SetImageTiling();
    inline void                       SetImageTiling(VkImageTiling tiling)      { FUN_ENTRY(GL_LOG_TRACE); mVkImageTiling = tiling;    }
    inline void                       SetImageTarget(XImageTarget target)       { FUN_ENTRY(GL_LOG_TRACE); mXImageTarget  = target;    }
    inline void                       SetImageLayout(VkImageLayout layout)      { FUN_ENTRY(GL_LOG_TRACE); mVkImageLayout = layout;    }
    inline void                       SetWidth(uint32_t width)                  { FUN_ENTRY(GL_LOG_TRACE); mWidth         = width;     }
    inline void                       SetHeight(uint32_t height)                { FUN_ENTRY(GL_LOG_TRACE); mHeight        = height;    }
    inline void                       SetMipLevels(uint32_t levels)             { FUN_ENTRY(GL_LOG_TRACE); mMipLevels     = levels;    }

    inline void                       SetCacheManager(CacheManager *manager)    { FUN_ENTRY(GL_LOG_TRACE); mCacheManager  = manager;   }

// Find Functions
    XFormat                           FindSupportedColorFormat(XFormat format);
};

}

#endif // __VKIMAGE_H__

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
 *  @file       vulkanResources.cpp
 *  @author     Think Silicon
 *  @date       26/08/2018
 *  @version    1.0
 *
 *  @brief      Vulkan Resources module. It provides an interface to basic Vulkan resources.
 *
 */

#include "vulkanResources.h"

VulkanResources::VulkanResources()
    : mSurface(VK_NULL_HANDLE), mSwapchain(VK_NULL_HANDLE),
      mSwapChainImageCount(0), mSwapChainImages(nullptr)
{
    FUN_ENTRY(DEBUG_DEPTH);
}

VulkanResources::~VulkanResources()
{
    FUN_ENTRY(DEBUG_DEPTH);

    Release();
}

void
VulkanResources::Release(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(mSwapChainImages) {
        delete[] mSwapChainImages;
        mSwapChainImages = nullptr;
        mSwapChainImageCount = 0;
   } 
}

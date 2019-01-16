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
 *  @file       displayDriversContainer.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Container of EGL Display Drivers. It allocates a new DisplayDriver for each new EGL Display connection.
 *
 */

#include "displayDriversContainer.h"
#include <algorithm>

DisplayDriversContainer *DisplayDriversContainer::mInstance = nullptr;

DisplayDriversContainer::DisplayDriversContainer()
: mDriversNumber(0)
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

DisplayDriver*
DisplayDriversContainer::FindDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    auto it = mDriverMap.find(display);
    return (it != mDriverMap.end()) ? it->second : nullptr;
}

DisplayDriver*
DisplayDriversContainer::GetDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    DisplayDriver *driver = FindDriver(display);

    if(driver) {
        return driver;
    }

    DisplayDriver *newDriver = new DisplayDriver();
    mDriversNumber++;

    mDriverMap[display] = newDriver;
    return newDriver;
}

DisplayDriver*
DisplayDriversContainer::RemoveDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    DisplayDriver *removedDriver = nullptr;

    auto it = mDriverMap.find(display);
    if(mDriverMap.end() == it) {
        return nullptr;
    }

    removedDriver = it->second;
    mDriverMap.erase(it);
    mDriversNumber--;

    return removedDriver;
}

DisplayDriversContainer*
DisplayDriversContainer::GetInstance()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(!mInstance) {
        mInstance = new DisplayDriversContainer();
    }

    return mInstance;
}

void
DisplayDriversContainer::DestroyInstance()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(mInstance) {
        delete mInstance;
        mInstance = nullptr;
    }
}

DisplayDriver*
DisplayDriversContainer::FindDisplayDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(display == nullptr) {
        return nullptr;
    }

    DisplayDriversContainer *displayDrivers = DisplayDriversContainer::GetInstance();

    return displayDrivers->FindDriver(display);
}

DisplayDriver*
DisplayDriversContainer::AddDisplayDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(display == nullptr) {
        return nullptr;
    }

    DisplayDriversContainer *displayDrivers = DisplayDriversContainer::GetInstance();

    return displayDrivers->GetDriver(display);
}

void
DisplayDriversContainer::RemoveDisplayDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(display == nullptr) {
        return;
    }

    DisplayDriversContainer *displayDrivers = DisplayDriversContainer::GetInstance();

    DisplayDriver *removedDriver = displayDrivers->RemoveDriver(display);
    if(removedDriver) {
        delete removedDriver;
    }
}

void
DisplayDriversContainer::Destroy()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    DisplayDriversContainer::DestroyInstance();
}

EGLBoolean
DisplayDriversContainer::IsEmpty()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    DisplayDriversContainer *displayDrivers = DisplayDriversContainer::GetInstance();

    return 0 == displayDrivers->GetDriversNumber() ? EGL_TRUE : EGL_FALSE;
}

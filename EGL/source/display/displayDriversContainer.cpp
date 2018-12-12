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

DisplayDriversContainer::DisplayDriversContainer()
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

DisplayDriversContainer::~DisplayDriversContainer()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    for(auto driver : mDriverMap) {
        delete driver.second;
    }
    mDriverMap.clear();
}

DisplayDriver*
DisplayDriversContainer::FindDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    auto it = mDriverMap.find(display);
    return (it != mDriverMap.end()) ? it->second : nullptr;
}

DisplayDriver*
DisplayDriversContainer::AddDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    DisplayDriver *driver = FindDriver(display);

    if(driver) {
        return driver;
    }

    DisplayDriver *newDriver = new DisplayDriver(display);

    mDriverMap[display] = newDriver;
    return newDriver;
}

void
DisplayDriversContainer::RemoveDriver(EGLDisplay_t* display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    DisplayDriver *removedDriver = nullptr;

    auto it = mDriverMap.find(display);
    if(mDriverMap.end() == it) {
        return;
    }

    removedDriver = it->second;
    mDriverMap.erase(it);

    delete removedDriver;
}

EGLBoolean
DisplayDriversContainer::IsEmpty()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    return mDriverMap.size() == 0 ? EGL_TRUE : EGL_FALSE;
}

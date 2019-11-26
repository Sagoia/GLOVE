########################################################################
# CMake build script for GLOVE
########################################################################

# Sets the minimum required version of cmake for a project.
# If the current version of CMake is lower than that required it will stop
# processing the project.
cmake_minimum_required(VERSION 2.8.12)

project(GLOVE)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected. Default: Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default: Release)" FORCE)
endif()
# Enables/Disables output of compile commands during generation.
# If enabled, generates a compile_commands.json file containing the exact
# compiler calls for all translation units of the project in machine-readable
# form.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")

# Search for Vulkan library
if(VULKAN_LIBRARY)
    set(Vulkan_FOUND ON)
    set(Vulkan_LIBRARY "${VULKAN_LIBRARY}" CACHE PATH "" FORCE)
    if(VULKAN_INCLUDE_PATH)
        set(Vulkan_INCLUDE_DIR "${VULKAN_INCLUDE_PATH}" CACHE PATH "" FORCE)
    else()
        get_filename_component(VULKAN_LIB_DIR "${VULKAN_LIBRARY}" DIRECTORY)
        set(Vulkan_INCLUDE_DIR "${VULKAN_LIB_DIR}/../include" CACHE PATH "" FORCE)
    endif()
else()
    if(NOT CMAKE_VERSION VERSION_LESS 3.7.2)
        find_package(Vulkan)
    else()
    if (APPLE)
            find_library(Vulkan_LIBRARY NAMES libMoltenVK.dylib HINTS ${CMAKE_SOURCE_DIR}/../MoltenVK/Package/Release/MoltenVK/macOS/dynamic)
    else()
            find_library(Vulkan_LIBRARY NAMES libvulkan.so libvulkan.so.1 HINTS ${CMAKE_INSTALL_FULL_LIBDIR})
    endif()
        find_path(Vulkan_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS ${CMAKE_INSTALL_FULL_LIBDIR})
        if(Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
        endif()
    endif()
endif()
if(Vulkan_FOUND)
    message(STATUS "Found Vulkan: ${Vulkan_LIBRARY}")
else()
    message(FATAL_ERROR "Could not find Vulkan library: ${Vulkan_LIBRARY}")
endif()

option(TRACE_BUILD "Build GLOVE with debug logs enabled" OFF)
if(TRACE_BUILD)
    message(STATUS "Building GLOVE with debug logs enabled")
    add_definitions(-DTRACE_BUILD)
else()
    remove_definitions(-DTRACE_BUILD)
endif()

add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

# Set c/cpp flag definitions for the compiler.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(C_REDUCE_ERRORS "-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -DNOMINMAX /wd\"4099\" /wd\"4101\"")
    set(CXX_REDUCE_ERRORS "-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -DNOMINMAX /wd\"4099\" /wd\"4101\"")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES -DGL_GLEXT_PROTOTYPES ${CXX_REDUCE_ERRORS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES -DGL_GLEXT_PROTOTYPES ${C_REDUCE_ERRORS}")

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    set(C_REDUCE_ERRORS "-Wno-unused-parameter -Wno-unused-function")
    set(CXX_REDUCE_ERRORS "-Wno-unused-parameter -Wno-unused-function")
    set(PEDANTIC "-Wall -Wextra -Winline -Wreturn-type -Wuninitialized -Winit-self")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES -DGL_GLEXT_PROTOTYPES -std=c++11 ${PEDANTIC} ${CXX_REDUCE_ERRORS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES -DGL_GLEXT_PROTOTYPES -std=c99 ${PEDANTIC} ${C_REDUCE_ERRORS}")
endif()

set(USE_SURFACE XCB CACHE STRING "Use surface")
set_property(CACHE USE_SURFACE PROPERTY STRINGS DISPLAY XCB ANDROID)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(USE_SURFACE STREQUAL "DISPLAY")
        MESSAGE(STATUS "Using Native surface for display")
    elseif(USE_SURFACE STREQUAL "XCB")
        MESSAGE(STATUS "Using XCB surface for display")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
    elseif(USE_SURFACE STREQUAL "WAYLAND")
        MESSAGE(STATUS "Using WAYLAND surface for display")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR -DWL_EGL_PLATFORM")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR -DWL_EGL_PLATFORM")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(USE_SURFACE ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_ANDROID_KHR")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_USE_PLATFORM_ANDROID_KHR")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(USE_SURFACE STREQUAL "XCB")
        MESSAGE(STATUS "Using XCB surface for display")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
    elseif(USE_SURFACE STREQUAL "MACOS")
        set(USE_SURFACE MACOS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK")
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(USE_SURFACE WINDOWS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
endif()

# Recurse into the the following subdirectories. This does not actually cause
# another cmake executable to run. The same process will walk through the
# project's entire directory structure.
add_subdirectory(EGL)
add_subdirectory(GLES)
add_subdirectory(Demos)

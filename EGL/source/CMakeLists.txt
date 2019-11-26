message(STATUS "  Building EGL Library")

# Sets the SOURCES variable to contain all the source files needed by
# EGL shared lib to be built.
set(SOURCES
    api/eglContext.cpp
    api/eglConfig.cpp
    api/egl.cpp
    api/eglSurface.cpp
    api/eglRefObject.cpp
    api/eglGlobalResourceManager.cpp
    display/displayDriver.cpp
    display/displayDriversContainer.cpp
    display/displayDriverResourceManager.cpp
    thread/renderingThread.cpp
    platform/platformFactory.cpp
    platform/vulkan/vulkanWindowInterface.cpp
    platform/vulkan/vulkanWSI.cpp
    platform/vulkan/vulkanAPI.cpp
    platform/vulkan/vulkanResources.cpp
    platform/vulkan/WSIPlaneDisplay.cpp
    rendering_api/rendering_api.c
    utils/eglUtils.cpp
    utils/eglLogger.cpp
)
set(HEADERS
    api/eglConfig.h
    api/eglConfig_android.h
    api/eglConfig_linux.h
    api/eglContext.h
    api/eglDisplay.h
    api/eglFunctions.h
    api/eglSurface.h
    display/displayDriver.h
    display/displayDriversContainer.h
    thread/renderingThread.h
    platform/platformFactory.h
    platform/platformResources.h
    platform/platformWindowInterface.h
    platform/vulkan/vulkanWindowInterface.h
    platform/vulkan/vulkanWSI.h
    platform/vulkan/vulkanAPI.h
    platform/vulkan/vulkanResources.h
    platform/vulkan/WSIMacOS.h
    platform/vulkan/WSIPlaneDisplay.h
    platform/vulkan/WSIWindows.h
    platform/vulkan/WSIXcb.h
    rendering_api/rendering_api.h
    utils/egl_defs.h
    utils/eglLogger.h
    utils/eglLoggerImpl.h
    utils/eglUtils.h
    utils/simpleLoggerImpl.h
)

set(OTHER_HEADERS
    ${CMAKE_SOURCE_DIR}/GLES/source/utils/arrays.hpp
    ${CMAKE_SOURCE_DIR}/EGL/include/rendering_api_interface.h
)


# For Microsoft Windows add the module-definition file to SOURCES (necessary to export API functions)
if(WIN32)
    set(SOURCES ${SOURCES} api/egl.def)
endif()

# Sets the LIBS variable to contain the libraries that GLESv2 shared library
# will need to be linked against to.
set(LIBS
    ${Vulkan_LIBRARY}
)

if(USE_SURFACE STREQUAL "XCB")
    find_package(X11)
    if(X11_FOUND)
        find_package(ECM REQUIRED NO_MODULE)
        ecm_use_find_modules(DIR "${CMAKE_SOURCE_DIR}/CMake"
                             MODULES FindXCB.cmake)
        find_package(XCB REQUIRED XCB)

        set(SOURCES ${SOURCES} platform/vulkan/WSIXcb.cpp)

        set(LIBS ${LIBS} X11-xcb)
    else()
        # Take action for other platforms
        message(FATAL_ERROR "Could not find X11 LIBRARIES")
    endif()
endif()

if(USE_SURFACE STREQUAL "WAYLAND")
    find_package(ECM REQUIRED NO_MODULE)
    ecm_use_find_modules(DIR "${CMAKE_SOURCE_DIR}/CMake"
                         MODULES FindWayland.cmake)
    find_package(Wayland)

    if(Wayland_FOUND)
        set(SOURCES ${SOURCES} platform/vulkan/WSIWayland.cpp)
        set(LIBS ${LIBS} wayland-client)
     else()
         message(FATAL_ERROR "Could not find Wayland LIBRARIES")
     endif()
endif()

if(USE_SURFACE STREQUAL "MACOS")
    set(SOURCES ${SOURCES} platform/vulkan/WSIMacOS.mm)
endif()

if(USE_SURFACE STREQUAL "WINDOWS")
    set(SOURCES ${SOURCES} platform/vulkan/WSIWindows.cpp)
endif()

include_directories(${CMAKE_SOURCE_DIR}/EGL/source
                    ${CMAKE_SOURCE_DIR}/EGL/include
                    ${CMAKE_SOURCE_DIR}/GLES/include
                    ${Vulkan_INCLUDE_DIR}
                    ${CMAKE_INSTALL_FULL_INCLUDEDIR})

if(WIN32)
    add_library(EGL SHARED ${HEADERS} ${SOURCES})

    set_target_properties(EGL PROPERTIES PREFIX "lib"
                                         POSITION_INDEPENDENT_CODE ON
                                         LINK_FLAGS "/INCREMENTAL:NO"
                                         INSTALL_RPATH_USE_LINK_PATH TRUE
                                         INSTALL_RPATH "\$ORIGIN")
else()
    if(APPLE)
        find_library(APPKIT APPKit)

        add_library(EGL SHARED ${OTHER_HEADERS} ${HEADERS} ${SOURCES})

        set_target_properties(EGL PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                             XCODE_ATTRIBUTE_VALID_ARCHS "x86_64"
					     #XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "YES"
                                             POSITION_INDEPENDENT_CODE ON
				             MACOSX_RPATH TRUE
					     SKIP_BUILD_RPATH FALSE
                                             #LINK_FLAGS "-Wl,-Bsymbolic"
                                             INSTALL_RPATH_USE_LINK_PATH TRUE
                                             INSTALL_RPATH "\$ORIGIN")
        target_link_libraries(EGL ${APPKIT})
    else()
        add_library(EGL SHARED ${SOURCES})
        set_target_properties(EGL PROPERTIES POSITION_INDEPENDENT_CODE ON
                                             LINK_FLAGS "-Wl,-Bsymbolic"
                                             INSTALL_RPATH_USE_LINK_PATH TRUE
                                             INSTALL_RPATH "\$ORIGIN")
    endif()
endif()



target_link_libraries(EGL ${LIBS})

if(WIN32)
    install(TARGETS EGL RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
else()
    install(TARGETS EGL LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

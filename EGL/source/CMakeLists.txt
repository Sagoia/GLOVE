message(STATUS "  Building EGL Library")

# Sets the SOURCES variable to contain all the source files needed by
# EGL shared lib to be built.
set(SOURCES
    api/eglContext.cpp
    api/eglConfig.cpp
    api/eglDisplay.cpp
    api/egl.cpp
    api/eglSurface.cpp
    display/displayDriver.cpp
    display/displayDriversContainer.cpp
    thread/renderingThread.cpp
    platform/platformFactory.cpp
    platform/vulkan/vulkanWindowInterface.cpp
    platform/vulkan/vulkanWSI.cpp
    platform/vulkan/vulkanAPI.cpp
    platform/vulkan/vulkanResources.cpp
    platform/vulkan/WSIPlaneDisplay.cpp
    platform/vulkan/WSIWindows.cpp
    platform/vulkan/WSIXcb.cpp
    rendering_api/rendering_api.c
    utils/eglUtils.cpp
    utils/eglLogger.cpp
)

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
        message(FATAL_ERROR "Implement EGL for your platform")
    endif()
endif()


include_directories(${CMAKE_SOURCE_DIR}/EGL/source
                    ${CMAKE_SOURCE_DIR}/EGL/include
                    ${CMAKE_SOURCE_DIR}/GLES/include
                    ${Vulkan_INCLUDE_DIR}
                    ${CMAKE_INSTALL_FULL_INCLUDEDIR})

add_library(EGL SHARED ${SOURCES})

if(WIN32)
    set_target_properties(EGL PROPERTIES PREFIX "lib"
                                         POSITION_INDEPENDENT_CODE ON
                                         LINK_FLAGS "/INCREMENTAL:NO"
                                         INSTALL_RPATH_USE_LINK_PATH TRUE
                                         INSTALL_RPATH "\$ORIGIN")
else()
    set_target_properties(EGL PROPERTIES POSITION_INDEPENDENT_CODE ON
                                         LINK_FLAGS "-Wl,-Bsymbolic"
                                         INSTALL_RPATH_USE_LINK_PATH TRUE
                                         INSTALL_RPATH "\$ORIGIN")
endif()



target_link_libraries(EGL ${LIBS})

if(WIN32)
    install(TARGETS EGL RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
else()
    install(TARGETS EGL LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

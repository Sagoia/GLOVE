message(STATUS "  Building graphics-engine library")

# Create GRAPHICS_ENGINE Lib
set(GRAPHICS_ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/asset_manager/shaderManager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/asset_manager/tgaLoader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/framebufferobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/rendering.c
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shading.c
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/viewport.c
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph/camera.c
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph/mesh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/profiler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/smoke_tests.c
)
set(GRAPHICS_ENGINE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/asset_manager/shaderManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glcore/abstract.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glcore/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/framebufferobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/rendering.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaderprogram.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shading.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/viewport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph/camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph/mesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/linmath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/profiler.h
)


if(WIN32 OR APPLE)
    set(LIBS
        ${LIBS}
        EGLUT
    )
    add_library(GRAPHICS_ENGINE STATIC ${GRAPHICS_ENGINE_HEADERS} ${GRAPHICS_ENGINE_SOURCES})
    target_link_libraries(GRAPHICS_ENGINE ${LIBS})
    if (APPLE)
        set_target_properties(GRAPHICS_ENGINE PROPERTIES  XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                                          XCODE_ATTRIBUTE_VALID_ARCHS "x86_64")
    endif()
else()
    add_library(GRAPHICS_ENGINE SHARED ${GRAPHICS_ENGINE_SOURCES})
    target_link_libraries(GRAPHICS_ENGINE EGLUT GL m)
endif()

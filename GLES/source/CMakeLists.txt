message(STATUS "  Building GLESv2 Library")

# Make sure CXX flags are in sync with libglslang.
# By default, GLOVE does not add support to AMD or NV.
# Further, ENABLE_OPT is also turned off by default.

if(WIN32)
    add_definitions(-DGLSLANG_OSINCLUDE_WIN32)
else()
    add_definitions(-DGLSLANG_OSINCLUDE_UNIX)
endif()
add_definitions(-DENABLE_HLSL)
add_definitions(-DENABLE_OPT=0)

# Sets the SOURCES variable to contain all the source files needed by
# GLESv2 shared lib to be built.
set(SOURCES
    api/gl.cpp
    api/eglInterface.cpp
    context/context.cpp
    context/contextBufferObject
    context/contextFrameBuffer.cpp
    context/contextRenderBuffer.cpp
    context/contextRendering.cpp
    context/contextShader.cpp
    context/contextShaderProgram.cpp
    context/contextSpecialFunctions.cpp
    context/contextStateFragmentOperations.cpp
    context/contextStateFramebufferOperations.cpp
    context/contextStateManager.cpp
    context/contextStatePixelOperations.cpp
    context/contextStateQueries.cpp
    context/contextStateRasterization.cpp
    context/contextStateViewportTransformation.cpp
    context/contextTexture.cpp
    context/contextUtilities.cpp
    context/contextVertexAttributes.cpp
    glslang/glslangCompiler.cpp
    glslang/glslangLinker.cpp
    glslang/glslangIoMapResolver.cpp
    glslang/glslangShaderCompiler.cpp
    glslang/glslangUtils.cpp
    glslang/shaderConverter.cpp
    glslang/FixSampler.cpp
    resources/attachment.cpp
    resources/bufferObject.cpp
    resources/framebuffer.cpp
    resources/genericVertexAttribute.cpp
    resources/refObject.cpp
    resources/resourceManager.cpp
    resources/renderbuffer.cpp
    resources/shader.cpp
    resources/shaderProgram.cpp
    resources/shaderReflection.cpp
    resources/shaderResourceInterface.cpp
    resources/texture.cpp
    resources/rect.cpp
    resources/sampler.cpp
    resources/screenSpacePass.cpp
    state/stateManager.cpp
    state/stateActiveObjects.cpp
    state/stateInputAssembly.cpp
    state/stateFragmentOperations.cpp
    state/stateFramebufferOperations.cpp
    state/statePixelStorage.cpp
    state/stateRasterization.cpp
    state/stateHintAspects.cpp
    state/stateViewportTransformation.cpp
    utils/GlToVkConverter.cpp
    utils/parser_helpers.cpp
    utils/VkToGlConverter.cpp
    utils/glLogger.cpp
    utils/glUtils.cpp
    utils/cacheManager.cpp
    utils/Twine.cpp
    utils/Text.cpp
    vulkan/commandBufferManager.cpp
    vulkan/commandBufferPool.cpp
    vulkan/clearPass.cpp
    vulkan/renderPass.cpp
    vulkan/buffer.cpp
    vulkan/memory.cpp
    vulkan/sampler.cpp
    vulkan/image.cpp
    vulkan/imageView.cpp
    vulkan/pipeline.cpp
    vulkan/pipelineCache.cpp
    vulkan/framebuffer.cpp
    vulkan/fence.cpp
    vulkan/context.cpp
    vulkan/utils.cpp
)

set(HEADERS
    api/glFunctions.h
    context/context.h
    glslang/glslangCompiler.h
    glslang/glslangLinker.h
    glslang/glslangIoMapResolver.h
    glslang/glslangShaderCompiler.h
    glslang/shaderConverter.h
    resources/attachment.h
    resources/bufferObject.h
    resources/framebuffer.h
    resources/genericVertexAttribute.h
    resources/resourceManager.h
    resources/renderbuffer.h
    resources/shader.h
    resources/shaderCompiler.h
    resources/shaderProgram.h
    resources/shaderReflection.h
    resources/shaderResourceInterface.h
    resources/texture.h
    resources/rect.h
    resources/sampler.h
    resources/screenSpacePass.h
    state/stateManager.h
    state/stateActiveObjects.h
    state/stateInputAssembly.h
    state/stateFragmentOperations.h
    state/stateFramebufferOperations.h
    state/statePixelStorage.h
    state/stateRasterization.h
    state/stateHintAspects.h
    state/stateViewportTransformation.h
    utils/color.hpp
    utils/globals.h
    utils/glsl_types.h
    utils/GlToVkConverter.h
    utils/parser_helpers.h
    utils/VkToGlConverter.h
    utils/glLogger.h
    utils/glLoggerImpl.h
    utils/glUtils.h
    utils/cacheManager.h
    vulkan/commandBufferManager.h
    vulkan/commandBufferPool.h
    vulkan/clearPass.h
    vulkan/renderPass.h
    vulkan/buffer.h
    vulkan/memory.h
    vulkan/sampler.h
    vulkan/image.h
    vulkan/imageView.h
    vulkan/pipeline.h
    vulkan/pipelineCache.h
    vulkan/framebuffer.h
    vulkan/fence.h
    vulkan/context.h
    vulkan/utils.h
)

set(OTHER_HEADERS
    ${CMAKE_SOURCE_DIR}/GLES/source/utils/arrays.hpp
)
# For Microsoft Windows add the module-definition file to SOURCES (necessary to export API functions)
if(WIN32)
    set(SOURCES ${SOURCES} api/gl.def)
endif()

# Sets the LIBS variable to contain the libraries that our GLESv2 shared library
# will need to be linked against to.


if(USE_SURFACE STREQUAL "WAYLAND")
    include_directories(${Wayland_INCLUDE_DIRS})
    if(Wayland_FOUND)
        set(LIBS ${LIBS} wayland-client)
     else()
         message(FATAL_ERROR "Could not find Wayland LIBRARIES")
     endif()
endif()

# Where the .h files included in the source files can be found.
include_directories(${GLES_PATH}/source
                    ${GLES_PATH}/include
                    ${EGL_PATH}/include
                    ${GLSLANG_PATH}/include
                    ${Vulkan_INCLUDE_DIR}
                    ${CMAKE_INSTALL_FULL_INCLUDEDIR})

link_directories(${GLSLANG_PATH}/lib)

# Ensure we re-link if our external libraries change.
if(WIN32)

    set(LIBS
        ${Vulkan_LIBRARY}
        debug ${GLSLANG_PATH}/lib/glslangd.lib
        optimized ${GLSLANG_PATH}/lib/glslang.lib
        debug ${GLSLANG_PATH}/lib/SPIRVd.lib
        optimized ${GLSLANG_PATH}/lib/SPIRV.lib
        debug ${GLSLANG_PATH}/lib/HLSLd.lib
        optimized ${GLSLANG_PATH}/lib/HLSL.lib
        debug ${GLSLANG_PATH}/lib/OGLCompilerd.lib
        optimized ${GLSLANG_PATH}/lib/OGLCompiler.lib
        debug ${GLSLANG_PATH}/lib/OSDependentd.lib
        optimized ${GLSLANG_PATH}/lib/OSDependent.lib
    )

    add_library(GLESv2 SHARED ${SOURCES})

    set_target_properties(GLESv2 PROPERTIES POSITION_INDEPENDENT_CODE ON
                                            LINK_FLAGS "/INCREMENTAL:NO"
                                            INSTALL_RPATH_USE_LINK_PATH TRUE
                                            INSTALL_RPATH "\$ORIGIN")

else()

    set(LIBS
        ${Vulkan_LIBRARY}
        glslang
        SPIRV
        HLSL
        OGLCompiler
        OSDependent
        pthread
        m
    )

    add_library(glslang STATIC IMPORTED)
    set_target_properties(glslang PROPERTIES IMPORTED_LOCATION ${GLSLANG_PATH}/lib/libglslang.a)

    add_library(SPIRV STATIC IMPORTED)
    set_target_properties(SPIRV PROPERTIES IMPORTED_LOCATION ${GLSLANG_PATH}/lib/libSPIRV.a)

    add_library(HLSL STATIC IMPORTED)
    set_target_properties(HLSL PROPERTIES IMPORTED_LOCATION ${GLSLANG_PATH}/lib/libHLSL.a)

    add_library(OGLCompiler STATIC IMPORTED)
    set_target_properties(OGLCompiler PROPERTIES IMPORTED_LOCATION ${GLSLANG_PATH}/lib/libOGLCompiler.a)

    add_library(OSDependent STATIC IMPORTED)
    set_target_properties(OSDependent PROPERTIES IMPORTED_LOCATION ${GLSLANG_PATH}/lib/libOSDependent.a)

    if(APPLE)
	add_library(GLESv2 SHARED ${OTHER_HEADERS} ${HEADERS} ${SOURCES})

        set_target_properties(GLESv2 PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                                XCODE_ATTRIBUTE_VALID_ARCHS "x86_64"
                                                POSITION_INDEPENDENT_CODE ON
					        MACOSX_RPATH TRUE
					        SKIP_BUILD_RPATH FALSE
                                                LINK_FLAGS "-w"
                                                INSTALL_RPATH_USE_LINK_PATH TRUE
                                                INSTALL_RPATH "\$ORIGIN")
    else()
        add_library(GLESv2 SHARED ${SOURCES})
        set_target_properties(GLESv2 PROPERTIES POSITION_INDEPENDENT_CODE ON
                                                LINK_FLAGS "-Wl,-Bsymbolic"
                                                INSTALL_RPATH_USE_LINK_PATH TRUE
                                                INSTALL_RPATH "\$ORIGIN")
    endif()

endif()


target_link_libraries(GLESv2 ${LIBS})

if(WIN32)
    install(TARGETS GLESv2 RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
else()
    install(TARGETS GLESv2 LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

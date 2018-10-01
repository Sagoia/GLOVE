
LOCAL_PATH := $(abspath $(call my-dir))
SRC_PATH := $(LOCAL_PATH)/../..
GLSLANG_PATH := $(SRC_PATH)/External/glslang

PRE_BUILD_FOLDER := ../obj/build

# Pull in prebuilt libraries
include $(CLEAR_VARS)
LOCAL_MODULE := glslang-prebuilt
LOCAL_SRC_FILES := $(PRE_BUILD_FOLDER)/libglslang.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := OGLCompiler-prebuilt
LOCAL_SRC_FILES := $(PRE_BUILD_FOLDER)/libOGLCompiler.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := OSDependent-prebuilt
LOCAL_SRC_FILES := $(PRE_BUILD_FOLDER)/libOSDependent.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := HLSL-prebuilt
LOCAL_SRC_FILES := $(PRE_BUILD_FOLDER)/libHLSL.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := SPIRV-prebuilt
LOCAL_SRC_FILES := $(PRE_BUILD_FOLDER)/libSPIRV.a
include $(PREBUILT_STATIC_LIBRARY)

# Build libGLESv2.so
include $(CLEAR_VARS)
LOCAL_MODULE := libGLESv2_GLOVE
LOCAL_SRC_FILES :=  $(SRC_PATH)/GLES/source/api/gl.cpp \
                    $(SRC_PATH)/GLES/source/api/eglInterface.cpp \
                    $(SRC_PATH)/GLES/source/context/context.cpp \
                    $(SRC_PATH)/GLES/source/context/contextBufferObject.cpp \
                    $(SRC_PATH)/GLES/source/context/contextFrameBuffer.cpp \
                    $(SRC_PATH)/GLES/source/context/contextRenderBuffer.cpp \
                    $(SRC_PATH)/GLES/source/context/contextRendering.cpp \
                    $(SRC_PATH)/GLES/source/context/contextShader.cpp \
                    $(SRC_PATH)/GLES/source/context/contextShaderProgram.cpp \
                    $(SRC_PATH)/GLES/source/context/contextSpecialFunctions.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStateFragmentOperations.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStateFramebufferOperations.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStateManager.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStatePixelOperations.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStateQueries.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStateRasterization.cpp \
                    $(SRC_PATH)/GLES/source/context/contextStateViewportTransformation.cpp \
                    $(SRC_PATH)/GLES/source/context/contextTexture.cpp \
                    $(SRC_PATH)/GLES/source/context/contextUtilities.cpp \
                    $(SRC_PATH)/GLES/source/context/contextVertexAttributes.cpp \
                    $(SRC_PATH)/GLES/source/glslang/glslangCompiler.cpp \
                    $(SRC_PATH)/GLES/source/glslang/glslangLinker.cpp \
                    $(SRC_PATH)/GLES/source/glslang/glslangIoMapResolver.cpp \
                    $(SRC_PATH)/GLES/source/glslang/glslangShaderCompiler.cpp \
                    $(SRC_PATH)/GLES/source/glslang/shaderConverter.cpp \
                    $(SRC_PATH)/GLES/source/resources/attachment.cpp \
                    $(SRC_PATH)/GLES/source/resources/bufferObject.cpp \
                    $(SRC_PATH)/GLES/source/resources/framebuffer.cpp \
                    $(SRC_PATH)/GLES/source/resources/genericVertexAttribute.cpp \
                    $(SRC_PATH)/GLES/source/resources/resourceManager.cpp \
                    $(SRC_PATH)/GLES/source/resources/renderbuffer.cpp \
                    $(SRC_PATH)/GLES/source/resources/shader.cpp \
                    $(SRC_PATH)/GLES/source/resources/shaderProgram.cpp \
                    $(SRC_PATH)/GLES/source/resources/shaderReflection.cpp \
                    $(SRC_PATH)/GLES/source/resources/shaderResourceInterface.cpp \
                    $(SRC_PATH)/GLES/source/resources/texture.cpp \
                    $(SRC_PATH)/GLES/source/resources/rect.cpp \
                    $(SRC_PATH)/GLES/source/resources/sampler.cpp \
                    $(SRC_PATH)/GLES/source/state/stateManager.cpp \
                    $(SRC_PATH)/GLES/source/state/stateActiveObjects.cpp \
                    $(SRC_PATH)/GLES/source/state/stateInputAssembly.cpp \
                    $(SRC_PATH)/GLES/source/state/stateFragmentOperations.cpp \
                    $(SRC_PATH)/GLES/source/state/stateFramebufferOperations.cpp \
                    $(SRC_PATH)/GLES/source/state/statePixelStorage.cpp \
                    $(SRC_PATH)/GLES/source/state/stateRasterization.cpp \
                    $(SRC_PATH)/GLES/source/state/stateHintAspects.cpp \
                    $(SRC_PATH)/GLES/source/state/stateViewportTransformation.cpp \
                    $(SRC_PATH)/GLES/source/utils/GlToVkConverter.cpp \
                    $(SRC_PATH)/GLES/source/utils/parser_helpers.cpp \
                    $(SRC_PATH)/GLES/source/utils/VkToGlConverter.cpp \
                    $(SRC_PATH)/GLES/source/utils/glLogger.cpp \
                    $(SRC_PATH)/GLES/source/utils/glUtils.cpp \
                    $(SRC_PATH)/GLES/source/utils/cacheManager.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/cbManager.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/clearPass.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/renderPass.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/buffer.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/memory.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/sampler.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/image.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/imageView.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/pipeline.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/pipelineCache.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/framebuffer.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/context.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/utils.cpp \
                    $(SRC_PATH)/GLES/source/vulkan/fence.cpp

LOCAL_C_INCLUDES := $(SRC_PATH)/GLES/source \
                    $(SRC_PATH)/GLES/include \
                    $(SRC_PATH)/EGL/include \
                    $(GLSLANG_PATH)/include \
                    $(LOADER_PATH)/include \
                    /usr/include/android

LOCAL_STATIC_LIBRARIES := \
                    glslang-prebuilt \
                    OSDependent-prebuilt \
                    OGLCompiler-prebuilt \
                    SPIRV-prebuilt \
                    HLSL-prebuilt

LOCAL_LDLIBS   += -llog \
                  -lm \
                  -lvulkan

LOCAL_CXXFLAGS += -pthread \
                  -DVK_PROTOTYPES \
                  -DVK_USE_PLATFORM_ANDROID_KHR \
                  -Wall \
                  -Wextra \
                  -Winline \
                  -Wuninitialized \
                  -Wno-mismatched-tags \
                  -frtti \
                  -DHAVE_PTHREADS

LOCAL_CFLAGS +=   -DVK_USE_PLATFORM_ANDROID_KHR

include $(BUILD_SHARED_LIBRARY)

# Build libEGL.so
include $(CLEAR_VARS)
LOCAL_MODULE := libEGL_GLOVE
LOCAL_SRC_FILES := $(SRC_PATH)/EGL/source/api/eglContext.cpp \
                   $(SRC_PATH)/EGL/source/api/eglConfig.cpp \
                   $(SRC_PATH)/EGL/source/api/egl.cpp \
                   $(SRC_PATH)/EGL/source/api/eglSurface.cpp \
                   $(SRC_PATH)/EGL/source/display/displayDriver.cpp \
                   $(SRC_PATH)/EGL/source/display/displayDriversContainer.cpp \
                   $(SRC_PATH)/EGL/source/thread/renderingThread.cpp \
                   $(SRC_PATH)/EGL/source/platform/platformFactory.cpp \
                   $(SRC_PATH)/EGL/source/platform/vulkan/WSIPlaneDisplay.cpp \
                   $(SRC_PATH)/EGL/source/platform/vulkan/WSIAndroid.cpp \
                   $(SRC_PATH)/EGL/source/platform/vulkan/vulkanWindowInterface.cpp \
                   $(SRC_PATH)/EGL/source/platform/vulkan/vulkanWSI.cpp \
                   $(SRC_PATH)/EGL/source/platform/vulkan/vulkanAPI.cpp \
                   $(SRC_PATH)/EGL/source/platform/vulkan/vulkanResources.cpp \
                   $(SRC_PATH)/EGL/source/rendering_api/rendering_api.c \
                   $(SRC_PATH)/EGL/source/utils/eglLogger.cpp

LOCAL_C_INCLUDES := $(SRC_PATH)/EGL/source \
                    $(SRC_PATH)/EGL/include \
                    $(SRC_PATH)/GLES/include \
                    /usr/include/android

LOCAL_SHARED_LIBRARIES :=  libGLESv2_GLOVE
LOCAL_LDLIBS           +=  -llog -lz -lvulkan -landroid
LOCAL_CXXFLAGS += -pthread \
                  -DVK_PROTOTYPES \
                  -DVK_USE_PLATFORM_ANDROID_KHR \
                  -Wall \
                  -Wextra \
                  -Winline \
                  -Wuninitialized \
                  -Wno-mismatched-tags \
                  -frtti \
                  -Wno-unused-private-field \
                  -DHAVE_PTHREADS

LOCAL_CFLAGS +=   -DVK_USE_PLATFORM_ANDROID_KHR

include $(BUILD_SHARED_LIBRARY)

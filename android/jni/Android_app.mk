
LOCAL_PATH :=                           $(abspath $(call my-dir))

include $(CLEAR_VARS)
SRC_PATH :=                             $(LOCAL_PATH)/../../Demos/demos
LIB_PATH :=                             $(LOCAL_PATH)/../obj/build
EGLUT_PATH :=                           $(LOCAL_PATH)/../../Demos/eglut
EGL_INC_PATH :=                         $(LOCAL_PATH)../../EGL/include

LOCAL_MODULE :=                         libEGL-prebuilt
LOCAL_SRC_FILES :=                      $(LIB_PATH)/glove/libEGL_GLOVE.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=                         libGLESv2-prebuilt
LOCAL_SRC_FILES :=                      $(LIB_PATH)/glove/libGLESv2_GLOVE.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE :=                         GLOVE_Demo

LOCAL_SRC_FILES :=                      $(LOCAL_PATH)/../android_app.c \
                                        $(LOCAL_PATH)/../es2gears.c

LOCAL_C_INCLUDES +=                     $(LOCAL_PATH)/.. \
                                        $(SRC_PATH)
LOCAL_PACKAGE_NAME := LocalPackage
LOCAL_CXXFLAGS +=                       -DVK_USE_PLATFORM_ANDROID_KHR
                                        -std=c++11 \
                                        -fexceptions \
                                        -Wall \
                                        -Wextra \
                                        -Wno-unused-parameter \
                                        -Wno-unused-function \
                                        -DVK_NO_PROTOTYPES \
                                        -DGLM_FORCE_RADIANS

LOCAL_CFLAGS +=                         -std=c99 \
                                        -Wall \
                                        -Wextra \
                                        -Wno-unused-parameter \
                                        -Wno-unused-function \
                                        -Wno-format-security \
                                        -DVK_NO_PROTOTYPES \
                                        -DGLM_FORCE_RADIANS \
                                        -DGL_GLEXT_PROTOTYPES \
                                        -DVK_USE_PLATFORM_ANDROID_KHR \
                                        -DWIDTH=800 \
                                        -DHEIGHT=552 \
                                        -DKILL_APP_PERIOD=10 \
                                        -DINFO_DISPLAY

LOCAL_WHOLE_STATIC_LIBRARIES :=         android_native_app_glue
LOCAL_SHARED_LIBRARIES :=               libGLESv2-prebuilt libEGL-prebuilt
LOCAL_LDLIBS :=                         -llog -landroid

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)

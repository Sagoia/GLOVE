APP_ABI := arm64-v8a #x86
APP_PLATFORM := android-24
APP_STL := c++_static
APP_CPPFLAGS += -DVK_PROTOTYPES \
                -DVK_USE_PLATFORM_ANDROID_KHR \
                -Wall \
                -Wextra \
                -Winline \
                -Wuninitialized \
                -Wno-mismatched-tags \
                -frtti \
                -DHAVE_PTHREADS \
                -DNDEBUG

APP_CFLAGS +=   -DVK_USE_PLATFORM_ANDROID_KHR \
                -DNDEBUG

NDK_TOOLCHAIN_VERSION := clang


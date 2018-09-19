APP_ABI := arm64-v8a  #x86
APP_PLATFORM := android-24
APP_STL := c++_static
APP_CPPFLAGS += -Wall \
                -Wno-unused-parameter \
                -Wno-reorder

NDK_TOOLCHAIN_VERSION := clang

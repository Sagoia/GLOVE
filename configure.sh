#!/usr/bin/env bash

set -e

BASEDIR=$PWD
BUILD_TYPE=Release
VULKAN_LIBRARY=""
VULKAN_INCLUDE_PATH=""
TRACE_BUILD=OFF
TOOLCHAIN_FILE=""
SYSROOT=""
C_FLAGS=""
CXX_FLAGS=""
INSTALL_PREFIX="/usr/local"
BUILD_FOLDER=build
CROSS_COMPILATION_ARM=false
USE_SURFACE=XCB

#########################################################
####################### GLOVE ###########################
#########################################################
function buildGlove() {
    mkdir -p $BUILD_FOLDER
    cd $BUILD_FOLDER

    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
          -DVULKAN_LIBRARY=$VULKAN_LIBRARY \
          -DUSE_SURFACE=$USE_SURFACE \
          -DVULKAN_INCLUDE_PATH=$VULKAN_INCLUDE_PATH \
          -DTRACE_BUILD=$TRACE_BUILD \
          -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
          -DCMAKE_SYSROOT=$SYSROOT \
          -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
          -DCMAKE_C_FLAGS=$C_FLAGS \
          -DCMAKE_CXX_FLAGS=$CXX_FLAGS \
          --no-warn-unused-cli ..

    cd ..
}

if [ "$#" == 0 ]; then
    echo "Building GLOVE"
else
    # Parse options
    while [[ $# > 0 ]]
    do
        option="$1"

        case $option in
            # option to cross compile
            -a|--arm-compile)
                CROSS_COMPILATION_ARM=true
                BUILD_FOLDER=cross_${BUILD_FOLDER}
                TOOLCHAIN_FILE=$BASEDIR/CMake/toolchain-arm.cmake
                INSTALL_PREFIX=""
                echo "Cross compiling for ARM"
                ;;
            # option to build in Debug mode
            -d|--debug)
                BUILD_TYPE=Debug
                BUILD_FOLDER=${BUILD_FOLDER}_debug
                echo "Building in Debug mode"
                ;;
            # option to handle warnings as errors
            -e|--werror)
                C_FLAGS="-Werror"
                CXX_FLAGS="-Werror"
                echo "Turn all compilation warnings into errors"
                ;;
            # option to set install path
            -i|--install-prefix)
                shift
                INSTALL_PREFIX=$1
                if [ ! "${INSTALL_PREFIX:0:1}" == "/" ]; then
                    INSTALL_PREFIX=$BASEDIR/$1
                fi
                echo "Setting installation prefix to $INSTALL_PREFIX"
                ;;
            # option to set sysroot
            -s|--sysroot)
                shift
                SYSROOT=$1
                echo "Setting SYSROOT"
                ;;
            # option to activate GL & Vulkan logs
            -t|--trace-build)
                TRACE_BUILD=ON
                echo "Activating logs"
                ;;
            # option to set Vulkan include path
            -u|--vulkan-include-path)
                shift
                VULKAN_INCLUDE_PATH=$1
                echo "Setting Vulkan include path"
                ;;
            # option to set Vulkan loader
            -v|--vulkan-loader)
                shift
                VULKAN_LIBRARY=$1
                echo "Setting Vulkan loader"
                ;;
            # option to set surface
            -f|--use-surface)
                shift
                if [ $1 == "XCB" || [ $1 == "WAYLAND" ]] || [ $1 == "ANDROID" ] || [ $1 == "NATIVE" ]; then
                    USE_SURFACE=$1
                    echo "Setting windowing system to $USE_SURFACE"
                else
                    echo "Wrong surface argument $1 provided (Options are: XCB, ANDROID and NATIVE). Using $USE_SURFACE."
                fi
                ;;
            *)
                echo "Unrecognized option: $option"
                echo "Try the following:"
                echo " -a | --arm-compile                   # cross build for ARM platform (default OFF)"
                echo " -d | --debug                         # build in Debug mode (default Release)"
                echo " -e | --werror                        # handle warnings as errors (default OFF)"
                echo " -f | --use-surface                   # set windowing system (Options: XCB, ANDROID, NATIVE) (default XCB)"
                echo " -i | --install-prefix      (dir)     # set custom installation prefix path"
                echo " -s | --sysroot             (dir)     # set sysroot for cross compilation"
                echo " -t | --trace-build                   # activate logs (default OFF)"
                echo " -u | --vulkan-include-path (dir)     # set custom Vulkan include path"
                echo " -v | --vulkan-loader       (lib)     # set custom Vulkan loader library"
                exit 1
                ;;
        esac
        shift
    done
fi

if [ ! -d "$BASEDIR/External/glslang" ] || [ ! -d "$BASEDIR/External/glslang/.git" ]; then
    echo -e "\e[0;31mExternal sources are missing. Run './update_external_sources.sh'.\e[0m"
    exit 1
fi

if [ -n "$VULKAN_LIBRARY" ] && { [ ! -e $VULKAN_LIBRARY ] || echo $VULKAN_LIBRARY | grep -q -v ".so" ; }; then
    echo -e "\e[0;31mCannot find Vulkan loader '$VULKAN_LIBRARY'.\e[0m"
    exit 1
fi

if [ -n "$VULKAN_INCLUDE_PATH" ] && [ ! -d $VULKAN_INCLUDE_PATH ]; then
    echo -e "\e[0;31mCannot find Vulkan include path '$VULKAN_INCLUDE_PATH'.\e[0m"
    exit 1
fi

if [ ! -d $INSTALL_PREFIX ]; then
    echo -e "\e[0;31mCannot find '$INSTALL_PREFIX' path to install the libraries.\e[0m"
    exit 1
fi

if [ $CROSS_COMPILATION_ARM == "true" ]; then
    if [ -z "$SYSROOT" ] || [ ! -d $SYSROOT ]; then
        echo -e "\e[0;31mCannot find '$SYSROOT' path for sysroot.\e[0m"
        exit 1
    fi
    if [ -z "$INSTALL_PREFIX" ]; then
        echo -e "\e[0;31mSet custom installation path (use '-i' option).\e[0m"
        exit 1
    fi
    if [ ! -d "$BASEDIR/External/glslang" ] || [ ! -d "$BASEDIR/External/googletest/cross_build" ]; then
        echo -e "\e[0;31mExternal sources are not cross-compiled. Run './update_external_sources.sh -s $SYSROOT -i $INSTALL_PREFIX'.\e[0m"
        exit 1
    fi
fi

buildGlove

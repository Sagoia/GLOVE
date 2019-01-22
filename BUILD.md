# Building GLOVE for Linux

The building process has been tested on Ubuntu 16.04 and 18.04.

## Configure Building

GLOVE building can be configured according to the options listed in the following table:

```
./configure.sh [-options]
```

| **Option** | **Default** | **Description** |
| --- | --- | --- |
| -a \| --arm-compile | _OFF_ | _Enable cross building for ARM platform_ |
| -d \| --debug | _OFF_ | _Enable building Debug mode_ |
| -e \| --werror | _OFF_ | _Turn all compilation warnings into errors_ |
| -f \| --use-surface | _XCB_ |  _Sets the windowing system<br>(Options: XCB, ANDROID, NATIVE)_ |
| -i \| --install-prefix (dir) | _System Installation Prefix (/usr/local)_ | _Set custom installation prefix path_ |
| -s \| --sysroot (dir) | _-_ | _Set sysroot for cross compilation_ |
| -t \| --trace-build | _OFF_ | _Enable logs_ |
| -u \| --vulkan-include-path (dir) | _System Include Path_ | _Set custom Vulkan include path_ |
| -v \| --vulkan-loader (lib) | _System Vulkan Loader_ | _Set custom Vulkan loader library_ |


## Build Project

To build the Project:

```
make
```

## Install Project

To install all necessary files to system directories (superuser privilege might be required):

```
make install
```

## Uninstall

To uninstall the libraries from the system directories (superuser privilege might be required):

```
make uninstall
```

# Building GLOVE for Android

The building process has been tested on Android 7 and 8.

Building GLOVE for Android requires Java 8. An environmental variable must be set
```
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
```
Download Android Studio on Ubuntu at https://developer.android.com/studio/

Downgrade Android-SDK to version 25:
```
cd <android-sdk-dir>/
mv tools tools_back
wget http://dl.google.com/android/repository/tools_r25-linux.zip
unzip tools_r25-linux.zip
```
Required packages for Android building:
```
sudo apt-get install android-platform-build-headers xcb-proto android-platform-frameworks-native-headers android-platform-system-core-headers android-libcutils-dev ant
```
GLOVE building can be configured according to the options listed in the following table:

```
./android_build.sh  [-options]
```

| **Option** | **Default** | **Description** |
| --- | --- | --- |
| -d \| --debug | _OFF_ | _Enable building Debug mode_ |
| -t \| --trace-build | _OFF_ | _Enable logs_ |

The above process builds GLOVE for Android and generates an apk to be later installed on the Android device. See the installation process [here](Demos/README_demos.md).

# Building GLOVE for iOS

The building process has been tested on macOS Mojave.

```
cd /PATH/TO/GLOVE/ROOT
mkdir -p Build
cd Build
cmake .. -G Xcode \
        -DCMAKE_INSTALL_PREFIX=../Bin \
        -DENABLE_GLSLANG_BINARIES=OFF \
        -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake \
        -DIOS_PLATFORM=OS64 \
        -DVULKAN_LIBRARY=/PATH/TO/libMoltenVK.a \
        -DVULKAN_INCLUDE_PATH=/PATH/TO/MoltenVK/include \
        -DUSE_SURFACE=IOS 
```

GLOVE.xcodeproj is generated in GLOVE_ROOT/Build.

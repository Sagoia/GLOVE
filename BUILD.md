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
| -f \| --use-surface | _XCB_ |  _Sets the windowing system<br>(Options: XCB, WAYLAND, ANDROID, NATIVE, WINDOWS, MACOS)_ |
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

# Building GLOVE for Windows

The building process has been tested on Windows 10, using MS Visual Studio 2019.

## Create an MS Visual Studio Project

At first, you should create an MS Visual Studio Project by cloning GLOVE from github.
Afterwards, you should resolve the external dependencies, as described [here](README.md#external-repositories-dependencies)

## Configure Building

GLOVE building can be configured according to the options listed in the following table:

Select Project->CMake Settings

| **Option** | **Default** | **Description** |
| --- | --- | --- |
| -d \| --debug | _OFF_ | _Enable building Debug mode_ |
| -t \| --trace-build | _OFF_ | _Enable logs_ |

example of CMakeSettings json file

```
{
    "configurations": [
        {
            "name": "x64-Release",
            "generator": "Ninja",
            "configurationType": "Release",
            "inheritEnvironments": [ "msvc_x64_x64" ],
            "buildRoot": "${projectDir}\\out\\build\\${name}",
            "installRoot": "${projectDir}\\out\\install\\${name}",
            "cmakeCommandArgs": "",
            "buildCommandArgs": "-v",
            "ctestCommandArgs": "",
            "variables": [
                {
                    "name": "TRACE_BUILD",
                    "value": "false",
                    "type": "BOOL"
                }
            ]
        }
    ]
}
```

## Build Project

To build the Project, use MS Visual Studio GUI (Build->Build All)


# Building GLOVE for macOS

The building process has been tested on macOS Catalina (10.15).

## Configure Building

GLOVE building can be configured according to the options listed in the following table:

```
./configure.sh [-options]
```

| **Option** | **Default** | **Description** |
| --- | --- | --- |
| -d \| --debug | _OFF_ | _Enable building Debug mode_ |
| -e \| --werror | _OFF_ | _Turn all compilation warnings into errors_ |
| -f \| --use-surface | _XCB_ |  _Sets the windowing system<br>_ **(MACOS option must be set for macOS)** |
| -i \| --install-prefix (dir) | _System Installation Prefix (/usr/local)_ | _Set custom installation prefix path_ |
| -t \| --trace-build | _OFF_ | _Enable logs_ |
| -u \| --vulkan-include-path (dir) | _System Include Path_ | _Set custom Vulkan include path_ |
| -v \| --vulkan-loader (lib) | _System Vulkan Loader_ | _Set custom Vulkan loader library_ |


In macOS, the configure.sh script calls CMake with "-G Xcode" argument, thus preparing all necessary files for opening GLOVE in Xcode. Build files are stored in "build" folder.

## Build Project

Open GLOVE.xcodeproj (\<GLOVE_root\>/build/GLOVE.xcodeproj) with Xcode and build the Project from the tool (Product | Build). 

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

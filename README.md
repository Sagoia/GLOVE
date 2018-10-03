# GLOVE - GL Over Vulkan

![GLOVE functionality](Docs/Images/GLOVEfunction.jpg)

GLOVE (GL Over Vulkan) is a cross-platform software library that acts as an intermediate layer between an OpenGL ES application and Vulkan.

GLOVE is focused towards embedded systems and is comprised of OpenGL ES and EGL implementations, which translate at runtime all OpenGL ES / EGL calls & ESSL shaders to Vulkan commands &amp; SPIR-V shader respectively and finally relays them to the underlying Vulkan driver.

GLOVE has been designed towards facilitating developers to easily build and integrate new features, allowing at the same time its further extension, portability and interoperability. Currently, GLOVE supports [OpenGL ES 2.0](https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf) and [EGL 1.4](https://www.khronos.org/registry/EGL/specs/eglspec.1.4.pdf) on Linux and Android platforms, but the modular design can be easily extended to encompass implementations of other client APIs as well.

GLOVE is considered as a work-in-progress and is open-sourced under the LGPL v3 license through which it is provided as free software with unlimited use for educational and research purposes.

Future planned extensions of GLOVE include the support for OpenGL ES 3.x and OpenGL applications.

# Prerequisites

The minimum Vulkan loader version must be 1.0.24.

Optionally, GLOVE supports the **VK_KHR_maintenance1** extension, used for OpenGL to Vulkan Coordinates conversion (left handed to right handed coordinate system).

# Tested with the following configurations

GLOVE has been successfully tested with [GLOVE demos](Demos/README_demos.md) with the following configurations

| **GL version**  | **Graphics Card** | **Vulkan Driver** | **Vulkan API** | **OS** | **Windows Platform** | Status |
| --- | --- | --- | --- | --- | --- | --- |
| ES 2.0  | Intel Ivybridge Desktop             | Mesa 17.3.3          | 1.0.54 | Ubuntu 16.04  | XCB | success |
| ES 2.0  | Intel HD Graphics 530 (Skylake GT2) | Mesa 18.0.5        | 1.0.57 | Ubuntu 16.04  | XCB | success |
| ES 2.0  | Intel HD Graphics 630 (Kabylake GT2) | Mesa 18.0.5        | 1.0.61 | Ubuntu 16.04  | XCB | success |
| ES 2.0  | GeForce 940M                        | NVIDIA 396.51      | 1.1.70 | Ubuntu 16.04  | XCB | success |
| ES 2.0  | GeForce GTX 670                     | NVIDIA 396.54      | 1.1.70 | Ubuntu 18.04  | XCB | success |
| ES 2.0  | Mali-G71                            | ARM 482.381.3347   | 1.0.26 | Android 7.0   | Android | success |
| ES 2.0  | Mali-G71                            | ARM 485.111.1108   | 1.0.65 | Android 8.0   | Android | success |

# SW Design

You can find a short description on GLOVE SW design as well as "How To extend GLOVE" guidelines in [GLOVE Design Document](Docs/GLOVEDesignDocument.md)

# Contribution

GLOVE project is considered as work in progress, therefore contributions are more than welcome! Guidelines of how to contribute to GLOVE can be found [here](CONTRIBUTING.md)

# Installation Instructions

## Download the Repository

To create your local git repository:

```
git clone https://github.com/Think-Silicon/GLOVE.git
```

## Required Packages

To install all required packages:

```
sudo apt-get install git cmake extra-cmake-modules libvulkan-dev vulkan-utils build-essential libx11-xcb-dev
```

Optionally "mesa-vulkan-drivers" package is needed if no other Vulkan driver is available.
The compiler minimum version that this project is built with, is GCC 4.9.3, although earlier versions may work.

## External Repositories Dependencies

Khronos [glslang](https://github.com/KhronosGroup/glslang) repository is mandatory for compiling, validating and generating SPIR-V from ESSL shaders.

Google [googletest](https://github.com/google/googletest) repository is used for unit testing.

To get and build the above projects:

```
./update_external_sources.sh
```
# Building 

View the [Building instructions](BUILD.md).

# Known Issues

GLOVE is considered as work-in-progress, therefore there are known issues that have to be resolved or improved.

You can see a detailed list of issues in [Known Issues List](Docs/KnownIssues.md)

# Demos

GLOVE is accompanied by a demo SDK that contains fully commented, highly optimized C applications (accompanied by the ESSL shader source code). These demos demonstrate some simple rendering techniques with different geometry complexities, as they were designed with the restrictions of low-power embedded platforms in mind.

See details in [Demos README](Demos/README_demos.md)

# Benchmarking

GLOVE is aiming to take advantage of Vulkan in terms of performance. First results are very promising and major performance upgrades are in progress too. Instructions to use some available benchmarks for testing can be found in [Benchmarking README](Benchmarking/README_benchmarking.md)

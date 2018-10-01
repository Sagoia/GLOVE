
LOCAL_PATH := $(abspath $(call my-dir))
SRC_PATH := $(LOCAL_PATH)/../../External/glslang
GLSLANG_OS_FLAGS := -DGLSLANG_OSINCLUDE_UNIX

include $(CLEAR_VARS)
LOCAL_MODULE := SPIRV
LOCAL_CXXFLAGS := \
                -std=c++11 \
                -fno-exceptions \
                -fno-rtti \
                $(GLSLANG_OS_FLAGS)

LOCAL_EXPORT_C_INCLUDES:=$(SRC_PATH)
LOCAL_SRC_FILES:= \
                $(SRC_PATH)/SPIRV/GlslangToSpv.cpp \
                $(SRC_PATH)/SPIRV/InReadableOrder.cpp \
                $(SRC_PATH)/SPIRV/Logger.cpp \
                $(SRC_PATH)/SPIRV/SPVRemapper.cpp \
                $(SRC_PATH)/SPIRV/SpvBuilder.cpp \
                $(SRC_PATH)/SPIRV/disassemble.cpp \
                $(SRC_PATH)/SPIRV/doc.cpp

LOCAL_C_INCLUDES:=$(SRC_PATH) $(SRC_PATH)/glslang/SPIRV
LOCAL_EXPORT_C_INCLUDES:=$(SRC_PATH)/glslang/SPIRV
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := OSDependent
LOCAL_CXXFLAGS := \
                -std=c++11 \
                -fno-exceptions \
                -fno-rtti \
                $(GLSLANG_OS_FLAGS)

LOCAL_EXPORT_C_INCLUDES := $(SRC_PATH)
LOCAL_SRC_FILES:=$(SRC_PATH)/glslang/OSDependent/Unix/ossource.cpp
LOCAL_C_INCLUDES:=$(SRC_PATH) $(SRC_PATH)/glslang/OSDependent/Unix/
LOCAL_EXPORT_C_INCLUDES:=$(SRC_PATH)/glslang/OSDependent/Unix/
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=OGLCompiler
LOCAL_CXXFLAGS:=-std=c++11 \
                -fno-exceptions \
                -fno-rtti \
                $(GLSLANG_OS_FLAGS)

LOCAL_EXPORT_C_INCLUDES:=$(SRC_PATH)
LOCAL_SRC_FILES:=$(SRC_PATH)/OGLCompilersDLL/InitializeDll.cpp
LOCAL_C_INCLUDES:=$(SRC_PATH)/OGLCompiler
LOCAL_STATIC_LIBRARIES:=OSDependent
include $(BUILD_STATIC_LIBRARY)

# Build Glslang's HLSL parser library.
include $(CLEAR_VARS)
LOCAL_MODULE:=HLSL
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti -DENABLE_HLSL=1
LOCAL_SRC_FILES:= \
                $(SRC_PATH)/hlsl/hlslGrammar.cpp \
                $(SRC_PATH)/hlsl/hlslOpMap.cpp \
                $(SRC_PATH)/hlsl/hlslParseables.cpp \
                $(SRC_PATH)/hlsl/hlslParseHelper.cpp \
                $(SRC_PATH)/hlsl/hlslScanContext.cpp \
                $(SRC_PATH)/hlsl/hlslTokenStream.cpp \
                $(SRC_PATH)/hlsl/hlslAttributes.cpp

LOCAL_C_INCLUDES:=$(SRC_PATH) \
        $(SRC_PATH)/hlsl
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
GLSLANG_OUT_PATH=$(abspath $(TARGET_OUT))
LOCAL_MODULE:=glslang
LOCAL_CXXFLAGS:=-std=c++11 \
                -fno-exceptions \
                -fno-rtti \
                $(GLSLANG_OS_FLAGS)

LOCAL_EXPORT_C_INCLUDES:=$(SRC_PATH)
LOCAL_SRC_FILES:= \
                $(SRC_PATH)/glslang/GenericCodeGen/CodeGen.cpp \
                $(SRC_PATH)/glslang/GenericCodeGen/Link.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/attribute.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/Constant.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/glslang_tab.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/InfoSink.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/Initialize.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/Intermediate.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/intermOut.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/IntermTraverse.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/iomapper.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/limits.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/linkValidate.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/parseConst.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/ParseContextBase.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/ParseHelper.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/PoolAlloc.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/propagateNoContraction.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/reflection.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/RemoveTree.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/Scan.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/ShaderLang.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/SymbolTable.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/Versions.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/preprocessor/PpAtom.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/preprocessor/PpContext.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/preprocessor/Pp.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/preprocessor/PpScanner.cpp \
                $(SRC_PATH)/glslang/MachineIndependent/preprocessor/PpTokens.cpp \

LOCAL_C_INCLUDES:= \
                $(SRC_PATH) \
                $(SRC_PATH)/glslang/MachineIndependent \
                $(GLSLANG_OUT_PATH)

LOCAL_STATIC_LIBRARIES:= \
                OSDependent \
                OGLCompiler \
                SPIRV \
                HLSL

include $(BUILD_STATIC_LIBRARY)

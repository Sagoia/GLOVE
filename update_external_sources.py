import os
import sys
import getopt
import shutil
import multiprocessing
from subprocess import check_output

BASEDIR = os.path.dirname(os.path.realpath(__file__))
EXT_DIR = BASEDIR + os.path.sep + "External"
BUILD_FOLDER = "build"
INSTALL_PATH = ""
TOOLCHAIN_FILE = ""
SYSROOT = ""

GLSLANG_REPOSITORY="https://github.com/KhronosGroup/glslang.git"
GOOGLETEST_REPOSITORY="https://github.com/google/googletest.git"
NO_AMD_FLAG = "-DENABLE_AMD_EXTENSIONS=OFF"
NO_NV_FLAG  = "-DENABLE_NV_EXTENSIONS=OFF"
NO_OPT_FLAG = "-DENABLE_OPT=OFF"

def ReadLine(fileName):
    file = open(fileName, "rb")
    text = str(file.readline(), 'utf-8')
    file.close()
    return text

def Create(proj, repo, ver):
    projPath = EXT_DIR + os.path.sep + proj
    print("Creating local " + proj + " repository (" + projPath + ")")
    if os.path.exists(projPath):
        shutil.rmtree(projPath)
    os.mkdir(projPath)
    os.chdir(projPath)
    check_output(["git", "clone", repo, "."])
    check_output(["git", "checkout", str(ver)])
    os.chdir(BASEDIR)

def Update(proj, ver):
    projPath = EXT_DIR + os.path.sep + proj
    print("Updating " + proj + " repository ("+ projPath + ")")
    os.chdir(projPath)
    check_output(["git", "fetch", "--all"])
    check_output(["git", "checkout", "--force", ver])
    os.chdir(BASEDIR)

def Build(proj):
    projPath = EXT_DIR + os.path.sep + proj
    print("Building " + proj)

    install_prefix = INSTALL_PATH
    if install_prefix == "":
        install_prefix = projPath

    os.chdir(projPath)
    if os.path.exists(BUILD_FOLDER) == False:
        os.mkdir(BUILD_FOLDER)
    os.chdir(projPath + os.path.sep + BUILD_FOLDER)

    amd_flag = ""
    nv_flag  = ""
    opt_flag = ""
    if proj == "glslang":
        amd_flag = NO_AMD_FLAG
        nv_flag  = NO_NV_FLAG
        opt_flag = NO_OPT_FLAG

    if sys.platform == "linux2" or sys.platform == "linux":
        print(check_output(["cmake", "-DCMAKE_BUILD_TYPE=Release",
                                     "-DCMAKE_TOOLCHAIN_FILE=" + TOOLCHAIN_FILE,
                                     "-DCMAKE_SYSROOT=" + SYSROOT,
                                     "-DCMAKE_INSTALL_PREFIX=" + install_prefix,
                                     amd_flag,
                                     nv_flag,
                                     opt_flag,
                                     ".."]))

        os.system("make -j" + str(multiprocessing.cpu_count()))
        os.system("make install")
    elif sys.platform == "win32" :
        print(check_output(["cmake", "-G", "Visual Studio 16 2019",
                                     "-DCMAKE_INSTALL_PREFIX=" + install_prefix,
                                     amd_flag,
                                     nv_flag,
                                     opt_flag,
                                     ".."]))
        # Build both versions for Windows to comply with MSVC requirements
        os.system("cmake --build . --config Release --target install")
        os.system("cmake --build . --config Debug --target install")
    elif sys.platform == "darwin" :
        print(check_output(["cmake", "-G", "Xcode",
                                     "-DCMAKE_INSTALL_PREFIX=" + install_prefix,
                                     "-DENABLE_GLSLANG_BINARIES=OFF",
                                     amd_flag,
                                     nv_flag,
                                     opt_flag,
                                     ".."]))
        os.system("cmake --build . --config Release --target install")
    else :
        print(sys.platform + " not supported!")
        return

    os.chdir(BASEDIR)

def PrintUsage():
    print('Usage:')
    print('update_external_sources.py -i <install_path> -s <sysroot>')
    print(' -i | --install-path (dir)    # set custom installation path')
    print(' -s | --sysroot      (dir)    # set sysroot for cross compilation')

def ParseOptions():
    try:
        opts, args = getopt.getopt(sys.argv[1:],"hi:s:",["install_path=","sysroot="])
    except getopt.GetoptError:
        print('Unrecognized option!')
        PrintUsage()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            PrintUsage()
            sys.exit()
        elif opt in ("-i", "--install_path"):
            INSTALL_PATH = arg
        elif opt in ("-s", "--sysroot"):
            SYSROOT = arg
            BUILD_FOLDER = cross_build
            TOOLCHAIN_FILE = "BASEDIR" + CMake/toolchain-arm.cmake

## start script
os.chdir(BASEDIR)
ParseOptions()

GLSLANG_REVISION = ReadLine(EXT_DIR + os.path.sep + "glslang_revision")
GOOGLETEST_REVISION = ReadLine(EXT_DIR + os.path.sep + "googletest_revision")
print('glslang revision: ' + GLSLANG_REVISION)
print('googletest revision: ' + GOOGLETEST_REVISION)

if os.path.exists(EXT_DIR + os.path.sep + "glslang") == False or os.path.exists(EXT_DIR + os.path.sep + "glslang" + os.path.sep + ".git") == False :
    Create("glslang", GLSLANG_REPOSITORY, GLSLANG_REVISION)
else :
    Update("glslang", GLSLANG_REVISION)

Build("glslang")

if os.path.exists(EXT_DIR + os.path.sep + "googletest") == False or os.path.exists(EXT_DIR + os.path.sep + "googletest" + os.path.sep + ".git") == False :
    Create("googletest", GOOGLETEST_REPOSITORY, GOOGLETEST_REVISION)
else :
    Update("googletest", GOOGLETEST_REVISION)

Build("googletest")

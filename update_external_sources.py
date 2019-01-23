import os
import sys
import shutil
from subprocess import check_output

BASEDIR = os.path.dirname(os.path.realpath(__file__))
EXT_DIR = BASEDIR + os.path.sep + "External"
BUILD_FOLDER = "build"
INSTALL_PATH = ""
TOOLCHAIN_FILE = ""
SYSROOT = ""

GLSLANG_REPOSITORY="https://github.com/KhronosGroup/glslang.git"
GOOGLETEST_REPOSITORY="https://github.com/google/googletest.git"

def ReadLine(fileName):
    file = open(fileName, "rb")
    text = file.readline()
    file.close()
    return text

def Create(proj, repo, ver):
    projPath = EXT_DIR + os.path.sep + proj
    print "Creating local " + proj + " repository (" + projPath + ")"
    if os.path.exists(projPath):
        shutil.rmtree(projPath)
    os.mkdir(projPath)
    os.chdir(projPath)
    check_output(["git", "clone", repo, "."])
    check_output(["git", "checkout", ver])
    os.chdir(BASEDIR)

def Update(proj, ver):
    projPath = EXT_DIR + os.path.sep + proj
    print "Updating " + proj + " repository ("+ projPath + ")"
    os.chdir(projPath)
    check_output(["git", "fetch", "--all"])
    check_output(["git", "checkout", "--force", ver])
    os.chdir(BASEDIR)

def Build(proj):
    projPath = EXT_DIR + os.path.sep + proj
    print "Building " + proj
    install_prefix = INSTALL_PATH
    if install_prefix == "":
        install_prefix = projPath
    os.chdir(projPath)
    if os.path.exists(BUILD_FOLDER) == False:
        os.mkdir(BUILD_FOLDER)
    os.chdir(projPath + os.path.sep + BUILD_FOLDER)

    if sys.platform == "win32" :
        print check_output(["cmake", "..", "-G", "Visual Studio 14 2015 Win64", "-DCMAKE_INSTALL_PREFIX=" + install_prefix])
        os.system("cmake --build . --config Release --target install")
    elif sys.platform == "darwin" :
        print check_output(["cmake", "..", "-G", "Xcode", "-DCMAKE_INSTALL_PREFIX=" + install_prefix, "-DENABLE_GLSLANG_BINARIES=OFF", "-DCMAKE_TOOLCHAIN_FILE=../../../ios.toolchain.cmake", "-DIOS_PLATFORM=OS64"])
        os.system("cmake --build . --config Release --target install")
    else :
        print sys.platform + " not supported!"
        return

    os.chdir(BASEDIR)

os.chdir(BASEDIR)

GLSLANG_REVISION = ReadLine(EXT_DIR + os.path.sep + "glslang_revision")
GOOGLETEST_REVISION = ReadLine(EXT_DIR + os.path.sep + "googletest_revision")
print GLSLANG_REVISION
print GOOGLETEST_REVISION

if os.path.exists(EXT_DIR + os.path.sep + "glslang") == False or os.path.exists(EXT_DIR + os.path.sep + "glslang" + os.path.sep + ".git") == False :
    Create("glslang", GLSLANG_REPOSITORY, GLSLANG_REVISION)
else :
    Update("glslang", GLSLANG_REVISION)

Build("glslang")

#JAVA 8 is required
#sudo apt-get install  android-platform-build-headers xcb-proto android-platform-frameworks-native-headers android-platform-system-core-headers android-libcutils-dev

export ANDROID_HOME=~/Android/Sdk
export PATH=$ANDROID_HOME/tools:$PATH
export PATH=$ANDROID_HOME/tools/bin:$PATH
export PATH=$ANDROID_HOME/platform-tools:$PATH
export PATH=$PATH:$ANDROID_HOME/ndk-bundle/build/
#export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
DEBUG=0
TRACE=0

if [ "$#" == 0 ]; then
    echo "Building GLOVE for Android"
else
    # Parse options
    while [[ $# > 0 ]]
    do
        option="$1"

        case $option in
            # option to build in Debug mode
            -d|--debug)
                DEBUG=1
                echo "Building in Debug mode"
                ;;
            # option to activate GL logs
            -t|--trace-build)
                TRACE=1
                echo "Activating logs"
                ;;

            *)
                echo "Unrecognized option: $option"
                echo "Try the following:"
                echo " -d | --debug                         # build in Debug mode (default Release)"
                echo " -t | --trace-build                   # activate logs (default OFF)"
                exit 1
                ;;
        esac
        shift
    done
fi


ndk-build NDK_DEBUG=$DEBUG \
          NDK_PROJECT_PATH=./android \
          APP_BUILD_SCRIPT=./android/jni/Android_glslang.mk \
          NDK_APPLICATION_MK=./android/jni/Application.mk \
          TARGET_OUT=./android/obj/build \
          -j8
if [ $TRACE == 1 ];then
    ndk-build NDK_DEBUG=$DEBUG \
          NDK_PROJECT_PATH=./android \
          APP_BUILD_SCRIPT=./android/jni/Android_glove.mk \
          NDK_APPLICATION_MK=./android/jni/Application.mk \
          TARGET_OUT=./android/obj/build/glove \
          APP_CPPFLAGS=-DTRACE_BUILD \
          -j8 -B
else
    ndk-build NDK_DEBUG=$DEBUG \
          NDK_PROJECT_PATH=./android \
          APP_BUILD_SCRIPT=./android/jni/Android_glove.mk \
          NDK_APPLICATION_MK=./android/jni/Application.mk \
          TARGET_OUT=./android/obj/build/glove \
          -j8 -B
fi

ndk-build NDK_DEBUG=$DEBUG \
          NDK_PROJECT_PATH=./android \
          APP_BUILD_SCRIPT=./android/jni/Android_app.mk \
          NDK_APPLICATION_MK=./android/jni/Application.mk \
          TARGET_OUT=./android/obj/build \
          -j8

android update project -s -p ./android -t "android-24"

ant -file android/build.xml debug

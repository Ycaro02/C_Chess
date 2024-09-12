#!/bin/bash

NDK_PATH=~/android/android-ndk-r27b

export C_ARCHITECTURE=""
export CLANG_TOOLCHAIN=""
export C_TARGET=""
export SYSROOT_SUB_DIR=""
export ARM_DISABLE=""

export ANDROID_VERSION_COMPILE=21

# Check if we want arm or x86 version
if [ "$1" == "arm" ]; then
	export C_ARCHITECTURE="arm64-v8a"
	export CLANG_TOOLCHAIN="aarch64-linux-android-clang"
	export SYSROOT_SUB_DIR="aarch64-linux-android"
	export C_TARGET="aarch64-none-linux-android21"
	export ARM_DISABLE="-DSDL_DISABLE_IMMINTRIN_H"
elif [ "$1" == "x86" ]; then
	export C_ARCHITECTURE="x86_64"
	export CLANG_TOOLCHAIN="x86_64-clang"
	export SYSROOT_SUB_DIR="x86_64-linux-android"
	# export C_TARGET="i686-none-linux-android21"
	export C_TARGET="x86_64-none-linux-android21"
else
	echo "Usage: $0 [arm|x86]"
	exit 1
fi

echo "Architecture: ${C_ARCHITECTURE}"

function clean_sdl_lib {
    cd ../rsc/lib/SDL2
    ${NDK_PATH}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk APP_ABI=${C_ARCHITECTURE} clean
    echo "SDL2 cleaned in $PWD"
    cd ../SDL2_ttf-2.22.0
    ${NDK_PATH}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk APP_ABI=${C_ARCHITECTURE} clean
    echo "SDL2_ttf cleaned in $PWD"
    cd ../../../android
    echo "Back to $PWD"
}


function compile_sdl_lib {
	export NDK_TOOLCHAIN=${CLANG_TOOLCHAIN}
	cd ../rsc/lib/SDL2
	${NDK_PATH}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk APP_ABI=${C_ARCHITECTURE} APP_PLATFORM=android-${ANDROID_VERSION_COMPILE}
	echo "SDL2 build done in $PWD"
	export SDL2_INCLUDE_PATH=$PWD/include
	export SDL2_LIBS=$PWD/libs/${C_ARCHITECTURE}
	cd ../SDL2_ttf-2.22.0
	${NDK_PATH}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk APP_ABI=${C_ARCHITECTURE} APP_PLATFORM=android-${ANDROID_VERSION_COMPILE} APP_ALLOW_MISSING_DEPS=true
	echo "SDL2_ttf build done in $PWD"
	cd ../../../android
	echo "Back to $PWD"
	unset NDK_TOOLCHAIN
}

if [ "$2" == "clean" ]; then
	echo "Cleaning SDL2 ${C_ARCHITECTURE}"
	clean_sdl_lib
fi

compile_sdl_lib

# Définir les variables NDK
export NDK_TOOLCHAIN=${NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64
export NDK_SYSROOT=${NDK_TOOLCHAIN}/sysroot

# Compiler un fichier de test pour vérifier la configuration
# ${NDK_TOOLCHAIN}/bin/clang --sysroot=${NDK_SYSROOT} -target ${C_TARGET} -I${NDK_SYSROOT}/usr/include -I${NDK_SYSROOT}/usr/include/${SYSROOT_SUB_DIR} test_signal.c -o test_signal
# echo "Compilation test successful"

make

# ${NDK_TOOLCHAIN}/bin/llvm-readelf -d C_Chess

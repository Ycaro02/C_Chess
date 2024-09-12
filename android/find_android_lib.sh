#!/bin/bash

ARCHI=""

#check for architecture
if [ "$1" == "arm" ]; then
	echo "ARM architecture"
	ARCHI="aarch64-linux-android/"
elif [ "$1" == "x86" ]; then
	echo "x86 architecture"
	ARCHI="x86_64-linux-android/"
else
	echo "Usage: $0 [arm|x86]"
	exit 1
fi

# check for version needed to be found, if no version is given, return here
if [ -z "$2" ]; then
	echo "Usage: $0 [arm|x86] [version]"
	exit 1
fi

VERSION=$2

mkdir -p android_lib/${1}

# Find the android SDL2.so file
LIBSDL2=$(find ${PWD}/../rsc/lib/SDL2/libs/ -name libSDL2.so | grep ${1})
echo "libSDL2.so found at $LIBSDL2"

# Find the android SDL2_ttf.so file
LIBSDL2_TTF=$(find ${PWD}/../rsc/lib/SDL2_ttf-2.22.0/libs/ -name libSDL2_ttf.so | grep ${1})
echo "libSDL2_ttf.so found at $LIBSDL2_TTF"

# Copy the files to the android_lib folder
cp $LIBC $LIBLOG $LIBANDROID $LIBDL $LIBM $LIBSDL2 $LIBSDL2_TTF android_lib/${1}/

# Check the files
ls -la android_lib/${1}/
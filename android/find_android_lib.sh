#!/bin/bash

ARCHI=""

function get_sdl2_lib {
	local archi="${1}"
	# Find the android SDL2.so file
	LIBSDL2=$(find ${PWD}/../rsc/lib/SDL2/libs/ -name libSDL2.so | grep ${archi})
	echo "libSDL2.so found at ${LIBSDL2}"

	# Find the android SDL2_ttf.so file
	LIBSDL2_TTF=$(find ${PWD}/../rsc/lib/SDL2_ttf-2.22.0/libs/ -name libSDL2_ttf.so | grep ${archi})
	echo "libSDL2_ttf.so found at ${LIBSDL2_TTF}"

	# Copy the files to the android_lib folder
	mkdir -p android_lib/${archi}
	cp ${LIBSDL2} ${LIBSDL2_TTF} android_lib/${archi}
}


get_sdl2_lib "arm64-v8a"
get_sdl2_lib "armeabi-v7a"
get_sdl2_lib "x86"
get_sdl2_lib "x86_64"

# Check the files
ls -lRa android_lib/${1}/
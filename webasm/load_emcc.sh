#!/bin/bash 

# This script is used to load the emcc environment for emscripten

if [ -d "emsdk" ]; then
	echo "emsdk already exists"
	source emsdk/emsdk_env.sh && make
else
	echo "emsdk does not exist"
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk
	./emsdk install latest
	./emsdk activate latest
	source ./emsdk_env.sh && make -C ..
fi
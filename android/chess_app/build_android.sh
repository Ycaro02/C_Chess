#!/bin/bash

function update_lib_dir {
	local archi="${1}"

	mkdir -p app/src/main/jniLibs/${archi}
	echo "Updating ${archi} lib dir"
	cp ../android_lib/${archi}/* app/src/main/jniLibs/${archi}
}

update_lib_dir "arm64-v8a"
update_lib_dir "armeabi-v7a"
update_lib_dir "x86"
update_lib_dir "x86_64"

echo "Copying libs to app/src/main/jniLibs"

export APP_ALLOW_MISSING_DEPS=true && export ANDROID_HOME=~/Android/Sdk

./gradlew clean

if [ "$1" == "clean" ]; then
	exit 0
fi

if [ "$1" == "release" ]; then
	./gradlew assembleRelease
	cp app/build/outputs/apk/release/app-release-unsigned.apk apk_release/chess_app.apk
	echo "Release APK generated at apk_release/chess_app.apk"
	exit 0
fi

./gradlew assembleDebug

cp app/build/outputs/apk/debug/app-debug.apk apk_release/chess_app.apk

echo "APK generated at apk_release/chess_app.apk"
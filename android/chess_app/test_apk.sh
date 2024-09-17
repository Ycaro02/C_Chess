#!/bin/bash
# SDK_VERSION=$(adb shell getprop ro.build.version.sdk)
# echo "Version du SDK : $SDK_VERSION"
 export PATH=$PATH:/home/ycaro/Android/Sdk/platform-tools

#check for path apk provided
if [ -z "$1" ]; then
	echo "Usage: $0 /path/to/your/app.apk"
	exit 1
fi

APK="${1}"

PACKAGE_NAME=org.libsdl.app 

if [ "$2" == clean ]; then
	echo "Unninstal app libsdl.app ..."
	adb uninstall org.libsdl.app
	echo "Install app libsdl.app ..."
	adb install ${APK}
	adb shell monkey -p org.libsdl.app -c android.intent.category.LAUNCHER 1	
else 
	adb shell monkey -p org.libsdl.app -c android.intent.category.LAUNCHER 1	
fi

echo "Logcat ..."
adb logcat -c && adb logcat > log


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
fi

# echo "Listing all installed packages to identify the application package name..."
# adb shell pm list packages > before_packages.txt

# Installer l'APK

# # Lister tous les packages installés pour identifier le nom du package de l'application
# echo "Listing all installed packages to identify the application package name..."
# adb shell pm list packages > after_packages.txt

# echo "Package name is:"
# diff before_packages.txt after_packages.txt

# Get the PID of the application
PID=$(adb shell ps | grep ${PACKAGE_NAME} | awk '{print $2}')

if [ -z "$PID" ]; then
    echo "Failed to get PID for package ${PACKAGE_NAME}. Make sure the app is running."
    exit 1
fi

echo "PID of ${PACKAGE_NAME} is ${PID}"

# Filter logcat output for the application's PID
adb logcat | grep "$PID"
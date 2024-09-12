#!/bin/bash

adb push C_Chess rsc android_lib/x86/* /data/local/tmp/

adb shell chmod 777 /data/local/tmp/C_Chess

adb shell ls -la /data/local/tmp/C_Chess

adb shell 'export LD_LIBRARY_PATH=/data/local/tmp:/system/lib64:/vendor/lib64: && /data/local/tmp/C_Chess'

# SDK_VERSION=$(adb shell getprop ro.build.version.sdk)
# echo "Version du SDK : $SDK_VERSION"

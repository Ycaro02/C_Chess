#!/bin/bash

# This script is used to update the release on GitHub.
source rsc/sh/color.sh


display_color_msg ${YELLOW} "Building the release for all platforms..."

display_color_msg ${LIGHT_BLUE} "Building the Android release..."
./rsc/build_release/build_android_release.sh > /dev/null 2>&1

display_color_msg ${LIGHT_BLUE} "Building the Windows release..."
./rsc/build_release/build_win_release.sh > /dev/null 2>&1

display_color_msg ${LIGHT_BLUE} "Building the Unix release..."
./rsc/build_release/build_linux_release.sh > /dev/null 2>&1

display_color_msg ${GREEN} "All releases have been updated."
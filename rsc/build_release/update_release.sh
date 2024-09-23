#!/bin/bash

# This script is used to upload the Android release APK of the game to GitHub.
# It is intended to be run at the root of the source tree.

source rsc/sh/color.sh

# Global Variables
REPO="Ycaro02/C_Chess"
GITHUB_TOKEN=$(cat ~/.tok_C_chess)
# RELEASE_LOG_FILE="rsc/log/release_log.txt"

source rsc/build_release/release_utils.sh

# Compile the APK
# $1: data path (APK path)
function compile_apk {
	local data_path="${1}"
	display_color_msg ${LIGHT_BLUE} "Preparing the APK for release..."
	display_color_msg ${YELLOW} "From dir is $(pwd)"
	cd android/chess_app
	display_color_msg ${YELLOW} "Current dir is $(pwd)"
	./build_android.sh
	cd ../..
	# Check if the APK exists
	if [ ! -f "${data_path}" ]; then
		display_color_msg ${RED} "APK file not found: ${data_path}"
		exit 1
	fi
}

# Compile the Unix version
# $1 Tar file path 
function compile_linux_version {
	local tar_file="${1}"
	display_color_msg ${LIGHT_BLUE} "Building Unix Release directory..."

	# Build the game
	make -s

	mkdir -p linux_dir/rsc/lib/install/lib
	cp -r rsc/font rsc/texture linux_dir/rsc
	cp rsc/lib/install/lib/libSDL2* linux_dir/rsc/lib/install/lib
	cp C_Chess chess_server linux_dir

	display_color_msg ${LIGHT_BLUE} "Creating the tar.gz file from the build..."

	tar -cvf ${tar_file} -C linux_dir .
	display_color_msg ${GREEN} "Tar file created: ${tar_file}"
	rm -rf linux_dir
	if [ ! -f "${tar_file}" ]; then
		display_color_msg ${RED} "Tar file not found: ${tar_file}"
		exit 1
	fi

}

# Compile the Windows version
# $1: zip file path
function compile_window_version {
	local zip_file="${1}"
	local ino_out="Inno_out"

	display_color_msg ${LIGHT_BLUE} "Building Windows Release directory..."

	# Build the game
	make -s -C windows
	# Clean the release directory remove .o files
	make -s -C windows clean

	# wine "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /O"${INO_OUTPUT_DIR}" /F"Chess Install" windows/setup.iss > /dev/null 2>&1
	wine "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /O"${ino_out}" /F"Chess Install" windows/setup.iss
	mv "${ino_out}/Chess Install.exe" .

	display_color_msg ${LIGHT_BLUE} "Creating the zip file from Chess Install.exe..."
	zip -r ${zip_file} "Chess Install.exe" > /dev/null

	display_color_msg ${GREEN} "Zip file created: ${zip_file}, clean ${ino_out} Chess Install.exe"
	rm -rf ${ino_out} "Chess Install.exe"
}

# Update the release
# $1: release name
# $2: release tag
# $3: APK path
# $4: Compile function
function release_create {
	local release_name="${1}"
	local start_tag="${2}"
	local data_path="${3}"
	local compile_func="${4}"


	local version=$(get_version "rsc/version/version.h")
	display_color_msg ${YELLOW} "Version: ${version}"
	local release_tag="${start_tag}${version}"


	# compile_apk
	${compile_func} ${data_path}

	remove_old_release_name ${release_name}
	remove_old_release_tags ${start_tag}

	update_release ${release_name} ${release_tag} ${data_path}

	if [ "${release_name}" != "C_Chess_Android" ]; then
		# Clean up the tar.gz file
		rm -rf ${data_path}
		make fclean
		display_color_msg ${YELLOW} "\nRemoved ${data_path}!" 
	fi
}

# display_color_msg ${YELLOW} "Updating the Windows release ..."
# release_create "C_Chess_Win" "WindowsRelease_" "C_Chess_Win.zip" "compile_window_version"
# display_color_msg ${GREEN} "Windows release updated!"

# display_color_msg ${YELLOW} "Updating the Linux release ..."
# release_create "C_Chess_Linux" "LinuxRelease_" "C_Chess.tar.gz" "compile_linux_version"
# display_color_msg ${GREEN} "Linux release updated!"

display_color_msg ${YELLOW} "Updating the Android release..."
release_create "C_Chess_Android" "AndroidRelease_" "android/chess_app/apk_release/chess_app.apk" "compile_apk"
display_color_msg ${GREEN} "Android release updated!"

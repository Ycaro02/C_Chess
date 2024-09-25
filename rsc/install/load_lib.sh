#!/bin/bash

function setup_deb_packages {

	# Tmp directories for lib and include files for deb packages
	local pwd_save=${PWD}
	TMP_DIR=${PWD}/tmp
	TMP_LIB_DIR=${PWD}/tmp_lib
	TMP_INCLUDE_DIR=${PWD}/tmp_include

	local os_release=$(source /etc/os-release; echo ${ID})
	echo "OS release: ${os_release}"

	# Load missing deb packages function
	# Need to declare PWD and TMP_LIB_DIR/TMP_INCLUDE_DIR before loading the script
	source ${PWD}/rsc/install/install_missing_deb.sh

	# Create directories
	display_color_msg ${YELLOW} "Create directories ${DEPS_DIR} and ${INSTALL_DIR}."
	mkdir -p ${DEPS_DIR} ${INSTALL_DIR}/lib/pkgconfig ${INSTALL_DIR}/include

	# Load and install deb packages
	if [ "${os_release}" == "ubuntu" ]; then
		load_missing_deb_package
		cd ${pwd_save}
		# Copy lib and include files to tmp directories
		if [ ! -d ${TMP_LIB_DIR} ]; then
			display_color_msg ${RED} "TMP_LIB_DIR not found."
			exit 1
		fi

		if [ ! -d ${TMP_INCLUDE_DIR} ]; then
			display_color_msg ${RED} "TMP_INCLUDE_DIR not found."
			exit 1
		fi
	fi # end if os_release

	# Copy lib and include files to install directory
	display_color_msg ${MAGENTA} "Copy TMP lib and include files to install directory."
	cp -r ${TMP_LIB_DIR}/* ${INSTALL_DIR}/lib
	cp -r ${TMP_INCLUDE_DIR}/* ${INSTALL_DIR}/include

	rm -rf ${TMP_LIB_DIR} ${TMP_INCLUDE_DIR} ${TMP_DIR}
}

# Function to download and install SDL2_ttf
function load_SDL2_TTF {
	local url="${1}"
    local ttf_version="${2}"

    display_color_msg ${YELLOW} "Downloading SDL2_ttf version: ${ttf_version}"

	cd ${BASE_DIR}

    # Download SDL2_ttf
    wget ${url} >> $FD_OUT 2>&1

    tar -xzf SDL2_ttf-${ttf_version}.tar.gz >> $FD_OUT 2>&1
    cd SDL2_ttf-${ttf_version}

	mkdir build
	cd build
	cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
		-DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
		-DFREETYPE_INCLUDE_DIRS=${INSTALL_DIR}/include/freetype2 \
		-DFREETYPE_LIBRARY=${INSTALL_DIR}/lib/libfreetype.so \
		-DSDL2_INCLUDE_DIR=${INSTALL_DIR}/include/SDL2 \
		>> $FD_OUT 2>&1

	make -s -j$(nproc) >> $FD_OUT 2>&1
	make -s install >> $FD_OUT 2>&1

    # Clean up
    cd ../..
    rm -rf SDL2_ttf-${ttf_version}.tar.gz

	display_color_msg ${GREEN} "SDL2_ttf version ${ttf_version} installed successfully."
}

function load_SDL2 {
	local sdl_archive="${1}"
	local sdl_dir_version="${2}"
	local sdl_dir="${BASE_DIR}/SDL2"
	
	cd ${BASE_DIR}

	if [ ! -d "${sdl_dir}" ]; then
        display_color_msg ${CYAN} "Get SDL2 repo..."

		# Download and install SDL2
		wget ${sdl_archive} >> $FD_OUT 2>&1
		tar -xvf ${sdl_dir_version}.tar.gz >> $FD_OUT 2>&1
		rm -rf ${sdl_dir_version}.tar.gz
		mv ${sdl_dir_version} ${sdl_dir}
		cd ${sdl_dir}
		display_color_msg ${YELLOW} "Compile and install SDL2 in ${INSTALL_DIR}..."
		mkdir build
		cd build
		cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
			-DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
            -DCMAKE_INCLUDE_PATH=${INSTALL_DIR}/include \
            -DCMAKE_LIBRARY_PATH=${INSTALL_DIR}/lib \
            -DBUILD_SHARED_LIBS=ON \
			>> $FD_OUT 2>&1

		make -s -j$(nproc) >> $FD_OUT 2>&1
		make -s install >> $FD_OUT 2>&1
		display_color_msg ${GREEN} "SDL2 installation done in ${INSTALL_DIR}."
	fi

}

function get_SDL2_windows {
	cd ${BASE_DIR}
	mkdir -p win_lib && cd win_lib
	wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.5/SDL2-devel-2.30.5-mingw.tar.gz >> $FD_OUT 2>&1
	tar -xvf SDL2-devel-2.30.5-mingw.tar.gz >> $FD_OUT 2>&1
	rm -rf SDL2-devel-2.30.5-mingw.tar.gz
}

function get_SDL2_tff_windows {
	cd ${BASE_DIR}
	cd win_lib
	wget https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-devel-2.22.0-mingw.tar.gz >> $FD_OUT 2>&1
	tar -xvf SDL2_ttf-devel-2.22.0-mingw.tar.gz >> $FD_OUT 2>&1
	rm -rf SDL2_ttf-devel-2.22.0-mingw.tar.gz
}


function get_curl_lib_windows {
	cd ${BASE_DIR}
	cd win_lib

	# Download the libcurl DLL for Windows
	wget https://curl.se/windows/dl-8.10.1_1/curl-8.10.1_1-win64-mingw.zip
	# Unzip the downloaded file
	unzip curl-8.10.1_1-win64-mingw.zip
	# Remove the downloaded zip file and unnecessary files
	rm curl-8.10.1_1-win64-mingw.zip
	mv curl-8.10.1_1-win64-mingw curl_lib
}


function load_curl_lib {
	display_color_msg ${YELLOW} "Get curl lib in ${BASE_DIR}/curl_lib..."
	${BASE_DIR}/../install/get_curl_unix_lib.sh ${BASE_DIR}/curl_lib >> $FD_OUT 2>&1
}

function load_windows_lib {
	get_SDL2_windows
	get_SDL2_tff_windows
	get_curl_lib_windows
}

# Get the current directory
PWD=$(pwd)

# Variables
BASE_DIR="$PWD/rsc/lib"
DEPS_DIR="$BASE_DIR/deps"
INSTALL_DIR="$BASE_DIR/install"

# Load the color script and utils functions
# Need to declare PWD and DEPS/INSTALL_DIR before loading the script
source ${PWD}/rsc/install/install_utils.sh

FD_OUT="/dev/stdout"

# Update FD_OUT if -q option is passed
handle_quiet_opt "${@}"

# Setup deb packages (openGL lib and libudev)
# setup_deb_packages

# Set environment variables for dependencies
export PKG_CONFIG_PATH="${INSTALL_DIR}/lib/pkgconfig"
export CFLAGS="-I${INSTALL_DIR}/include"
export CXXFLAGS="-I${INSTALL_DIR}/include -L${INSTALL_DIR}/lib"
export LDFLAGS="-L${INSTALL_DIR}/lib"

# Cut script execution if any command fails
set -e 

# load_dependencies
# load_SDL2 "https://github.com/libsdl-org/SDL/releases/download/release-2.30.5/SDL2-2.30.5.tar.gz" "SDL2-2.30.5"
# # Load FreeType for SDL2_ttf for text rendering
# load_lib "https://sourceforge.net/projects/freetype/files/freetype2/2.11.0/freetype-2.11.0.tar.gz/download"
# load_SDL2_TTF "https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz" "2.22.0"


load_curl_lib

# load_windows_lib
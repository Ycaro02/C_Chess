#!/bin/bash

# Install missing deb packages
# mesa-common-dev libgl-dev libdrm-dev libglx-dev libpciaccess-dev libudev-dev 

# create tmp directory
# PWD=$(pwd)


##########################
# DEBUG HERE 
# TMP_DIR=${PWD}/tmp
# TMP_LIB_DIR=${PWD}/tmp_lib
# TMP_INCLUDE_DIR=${PWD}/tmp_include
# mkdir -p ${TMP_DIR} ${TMP_LIB_DIR} ${TMP_INCLUDE_DIR} && cd ${TMP_DIR}
##########################

# Thanks https://www.ubuntuupdates.org/ for the deb packages

source ${PWD}/rsc/sh/color.sh

# Include and lib directory options
ONLY_INC="0"
ONLY_LIB="1"
INC_AND_LIB="2"


mkdir -p ${TMP_DIR} ${TMP_LIB_DIR} ${TMP_INCLUDE_DIR} && cd ${TMP_DIR}


function install_deb_package {
	local package_name="${1}"
	local include_lib_dir="${2}"

	if [ ${include_lib_dir} == ${ONLY_INC} ]; then 
		cp -r ${TMP_DIR}/${package_name}/usr/include/* ${TMP_INCLUDE_DIR}
	fi

	if [ ${include_lib_dir} == ${ONLY_LIB} ]; then
		cp -r ${TMP_DIR}/${package_name}/usr/lib/x86_64-linux-gnu/* ${TMP_LIB_DIR}
	fi

	if [ ${include_lib_dir} == ${INC_AND_LIB} ]; then
		cp -r ${TMP_DIR}/${package_name}/usr/include/* ${TMP_INCLUDE_DIR}
		cp -r ${TMP_DIR}/${package_name}/usr/lib/x86_64-linux-gnu/* ${TMP_LIB_DIR}
	fi
}

function load_deb_package {
	local deb_url="${1}"
	local package_name="${2}"
	local include_lib_dir="${3}"
	
	local deb_file="${package_name}.deb"

	display_color_msg ${MAGENTA} "Download deb package ${package_name}..."

	wget ${deb_url} -O ${deb_file} >> $FD_OUT 2>&1

	dpkg-deb -x ${deb_file} ${package_name} >> $FD_OUT 2>&1

	if [ $? -ne 0 ]; then
		echo "Failed to install ${package_name}"
		exit 1
	fi
	rm ${deb_file}
	install_deb_package ${package_name} ${include_lib_dir}

	display_color_msg ${GREEN} "Download deb package ${package_name}"

}



function load_missing_deb_package {
	local ubuntu_url="http://security.ubuntu.com/ubuntu/pool/"

	display_color_msg ${YELLOW} "Installing missing deb packages from ${ubuntu_url} "

	# Mesa common dev and his dependencies (libgl, libGLU)
	load_deb_package ${ubuntu_url}/main/m/mesa/mesa-common-dev_24.0.5-1ubuntu1_amd64.deb mesa-common-dev ${INC_AND_LIB}
	
	load_deb_package ${ubuntu_url}/main/libg/libglu/libglu1-mesa-dev_9.0.2-1.1build1_amd64.deb libglu1-mesa-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libg/libglu/libglu1-mesa_9.0.2-1.1build1_amd64.deb libglu1-mesa ${ONLY_LIB}

	# Lib gl and his dependencies
	load_deb_package ${ubuntu_url}/main/libg/libglvnd/libgl-dev_1.6.0-1_amd64.deb libgl-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libg/libglvnd/libgl1_1.7.0-1build1_amd64.deb libgl1 ${ONLY_LIB}

	# Lib glx and his dependencies
	# load_deb_package ${ubuntu_url}/main/libg/libglvnd/libglx-dev_1.7.0-1build1_amd64.deb libglx-dev ${INC_AND_LIB}
	# load_deb_package ${ubuntu_url}/main/libg/libglvnd/libglx0_1.7.0-1build1_amd64.deb libglx0 ${ONLY_LIB}
	load_deb_package http://security.ubuntu.com/ubuntu/pool/main/libg/libglvnd/libglx-dev_1.6.0-1_amd64.deb libglx-dev ${INC_AND_LIB}
	load_deb_package http://security.ubuntu.com/ubuntu/pool/main/libg/libglvnd/libglx0_1.6.0-1_amd64.deb libglx0 ${ONLY_LIB}

	# Lib drm and his dependencies
	load_deb_package ${ubuntu_url}/main/libd/libdrm/libdrm-dev_2.4.101-2_amd64.deb libdrm-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libd/libdrm/libdrm2_2.4.120-2build1_amd64.deb  libdrm2 ${ONLY_LIB}
	load_deb_package ${ubuntu_url}/main/libd/libdrm/libdrm-radeon1_2.4.120-2build1_amd64.deb libdrm-radeon1 ${ONLY_LIB}
	load_deb_package ${ubuntu_url}/main/libd/libdrm/libdrm-nouveau2_2.4.120-2build1_amd64.deb libdrm-nouveau2 ${ONLY_LIB}
	load_deb_package ${ubuntu_url}/main/libd/libdrm/libdrm-intel1_2.4.120-2build1_amd64.deb libdrm-intel1 ${ONLY_LIB}
	load_deb_package ${ubuntu_url}/main/libd/libdrm/libdrm-amdgpu1_2.4.120-2build1_amd64.deb libdrm-amdgpu1 ${ONLY_LIB}

	# Lib pciaccess and his dependencies
	load_deb_package ${ubuntu_url}/main/libp/libpciaccess/libpciaccess-dev_0.17-3build1_amd64.deb libpciaccess-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libp/libpciaccess/libpciaccess0_0.17-3build1_amd64.deb libpciaccess0 ${ONLY_LIB}
	
	# Lib udev and his dependencies
	load_deb_package ${ubuntu_url}/main/s/systemd/libudev-dev_255.4-1ubuntu8.2_amd64.deb libudev-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/s/systemd/libudev1_255.4-1ubuntu8.2_amd64.deb libudev1 ${ONLY_LIB}

	# x11 
	# load_deb_package ${ubuntu_url}/main/libx/libx11/libx11-dev_1.8.7-1build1_amd64.deb libx11-dev ${INC_AND_LIB}
	# load_deb_package ${ubuntu_url}/main/libx/libx11/libx11-6_1.8.7-1build1_amd64.deb libx11-6 ${ONLY_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libx11/libx11-dev_1.7.5-1ubuntu0.3_amd64.deb libx11-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libx11/libx11-6_1.7.5-1ubuntu0.3_amd64.deb libx11-6 ${ONLY_LIB}

	# load libxext
	load_deb_package ${ubuntu_url}/main/libx/libxi/libxi-dev_1.8.1-1build1_amd64.deb libxi-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxi/libxi6_1.8.1-1build1_amd64.deb libxi6 ${ONLY_LIB}

	# load libxrandr
	load_deb_package ${ubuntu_url}/main/libx/libxrandr/libxrandr-dev_1.5.2-2build1_amd64.deb libxrandr-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxrandr/libxrandr2_1.5.2-2build1_amd64.deb libxrandr2 ${ONLY_LIB}

	# load libxext6
	load_deb_package ${ubuntu_url}/main/libx/libxext/libxext-dev_1.3.4-1build2_amd64.deb libxext-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxext/libxext6_1.3.4-1build2_amd64.deb libxext6 ${ONLY_LIB}

	# Xinera
	load_deb_package ${ubuntu_url}/main/libx/libxinerama/libxinerama-dev_1.1.4-3build1_amd64.deb libxinerama-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxinerama/libxinerama1_1.1.4-3build1_amd64.deb libxinerama1 ${ONLY_LIB}

	# Xcursor
	load_deb_package ${ubuntu_url}/main/libx/libxcursor/libxcursor-dev_1.2.1-1build1_amd64.deb libxcursor-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxcursor/libxcursor1_1.2.1-1build1_amd64.deb libxcursor1 ${ONLY_LIB}

	# xFixe
	load_deb_package ${ubuntu_url}/main/libx/libxfixes/libxfixes-dev_6.0.0-2build1_amd64.deb libxfixes-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxfixes/libxfixes3_6.0.0-2build1_amd64.deb libxfixes3 ${ONLY_LIB}
	
	# Xi
	load_deb_package ${ubuntu_url}/main/libx/libxi/libxi-dev_1.8.1-1build1_amd64.deb libxi-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/main/libx/libxi/libxi6_1.8.1-1build1_amd64.deb libxi6 ${ONLY_LIB}

	# freeglut
	load_deb_package ${ubuntu_url}/universe/f/freeglut/libglut-dev_3.4.0-1build1_amd64.deb libglut-dev ${INC_AND_LIB}
	load_deb_package ${ubuntu_url}/universe/f/freeglut/libglut3.12_3.4.0-1build1_amd64.deb libglut3.12 ${ONLY_LIB}
}

# load_missing_deb_package
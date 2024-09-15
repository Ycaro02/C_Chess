#!/bin/bash

# Convert all PNG files in the given directory to BMP files in ${2} directory
# Usage: ./convert_file.sh ${1} ${2} ${3} ${4}
# ${1} is the directory containing PNG files
# ${2} is the directory to store BMP files
# ${3} is the base extension here PNG
# ${4} is the target extension here BMP

source ../sh/color.sh

# Check if the number of arguments is correct
if [ $# -ne 4 ]
then
	display_color_msg ${YELLOW} "Usage: ./convert_file.sh ${1} ${2} ${3} ${4}\n"
	display_color_msg ${YELLOW} "\t${1} is the directory containing PNG files\n"
	display_color_msg ${YELLOW} "\t${2} is the directory to store BMP files\n"
	display_color_msg ${YELLOW} "\t${3} is the base extension here PNG\n"
	display_color_msg ${YELLOW} "\t${4} is the target extension here BMP\n"
	exit 1
fi

SOURCE_DIR=${1}
TARGET_DIR=${2}
BASE_EXTENSION=${3}
TARGET_EXTENSION=${4}

# Check if the source directory exists
if [ ! -d ${SOURCE_DIR} ]
then
	display_color_msg ${RED} "Directory ${SOURCE_DIR} does not exist"
	exit 1
fi

# Check if the target directory exists
if [ ! -d ${TARGET_DIR} ]
then
	display_color_msg ${CYAN} "Directory ${TARGET_DIR} does not exist, creating it..."
	mkdir ${TARGET_DIR}
fi


# Apply the conversion on all files in the source directory
for file in ${SOURCE_DIR}/*.${BASE_EXTENSION}
do
	filename=$(basename ${file})
	filename=${filename%.*}
	display_color_msg ${LIGHT_MAGENTA} "Converting ${file} to ${TARGET_DIR}/${filename}.${TARGET_EXTENSION}"
	convert ${file} ${TARGET_DIR}/${filename}.${TARGET_EXTENSION}
done
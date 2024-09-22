#!/bin/bash

# This script is used to upload the Android release APK of the game to GitHub.
# It is intended to be run at the root of the source tree.

source rsc/sh/color.sh

# Variables
REPO="Ycaro02/C_Chess"
RELEASE_NAME="C_Chess_Android"
GITHUB_TOKEN=$(cat ~/.tok_C_chess)
APK_PATH="android/chess_app/apk_release/chess_app.apk"
APK_NAME="chess_app.apk"

# Get the version from the version file
function get_version {
	local version_file="rsc/version/version.h"
	if [ ! -f "${version_file}" ]; then
		display_color_msg ${RED} "Version file not found: ${version_file}"
		exit 1
	fi
	echo $(grep -oP '(?<=#define CHESS_VERSION ")[^"]*' ${version_file})
}

# Compile the APK
function compile_apk {
	display_color_msg ${LIGHT_BLUE} "Preparing the APK for release..."
	display_color_msg ${YELLOW} "From dir is $(pwd)"
	cd android/chess_app
	display_color_msg ${YELLOW} "Current dir is $(pwd)"
	./build_android.sh
	cd ../..
}

# List all releases and filter by name, to update the release if it already exists
function get_release_by_name() {
	local release_name="${1}"
	local release_id=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
		"https://api.github.com/repos/${REPO}/releases" | jq -r ".[] | select(.name == \"${release_name}\") | .id")
	echo "${release_id}"
}

function update_release {

	local release_id=$(get_release_by_name "${RELEASE_NAME}")

	if [ "${release_id}" == "null" ]; then
		# Create a new release
		display_color_msg ${LIGHT_BLUE} "Creating a new release..."
		RESPONSE=$(curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
			-H "Content-Type: application/json" \
			-d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"${RELEASE_NAME}\", \"body\": \"Release of ${RELEASE_NAME}\", \"draft\": false, \"prerelease\": false}" \
			"https://api.github.com/repos/${REPO}/releases")
		release_id=$(echo "${RESPONSE}" | jq -r '.id')
	else
		# Update the existing release
		display_color_msg ${LIGHT_BLUE} "Updating the existing release : ID: ${release_id}, TAG: ${RELEASE_TAG}"
		RESPONSE=$(curl -s -X PATCH -H "Authorization: token ${GITHUB_TOKEN}" \
			-H "Content-Type: application/json" \
			-d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"$RELEASE_NAME\", \"body\": \"Updated release of ${RELEASE_NAME}: Version ${VERSION}\", \"draft\": false, \"prerelease\": false}" \
			"https://api.github.com/repos/${REPO}/releases/${release_id}")
	fi

	# Get the asset ID of the existing APK file
	ASSET_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
		"https://api.github.com/repos/${REPO}/releases/${release_id}/assets" | jq -r ".[] | select(.name == \"${APK_NAME}\") | .id")

	# Delete the existing APK file if it exists
	if [ "${ASSET_ID}" != "" ]; then
		display_color_msg ${LIGHT_BLUE} "Deleting the existing asset with ID: ${ASSET_ID}"
		curl -s -X DELETE -H "Authorization: token ${GITHUB_TOKEN}" \
			"https://api.github.com/repos/${REPO}/releases/assets/${ASSET_ID}"
	fi

	# Upload the new APK file to the release
	display_color_msg ${LIGHT_BLUE} "Uploading the new APK file..."
	curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
		-H "Content-Type: application/vnd.android.package-archive" \
		--data-binary @"${APK_PATH}" \
		"https://uploads.github.com/repos/${REPO}/releases/${release_id}/assets?name=${APK_NAME}"

	display_color_msg ${GREEN} "\nAndroid release APK uploaded to GitHub!"

}

function remove_old_release_tags {
	local tag_name="${1}"
	# List all tags and delete those starting with ${tag_name}
	TAGS=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
		"https://api.github.com/repos/${REPO}/git/refs/tags")

	ALL_TAGS=$(echo "${TAGS}" | jq -r ".[] | select(.ref | startswith(\"refs/tags/${tag_name}\")) | .ref")

	for TAG_NAME in ${ALL_TAGS}; do
		display_color_msg ${LIGHT_BLUE} "Deleting tag: ${TAG_NAME}"
		curl -s -X DELETE -H "Authorization: token ${GITHUB_TOKEN}" \
			"https://api.github.com/repos/${REPO}/git/${TAG_NAME}"
	done
}

VERSION=$(get_version)
display_color_msg ${YELLOW} "Version: ${VERSION}"
RELEASE_TAG="AndroidRelease_${VERSION}"


compile_apk

# Check if the APK exists
if [ ! -f "${APK_PATH}" ]; then
    display_color_msg ${RED} "APK file not found: ${APK_PATH}"
    exit 1
fi

remove_old_release_tags "AndroidRelease_"

update_release
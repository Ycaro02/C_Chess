#!/bin/bash

# This script is used to upload the Android release APK of the game to GitHub.
# It is intended to be run at the root of the source tree.

source rsc/sh/color.sh

# Variables
REPO="Ycaro02/C_Chess"
RELEASE_TAG="AndroidRelease"
RELEASE_NAME="C_Chess_Android"
GITHUB_TOKEN=$(cat ~/.tok_C_chess)
APK_PATH="android/chess_app/apk_release/chess_app.apk"
APK_NAME="chess_app.apk"

display_color_msg ${LIGHT_BLUE} "Preparing the APK for release..."

# Check if the APK exists
if [ ! -f "${APK_PATH}" ]; then
    display_color_msg ${RED} "APK file not found: ${APK_PATH}"
    exit 1
fi

# Create or update the release on GitHub
RELEASE_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
    "https://api.github.com/repos/${REPO}/releases/tags/${RELEASE_TAG}" | jq -r '.id')

if [ "${RELEASE_ID}" == "null" ]; then
    # Create a new release
    display_color_msg ${LIGHT_BLUE} "Creating a new release..."
    RESPONSE=$(curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"${RELEASE_NAME}\", \"body\": \"Release of ${RELEASE_NAME}\", \"draft\": false, \"prerelease\": false}" \
        "https://api.github.com/repos/${REPO}/releases")
    RELEASE_ID=$(echo "${RESPONSE}" | jq -r '.id')
else
    # Update the existing release
    display_color_msg ${LIGHT_BLUE} "Updating the existing release : ID: ${RELEASE_ID}, TAG: ${RELEASE_TAG}"
    RESPONSE=$(curl -s -X PATCH -H "Authorization: token ${GITHUB_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"$RELEASE_NAME\", \"body\": \"Updated release of ${RELEASE_NAME}\", \"draft\": false, \"prerelease\": false}" \
        "https://api.github.com/repos/${REPO}/releases/${RELEASE_ID}")
fi

# Get the asset ID of the existing APK file
ASSET_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
    "https://api.github.com/repos/${REPO}/releases/${RELEASE_ID}/assets" | jq -r ".[] | select(.name == \"${APK_NAME}\") | .id")

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
    "https://uploads.github.com/repos/${REPO}/releases/${RELEASE_ID}/assets?name=${APK_NAME}"

display_color_msg ${GREEN} "\nAndroid release APK uploaded to GitHub!"


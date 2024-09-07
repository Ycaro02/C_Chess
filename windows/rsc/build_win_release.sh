#!/bin/bash

# This script is used to build the Windows release of the game and upload it to GitHub.
# It is intended to be run at the root of the source tree.

source rsc/sh/color.sh

# Variables
REPO="Ycaro02/C_Chess"
RELEASE_TAG="WindowsRelease"
RELEASE_NAME="C_Chess_Win"
GITHUB_TOKEN=$(cat ~/.tok_C_chess)
CHES_WIN="C_Chess_Win"
ZIP_FILE="C_Chess_Win.zip"

INO_OUTPUT_DIR="Inno_out"

display_color_msg ${LIGHT_BLUE} "Building Windows Release directory..."

# Build the game
make -s -C windows

# Clean the release directory remove .o files
make -s -C windows clean

wine "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /O"${INO_OUTPUT_DIR}" /F"Chess Install" windows/setup.iss > /dev/null 2>&1

mv "${INO_OUTPUT_DIR}/Chess Install.exe" .

display_color_msg ${LIGHT_BLUE} "Creating the zip file from Chess Install.exe..."

zip -r ${ZIP_FILE} "Chess Install.exe" > /dev/null

# Create or update the release on GitHub
RELEASE_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
    "https://api.github.com/repos/${REPO}/releases/tags/${RELEASE_TAG}" | jq -r '.id')

if [ "${RELEASE_ID}" == "null" ]; then
    # Create a new release
    display_color_msg ${LIGHT_BLUE} "Creating a new release..."
    RESPONSE=$(curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"${RELEASE_NAME}\", \"body\": \"Release of ${CHES_WIN}\", \"draft\": false, \"prerelease\": false}" \
        "https://api.github.com/repos/${REPO}/releases")
    RELEASE_ID=$(echo "${RESPONSE}" | jq -r '.id')
else
    # Update the existing release
    display_color_msg ${LIGHT_BLUE} "Updating the existing release : ID: ${RELEASE_ID}, TAG: ${RELEASE_TAG}"
    RESPONSE=$(curl -s -X PATCH -H "Authorization: token ${GITHUB_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"$RELEASE_NAME\", \"body\": \"Updated release of ${CHES_WIN}\", \"draft\": false, \"prerelease\": false}" \
        "https://api.github.com/repos/${REPO}/releases/${RELEASE_ID}")
fi

# Get the asset ID of the existing zip file
ASSET_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
    "https://api.github.com/repos/${REPO}/releases/${RELEASE_ID}/assets" | jq -r '.[] | select(.name == "C_Chess_Win.zip") | .id')

# Delete the existing zip file if it exists
if [ "${ASSET_ID}" != "" ]; then
    display_color_msg ${LIGHT_BLUE} "Deleting the existing asset with ID: ${ASSET_ID}"
    curl -s -X DELETE -H "Authorization: token ${GITHUB_TOKEN}" \
        "https://api.github.com/repos/${REPO}/releases/assets/${ASSET_ID}"
fi

# Upload the new zip file to the release
display_color_msg ${LIGHT_BLUE} "Uploading the new zip file..."
curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
    -H "Content-Type: application/zip" \
    --data-binary @${ZIP_FILE} \
    "https://uploads.github.com/repos/${REPO}/releases/${RELEASE_ID}/assets?name=${ZIP_FILE}"

# Clean up the zip file
rm -rf ${ZIP_FILE} ${INO_OUTPUT_DIR} "Chess Install.exe"

display_color_msg ${YELLOW} "\nRemove ${ZIP_FILE} ${INO_OUTPUT_DIR} Chess Install.exe!" 
display_color_msg ${GREEN} "Windows release uploaded to GitHub!"
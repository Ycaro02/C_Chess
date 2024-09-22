#!/bin/bash

# This script is used to build the Unix release of the game and upload it to GitHub.
# It is intended to be run at the root of the source tree.

source rsc/sh/color.sh

# Variables
REPO="Ycaro02/C_Chess"
RELEASE_TAG="LinuxRelease"
RELEASE_NAME="C_Chess_Linux"
GITHUB_TOKEN=$(cat ~/.tok_C_chess)
CHES_UNIX="C_Chess"
TAR_FILE="C_Chess.tar.gz"

display_color_msg ${LIGHT_BLUE} "Building Unix Release directory..."

# Build the game
make -s

mkdir -p linux_dir/rsc/lib/install/lib
cp -r rsc/font rsc/texture linux_dir/rsc
cp rsc/lib/install/lib/libSDL2* linux_dir/rsc/lib/install/lib
cp C_Chess chess_server linux_dir

display_color_msg ${LIGHT_BLUE} "Creating the tar.gz file from the build..."

tar -cvf ${TAR_FILE} -C linux_dir .
rm -rf linux_dir

# Create or update the release on GitHub
RELEASE_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
    "https://api.github.com/repos/${REPO}/releases/tags/${RELEASE_TAG}" | jq -r '.id')

if [ "${RELEASE_ID}" == "null" ]; then
    # Create a new release
    display_color_msg ${LIGHT_BLUE} "Creating a new release..."
    RESPONSE=$(curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"${RELEASE_NAME}\", \"body\": \"Release of ${CHES_UNIX}\", \"draft\": false, \"prerelease\": false}" \
        "https://api.github.com/repos/${REPO}/releases")
    RELEASE_ID=$(echo "${RESPONSE}" | jq -r '.id')
else
    # Update the existing release
    display_color_msg ${LIGHT_BLUE} "Updating the existing release : ID: ${RELEASE_ID}, TAG: ${RELEASE_TAG}"
    RESPONSE=$(curl -s -X PATCH -H "Authorization: token ${GITHUB_TOKEN}" \
        -H "Content-Type: application/json" \
        -d "{\"tag_name\": \"${RELEASE_TAG}\", \"name\": \"$RELEASE_NAME\", \"body\": \"Updated release of ${CHES_UNIX}\", \"draft\": false, \"prerelease\": false}" \
        "https://api.github.com/repos/${REPO}/releases/${RELEASE_ID}")
fi

# Get the asset ID of the existing tar.gz file
ASSET_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
    "https://api.github.com/repos/${REPO}/releases/${RELEASE_ID}/assets" | jq -r '.[] | select(.name == "C_Chess.tar.gz") | .id')

# Delete the existing tar.gz file if it exists
if [ "${ASSET_ID}" != "" ]; then
    display_color_msg ${LIGHT_BLUE} "Deleting the existing asset with ID: ${ASSET_ID}"
    curl -s -X DELETE -H "Authorization: token ${GITHUB_TOKEN}" \
        "https://api.github.com/repos/${REPO}/releases/assets/${ASSET_ID}"
fi

# Upload the new tar.gz file to the release
display_color_msg ${LIGHT_BLUE} "Uploading the new tar.gz file..."
curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
    -H "Content-Type: application/gzip" \
    --data-binary @${TAR_FILE} \
    "https://uploads.github.com/repos/${REPO}/releases/${RELEASE_ID}/assets?name=${TAR_FILE}"

# Clean up the tar.gz file
rm -rf ${TAR_FILE}

make fclean

display_color_msg ${YELLOW} "\nRemoved ${TAR_FILE}!" 
display_color_msg ${GREEN} "Unix release uploaded to GitHub!"


# Global variables REPO and GITHUB_TOKEN must be defined before using the functions in this script

# Get the version from the version file
# $1: version file path
function get_version {
	local version_file="${1}"
	if [ ! -f "${version_file}" ]; then
		display_color_msg ${RED} "Version file not found: ${version_file}"
		exit 1
	fi
	echo $(grep -oP '(?<=#define CHESS_VERSION ")[^"]*' ${version_file})
}


# List all releases and filter by name, to update the release if it already exists
# $1: release names
function get_release_by_name() {
	local release_name="${1}"
	local release_id=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
		"https://api.github.com/repos/${REPO}/releases" | jq -r ".[] | select(.name == \"${release_name}\") | .id")
	echo "${release_id}"
}

# Get the last word of a string by a separator
# $1: string
# $2: separator
function get_last_word_by_sep {
	local string="${1}"
	local separator="${2}"
	echo $(echo "${string}" | rev | cut -d"${separator}" -f1 | rev)
}

# Create or update a release on GitHub, and upload the data file
# $1: release name
# $2: release tag
# $3: data file path
function update_release {

	local release_name="${1}"
	local release_tag="${2}"
	local data_path="${3}"

	local file_name=$(get_last_word_by_sep "${data_path}" "/")
	display_color_msg ${YELLOW} "Uploading APK file: ${file_name} from ${data_path}"

	local version=$(get_last_word_by_sep "${release_tag}" "_")
	display_color_msg ${YELLOW} "Version: ${version} from release_tag ${release_tag}"

	local release_id=$(get_release_by_name "${release_name}")



	local response=""

	if [ "${release_id}" == "" ]; then
		# Create a new release
		display_color_msg ${LIGHT_BLUE} "Creating a new release..."
		response=$(curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
			-H "Content-Type: application/json" \
			-d "{\"tag_name\": \"${release_tag}\", \"name\": \"${release_name}\", \"body\": \"Release of ${release_name}: Version ${version}\", \"draft\": false, \"prerelease\": false}" \
			"https://api.github.com/repos/${REPO}/releases")
		release_id=$(echo "${response}" | jq -r '.id')
	else
		# Update the existing release
		display_color_msg ${LIGHT_BLUE} "Updating the existing release : ID: ${release_id}, TAG: ${release_tag}"
		response=$(curl -s -X PATCH -H "Authorization: token ${GITHUB_TOKEN}" \
			-H "Content-Type: application/json" \
			-d "{\"tag_name\": \"${release_tag}\", \"name\": \"${release_name}\", \"body\": \"Updated release of ${release_name}: Version ${version}\", \"draft\": false, \"prerelease\": false}" \
			"https://api.github.com/repos/${REPO}/releases/${release_id}")
	fi

	# Get the asset ID of the existing APK file
	ASSET_ID=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
		"https://api.github.com/repos/${REPO}/releases/${release_id}/assets" | jq -r ".[] | select(.name == \"${file_name}\") | .id")

	# Delete the existing APK file if it exists
	if [ "${ASSET_ID}" != "" ]; then
		display_color_msg ${LIGHT_BLUE} "Deleting the existing asset with ID: ${ASSET_ID}"
		curl -s -X DELETE -H "Authorization: token ${GITHUB_TOKEN}" \
			"https://api.github.com/repos/${REPO}/releases/assets/${ASSET_ID}"
	fi

	# Upload the new APK file to the release
	display_color_msg ${LIGHT_BLUE} "Uploading the new ${release_name}..."
	curl -s -X POST -H "Authorization: token ${GITHUB_TOKEN}" \
		-H "Content-Type: application/vnd.android.package-archive" \
		--data-binary @"${data_path}" \
		"https://uploads.github.com/repos/${REPO}/releases/${release_id}/assets?name=${file_name}"

	display_color_msg ${GREEN} "\nRelease ${release_tag} uploaded to GitHub!"

}

# Remove the old release tags
# $1: tag name
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

# Remove the old release by name
# $1: release name
function remove_old_release_name {
	local release_name="${1}"
	local release_id=$(get_release_by_name "${release_name}")

	if [ "${release_id}" != "" ]; then
		display_color_msg ${LIGHT_BLUE} "Deleting the existing release with ID: ${release_id}"
		curl -s -X DELETE -H "Authorization: token ${GITHUB_TOKEN}" \
			"https://api.github.com/repos/${REPO}/releases/${release_id}"
	fi
}
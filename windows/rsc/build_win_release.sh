#!/bin/bash

# This script is used to build the Windows release of the game.
# It is intended to be run at the root of the source tree.

# Build the game
make -s -C windows

# Create a directory for the release
cp -r windows C_Chess_Win

# Zip the release
zip -r C_Chess_Win.zip C_Chess_Win

# Clean up
rm -rf C_Chess_Win

echo "Windows Release directory build"


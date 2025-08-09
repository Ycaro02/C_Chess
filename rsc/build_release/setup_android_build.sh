#!/bin/bash

# docker run --rm -it \
#   -v /home/ycaro:/app \
#   --workdir=/app/chess/C_Chess \
#   eclipse-temurin:17-jdk \
#   bash -c '
#     apt-get update &&
#     apt-get install -y unzip bash make g++ zip wget curl which file findutils &&
#     bash
#   '

# With the docker image:

# docker build -t android-chess-build -f rsc/build_release/Dockerfile .
# docker run --rm -it -v ./:/app/chess/C_Chess --workdir /app/chess/C_Chess chess-android-build ./rsc/build_release/setup_android_build.sh

export ANDROID_HOME=/Android-tools
export ANDROID_CMDLINE_TOOLS=$ANDROID_HOME/cmdline-tools/latest

# Nettoyage
rm -rf "$ANDROID_HOME"

# Création dossier SDK
mkdir -p "$ANDROID_HOME"

# Téléchargement cmdline-tools
wget -q https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip -O /tmp/cmdline-tools.zip
mkdir -p "$ANDROID_CMDLINE_TOOLS"
unzip -q /tmp/cmdline-tools.zip -d "$ANDROID_CMDLINE_TOOLS"
mv "$ANDROID_CMDLINE_TOOLS/cmdline-tools"/* "$ANDROID_CMDLINE_TOOLS"/
rm -rf "$ANDROID_CMDLINE_TOOLS/cmdline-tools"

# Ajout au PATH
export ANDROID_HOME
export PATH=$ANDROID_HOME/platform-tools:$ANDROID_HOME/cmdline-tools/latest/bin:$PATH

# Installation des packages requis
yes | sdkmanager --licenses
sdkmanager "platform-tools" "platforms;android-34" "build-tools;34.0.0" "ndk;25.1.8937393"


export ANDROID_WORK_DIR=/app/chess/C_Chess/android
cd ${ANDROID_WORK_DIR}/chess_app/

./build_android.sh
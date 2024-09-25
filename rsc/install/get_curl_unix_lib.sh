#!/bin/bash

# we need opeen ssl and libpsl-dev


# $1 is the path where you want to install curl
if [ -z "$1" ]
then
	echo "Please provide the path where you want to install curl"
	exit 1
fi

INSTALL_PATH=$1

mkdir -p ${INSTALL_PATH}

echo "Installing curl in ${INSTALL_PATH}"

cd ${INSTALL_PATH}

# Download and extract curl
wget https://github.com/curl/curl/archive/refs/tags/curl-8_10_0.tar.gz
tar -xvf curl-8_10_0.tar.gz
rm -rf curl-8_10_0.tar.gz

mv curl-curl-8_10_0  curl-8_10_0

cd curl-8_10_0

# Generate configure script
autoreconf -fi

# Configure
./configure --with-openssl prefix=${INSTALL_PATH}

# Build
make && make install && cd .. && rm -rf curl-8_10_0
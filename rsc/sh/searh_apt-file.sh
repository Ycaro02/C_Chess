#!/bin/bash

# Loop on args to call apt-file search

for package in "$@"
do
	echo -n "apt-file search $package : "
	apt-file search $package
done

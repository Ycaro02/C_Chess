#!/bin/bash

xhost +local:docker

docker build -t my-x11-app .
docker run -it --rm -e DISPLAY=$DISPLAY -v ./:/app --workdir /app -v /tmp/.X11-unix:/tmp/.X11-unix my-x11-app zsh

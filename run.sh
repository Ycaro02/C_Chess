#!/bin/bash

xhost +local:docker

docker build -t c-chess .
docker run -it --rm -e DISPLAY=$DISPLAY --name c-chess-app -v ./:/app --workdir /app -v /tmp/.X11-unix:/tmp/.X11-unix c-chess zsh
docker exec -it c-chess-app zsh
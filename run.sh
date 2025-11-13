#!/bin/bash

xhost +local:docker

docker build -t c-chess .

if [[ ${1} == "dev" ]]; then
    echo "Starting in development mode..."
    docker run -d -it --rm -e DISPLAY=$DISPLAY \
        --name c-chess-app -v ~/.ssh:/home/root/.ssh -v ./:/app --workdir /app \
        -v /tmp/.X11-unix:/tmp/.X11-unix --network host c-chess sleep infinity
    # Set up git user configuration
    GIT_EMAIL=$(git config user.email)
    GIT_NAME=$(git config user.name)
    docker exec c-chess-app git config --global user.email "${GIT_EMAIL}"
    docker exec c-chess-app git config --global user.name "${GIT_NAME}"
    docker exec -it c-chess-app zsh
else
    echo "Starting in normal mode..."
    docker run -it --rm -e DISPLAY=$DISPLAY \
        --name c-chess-app -v ./:/app --workdir /app \
        -v /tmp/.X11-unix:/tmp/.X11-unix --network host c-chess bash -c "make && ./C_Chess"
fi

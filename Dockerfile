# Need to run this on the host: xhost +local:docker
FROM ubuntu:22.04

# Install dependencies
RUN apt update \
    && apt install -y \
    make \
    clang \
    gcc \
    git \
    vim \
    zsh \
    curl \
    valgrind \
    wget \
    libxext-dev \
    cmake \
    x11-apps \
    libssl-dev \
    libx11-dev

# Get oh-my-zsh for a better terminal experience
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"


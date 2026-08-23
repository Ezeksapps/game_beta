#!/bin/bash

GREEN='\033[0;32m'
CYAN='\033[0;36m'
ORANGE='\033[0;33m'
DEFAULT='\033[0m'

# Initial setup, install dependencies #

mkdir deps && cd deps

echo "${CYAN}Fetching dependencies before running build${DEFAULT}"

git clone --recursive https://github.com/DiligentGraphics/DiligentCore
git clone --recursive https://github.com/DiligentGraphics/DiligentTools
git clone --recursive https://github.com/librats/librats
git clone --recursive https://github.com/glfw/glfw
git clone --recursive https://github.com/g-truc/glm

echo "${CYAN}Completed dependency downloads${DEFAULT}"

cd ..

mkdir build
cmake -S . -B ./build
cmake --build ./build

#!/bin/bash

echo ">build.sh : Configuring CMake project..."
cmake .
echo ">build.sh : Generating CMake build..."
cmake --build .
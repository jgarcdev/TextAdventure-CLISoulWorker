#!/usr/bin/bash

if [ -d "build" ]; then
		rm -rf build
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

cmake --build . --config Release --target clisw-launcher
cmake --build . --config Release --target clisw-installer

cmake --build . --config Release --target package
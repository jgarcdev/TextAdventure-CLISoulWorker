#!/usr/bin/bash

if [ -d "build" ]; then
	rm -rf build
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER:FILEPATH=/opt/LLVM-21.1.6-Linux-X64/bin/clang-cl -DCMAKE_CXX_COMPILER:FILEPATH=/opt/LLVM-21.1.6-Linux-X64/bin/clang-cl ..
cmake --build . --config Release

cmake --build . --config Release --target clisw-launcher
cmake --build . --config Release --target clisw-installer

cmake --build . --config Release --target package
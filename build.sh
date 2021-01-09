#!/bin/bash

set -e
set -x

rm -rf build
mkdir build
pushd build

conan install .. --build=openssl --build=sqlcipher --build=quill
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
# 
# bin/passwordmanager

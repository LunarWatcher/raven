#!/usr/bin/env bash
if [ ! -d cmake-conan ]; then
    git clone https://github.com/conan-io/cmake-conan
fi

cmake .. "-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=$(pwd)/cmake-conan/conan_provider.cmake" "$@"

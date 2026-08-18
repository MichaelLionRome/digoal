#!/bin/bash

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

mkdir -p _tmp_build_0
cd _tmp_build_0
cmake .. > /dev/null 2> /dev/null
make -j4 > /dev/null 2> /dev/null
if [ $? -eq 0 ]; then
    echo "Works with GNU Compiler"
else
    echo "Doesn't work with GNU Compiler"
fi
cd ..
rm -rf _tmp_build_0


export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

mkdir -p _tmp_build_0
cd _tmp_build_0
cmake .. > /dev/null 2> /dev/null
make -j4 > /dev/null 2> /dev/null
if [ $? -eq 0 ]; then
    echo "Works with CLANG Compiler";
else
    echo "Doesn't work with CLANG Compiler";
fi
cd ..
rm -rf _tmp_build_0

#!/bin/bash

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

make re > /dev/null 2> /dev/null

if [ $? -eq 0 ]; then
    echo "Works with GNU Compiler"
else
    echo "Doesn't work with GNU Compiler"
fi

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

make re > /dev/null 2> /dev/null 

if [ $? -eq 0 ]; then
    echo "Works with CLANG Compiler";
else
    echo "Doesn't work with CLANG Compiler";
fi

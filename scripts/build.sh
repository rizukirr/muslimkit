#!/bin/bash

BUILD_TYPE="${1,,}"

if [[ "$BUILD_TYPE" == "release" ]]; then
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Relase -DCMAKE_C_COMPILER=clang && cmake --build build --clean-first
else
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_C_FLAGS_DEBUG="-g -O1" && cmake --build build --clean-first
fi

cppcheck --enable=all --inconclusive --std=c11 \
  --template='{file}:{line}:{severity}:{id}:{message}' \
  -I include/ src/ main.c 2>&1 | grep -E "(leak|free|uninit)"

valgrind --leak-check=full --show-leak-kinds=all \
  --track-origins=yes --log-file=valgrind.log \
  ./build/bin/muslimkit

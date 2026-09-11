#!/usr/bin/env bash
set -e
cmake -S . -B build -DCMAKE_BUILD_TYPE=$1
cmake --build build --config $1 --parallel
#!/usr/bin/env bash
set -e
cd assembler
./build.sh Debug
build/FpuASM ../programs/$1.asm ../programs/$1.rom
cd ..
./build.sh Debug
build/FloorPU programs/$1.rom
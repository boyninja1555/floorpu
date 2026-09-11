#pragma once

#include "libfloorpu/typing.h"

#define RAM_SIZE 0x10000
#define RAM_SIZE_ROM 0x8000
#define RAM_SIZE_VRAM 0x4000
#define RAM_SIZE_STACK 0x1000

#define RAM_ADDR_ROM 0
#define RAM_ADDR_VRAM RAM_SIZE_ROM
#define RAM_ADDR_STACK (RAM_SIZE - RAM_SIZE_STACK)

extern u8 ram[RAM_SIZE];

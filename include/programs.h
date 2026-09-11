#pragma once

#include "libfloorpu/cpu.h"

static const u8 program_addfunc[] = {
    // _start0
    OP_PUSHA, 0x0A, 0x00,  // PUSHA _start1
    OP_PUSHL, 0x01,        // PUSHL 1
    OP_PUSHL, 0x01,        // PUSHL 1
    OP_JUMP, 0x1C, 0x00,   // JUMP add

    // _start1
    OP_SET, 0x00, 0x00,  // SET r0 0
    OP_SET, 0x01, 0x00,  // SET r1 0
    OP_SET, 0x02, 0x00,  // SET r2 0
    OP_SET, 0x03, 0x00,  // SET r3 0
    OP_SET, 0x04, 0x00,  // SET r4 0
    OP_POP, 0x00,        // POP r0
    OP_HALT,             // HALT

    // add
    OP_POP, 0x04,              // POP r4
    OP_POP, 0x03,              // POP r3
    OP_POPA, 0x01, 0x02,       // POPA r1 r2
    OP_ADD, 0x00, 0x03, 0x04,  // ADD r0 r1 r2
    OP_PUSH, 0x00,             // PUSH r0
    OP_JUMPR, 0x01, 0x02,      // JUMPR r1 r2
};

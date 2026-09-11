#pragma once

#include "libfloorpu/typing.h"
#include "libfloorpu/ram.h"

#define CPU_REGISTERS 16
#define CPU_REG_SP_L 12
#define CPU_REG_SP_H 13
#define CPU_REG_PC_L 14
#define CPU_REG_PC_H 15

#define CPU_CLOCK_SPEED 2000000ULL
#define CPU_NS_PER_SECOND 1000000000ULL
#define CPU_NS_PER_CYCLE (CPU_NS_PER_SECOND / CPU_CLOCK_SPEED)

typedef enum
{
    // Control
    OP_HALT,
    OP_JUMP,
    OP_JUMPIZ,
    OP_JUMPNZ,
    OP_JUMPR,
    OP_JUMPRIZ,
    OP_JUMPRNZ,

    // Memory
    OP_SET,
    OP_COPY,
    OP_LOAD,
    OP_STORE,
    OP_STOREL,

    // Stack
    OP_PUSH,
    OP_PUSHL,
    OP_PUSHA,
    OP_POP,
    OP_POPA,

    // ALU,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
} Opcodes;

typedef struct
{
    u8 running;
    u8 registers[CPU_REGISTERS];
} CPU;

u16 cpu_get_sp(const CPU *cpu);

void cpu_set_sp(CPU *cpu, u16 sp);

u16 cpu_get_pc(const CPU *cpu);

void cpu_set_pc(CPU *cpu, u16 pc);

void cpu_init(CPU *cpu);

u64 cpu_step(CPU *cpu);

void cpu_halt(CPU *cpu);

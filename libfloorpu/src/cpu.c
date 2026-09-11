#include "libfloorpu/cpu.h"
#include <stdio.h>

// Private

u8 peek_u8(const CPU *cpu, u16 ahead)
{
    return ram[cpu_get_pc(cpu) + ahead];
}

u16 peek_u16(const CPU *cpu, u16 ahead)
{
    return ((u16)peek_u8(cpu, ahead + 1) << 8) | peek_u8(cpu, ahead);
}

u8 consume_u8(CPU *cpu)
{
    u16 pc = cpu_get_pc(cpu);
    cpu_set_pc(cpu, pc + 1);
    return ram[pc];
}

u16 consume_u16(CPU *cpu)
{
    u8 low = consume_u8(cpu);
    return ((u16)consume_u8(cpu) << 8) | low;
}

// Public

u16 cpu_get_sp(const CPU *cpu)
{
    return ((u16)cpu->registers[CPU_REG_SP_H] << 8) | cpu->registers[CPU_REG_SP_L];
}

void cpu_set_sp(CPU *cpu, u16 sp)
{
    cpu->registers[CPU_REG_SP_H] = (sp >> 8) & 0xFF;
    cpu->registers[CPU_REG_SP_L] = sp & 0xFF;
}

u16 cpu_get_pc(const CPU *cpu)
{
    return ((u16)cpu->registers[CPU_REG_PC_H] << 8) | cpu->registers[CPU_REG_PC_L];
}

void cpu_set_pc(CPU *cpu, u16 pc)
{
    cpu->registers[CPU_REG_PC_H] = (pc >> 8) & 0xFF;
    cpu->registers[CPU_REG_PC_L] = pc & 0xFF;
}

void cpu_init(CPU *cpu)
{
    cpu->running = 1;
    cpu_set_sp(cpu, RAM_ADDR_STACK);
    cpu_set_pc(cpu, RAM_ADDR_ROM);
}

u64 cpu_step(CPU *cpu)
{
    if (!cpu->running)
        return 0;

    u8 opcode = consume_u8(cpu);
    switch (opcode)
    {
    case OP_HALT: // HALT
    {
        cpu_halt(cpu);
        break;
    }

    case OP_JUMP: // JUMP addr(u16)
    {
        cpu_set_pc(cpu, consume_u16(cpu));
        break;
    }

    case OP_JUMPIZ: // JUMPIZ addr(u16) reg_condition(u8)
    {
        u16 addr = consume_u16(cpu);
        if (cpu->registers[consume_u8(cpu)] == 0)
            cpu_set_pc(cpu, addr);

        break;
    }

    case OP_JUMPNZ: // JUMPNZ addr(u16) reg_condition(u8)
    {
        u16 addr = consume_u16(cpu);
        if (cpu->registers[consume_u8(cpu)] != 0)
            cpu_set_pc(cpu, addr);

        break;
    }

    case OP_JUMPR: // JUMPR reg_addr_l(u8) reg_addr_h(u8)
    {
        u8 reg_addr_l = cpu->registers[consume_u8(cpu)];
        cpu_set_pc(cpu, ((u16)cpu->registers[consume_u8(cpu)] << 8) | reg_addr_l);
        break;
    }

    case OP_JUMPRIZ: // JUMPRIZ reg_addr_l(u8) reg_addr_h(u8) reg_condition(u8)
    {
        u8 reg_addr_l = cpu->registers[consume_u8(cpu)];
        if (cpu->registers[consume_u8(cpu)] == 0)
            cpu_set_pc(cpu, ((u16)cpu->registers[consume_u8(cpu)] << 8) | reg_addr_l);

        break;
    }

    case OP_JUMPRNZ: // JUMPRNZ reg_addr_l(u8) reg_addr_h(u8) reg_condition(u8)
    {
        u8 reg_addr_l = cpu->registers[consume_u8(cpu)];
        if (cpu->registers[consume_u8(cpu)] != 0)
            cpu_set_pc(cpu, ((u16)cpu->registers[consume_u8(cpu)] << 8) | reg_addr_l);

        break;
    }

    case OP_SET: // SET reg(u8) value(u8)
    {
        cpu->registers[consume_u8(cpu)] = consume_u8(cpu);
        break;
    }

    case OP_COPY: // COPY reg_source(u8) reg_target(u8)
    {
        u8 source = consume_u8(cpu);
        cpu->registers[consume_u8(cpu)] = cpu->registers[source];
        break;
    }

    case OP_LOAD: // LOAD addr(u16) reg(u8)
    {
        u16 addr = consume_u16(cpu);
        cpu->registers[consume_u8(cpu)] = ram[addr];
        break;
    }

    case OP_LOADP: // LOADP reg_addr_l(u8) reg_addr_r(u8) reg(u8)
    {
        u8 reg_addr_l = consume_u8(cpu);
        u8 reg_addr_r = consume_u8(cpu);
        cpu->registers[consume_u8(cpu)] = ram[((u16)cpu->registers[consume_u8(cpu)] << 8) | cpu->registers[reg_addr_l]];
        break;
    }

    case OP_STORE: // STORE reg(u8) addr(u16)
    {
        u8 reg = consume_u8(cpu);
        ram[consume_u16(cpu)] = cpu->registers[reg];
        break;
    }

    case OP_STOREP: // STOREP reg(u8) reg_addr_l(u8) reg_addr_r(u8)
    {
        u8 reg = consume_u8(cpu);
        u8 reg_addr_l = consume_u8(cpu);
        u8 reg_addr_r = consume_u8(cpu);
        ram[((u16)cpu->registers[consume_u8(cpu)] << 8) | cpu->registers[reg_addr_l]] = cpu->registers[reg];
        break;
    }

    case OP_STOREL: // STOREL value(u8) addr(u16)
    {
        u8 value = consume_u8(cpu);
        ram[consume_u16(cpu)] = value;
        break;
    }

    case OP_PUSH: // PUSH reg_value(u8)
    {
        u16 sp = cpu_get_sp(cpu);
        if (++sp >= RAM_ADDR_STACK + RAM_SIZE_STACK)
        {
            fprintf(stderr, "PUSH failed! Stack pointer would exceed it's maximum size.\n");
            break;
        }

        cpu_set_sp(cpu, sp);
        ram[sp] = cpu->registers[consume_u8(cpu)];
        break;
    }

    case OP_PUSHL: // PUSHL value(u8)
    {
        u16 sp = cpu_get_sp(cpu);
        if (++sp >= RAM_ADDR_STACK + RAM_SIZE_STACK)
        {
            fprintf(stderr, "PUSHL failed! Stack pointer would exceed it's maximum size.\n");
            break;
        }

        cpu_set_sp(cpu, sp);
        ram[sp] = consume_u8(cpu);
        break;
    }

    case OP_PUSHA: // PUSHA value(u16)
    {
        u16 sp = cpu_get_sp(cpu);
        if ((sp += 2) >= RAM_ADDR_STACK + RAM_SIZE_STACK)
        {
            fprintf(stderr, "PUSHA failed! Stack pointer would exceed it's maximum size.\n");
            break;
        }

        cpu_set_sp(cpu, sp);
        ram[sp - 1] = consume_u8(cpu);
        ram[sp] = consume_u8(cpu);
        break;
    }

    case OP_POP: // POP reg_target(u8)
    {
        u16 sp = cpu_get_sp(cpu);
        cpu->registers[consume_u8(cpu)] = ram[sp--];
        cpu_set_sp(cpu, sp);
        break;
    }

    case OP_POPA: // POPA reg_target(u16)
    {
        u8 reg_target_l = consume_u8(cpu);
        u8 reg_target_h = consume_u8(cpu);
        u16 sp = cpu_get_sp(cpu);
        cpu->registers[reg_target_h] = ram[sp--];
        cpu->registers[reg_target_l] = ram[sp--];
        cpu_set_sp(cpu, sp);
        break;
    }

    case OP_ADD: // ADD reg_target(u8) reg_a(u8) reg_b(u8)
    {
        u8 reg_target = consume_u8(cpu);
        u8 reg_a = consume_u8(cpu);
        u8 reg_b = consume_u8(cpu);
        cpu->registers[reg_target] = cpu->registers[reg_a] + cpu->registers[reg_b];
        break;
    }

    case OP_SUB: // SUB reg_target(u8) reg_a(u8) reg_b(u8)
    {
        u8 reg_target = consume_u8(cpu);
        u8 reg_a = consume_u8(cpu);
        u8 reg_b = consume_u8(cpu);
        cpu->registers[reg_target] = cpu->registers[reg_a] - cpu->registers[reg_b];
        break;
    }

    case OP_MUL: // MUL reg_target(u8) reg_a(u8) reg_b(u8)
    {
        u8 reg_target = consume_u8(cpu);
        u8 reg_a = consume_u8(cpu);
        u8 reg_b = consume_u8(cpu);
        cpu->registers[reg_target] = cpu->registers[reg_a] * cpu->registers[reg_b];
        break;
    }

    case OP_DIV: // DIV reg_target(u8) reg_a(u8) reg_b(u8)
    {
        u8 reg_target = consume_u8(cpu);
        u8 reg_a = consume_u8(cpu);
        u8 reg_b = consume_u8(cpu);
        cpu->registers[reg_target] = cpu->registers[reg_a] / cpu->registers[reg_b];
        break;
    }
    }

    printf("opcode = 0x%02X, ", opcode);
    for (u8 i = 0; i < CPU_REGISTERS; i++)
        printf("r%u = 0x%02X, ", i, cpu->registers[i]);
    printf("\n");

    return 1;
}

void cpu_halt(CPU *cpu)
{
    cpu->running = 0;
}

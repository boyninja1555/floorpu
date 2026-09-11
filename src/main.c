#include <string.h>
#include <stdio.h>
#include "libfloorpu/ram.h"
#include "libfloorpu/cpu.h"
#include "programs.h"

int main(int argc, const char *argv[])
{
    if (argc == 1)
        memcpy(ram + RAM_ADDR_ROM, program_addfunc, sizeof program_addfunc);
    else if (argc == 2)
    {
        FILE *file = fopen(argv[1], "rb");
        if (!file)
        {
            fprintf(stderr, "Unable to open %s for reading!\n", argv[1]);
            return 1;
        }

        if (fseek(file, 0, SEEK_END) != 0)
        {
            perror("Unable to seek file");
            fclose(file);
            return 1;
        }

        long size = ftell(file);
        if (size == -1L)
        {
            perror("Unable to tell file size");
            fclose(file);
            return 1;
        }

        if (size != RAM_SIZE_ROM)
        {
            fprintf(stderr, "%s must be exactly %u kB!\n", argv[1], (u32)(RAM_SIZE_ROM / 0x400));
            fclose(file);
            return 1;
        }

        fseek(file, 0, SEEK_SET);
        fread(ram + RAM_ADDR_ROM, RAM_SIZE_ROM, 1, file);
        fclose(file);
    }
    else
    {
        fprintf(stderr, "Usage: %s [program.rom]\n", argv[0]);
        return 1;
    }

    for (uint16_t i = 0; i < 128; i++)
    {
        printf("%02X ", ram[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    CPU cpu = {};
    cpu_init(&cpu);

    while (cpu.running)
        cpu_step(&cpu);

    return 0;
}

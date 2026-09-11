#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "libfloorpu/ram.h"
#include "libfloorpu/cpu.h"

u64 time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * CPU_NS_PER_SECOND) + (uint64_t)ts.tv_nsec;
}

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <program.rom>\n", argv[0]);
        return 1;
    }

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

    CPU cpu = {};
    cpu_init(&cpu);

    u64 time_started = time_ns();
    u64 cycles = 0;
    while (cpu.running)
    {
        uint64_t time_now = time_ns();
        uint64_t time_elapsed = time_now - time_started;
        uint64_t target_cycles = time_elapsed / CPU_NS_PER_CYCLE;
        while (cpu.running && cycles < target_cycles)
            cycles += cpu_step(&cpu);

        struct timespec delay = {.tv_sec = 0, .tv_nsec = 1};
        nanosleep(&delay, NULL);
    }

    return 0;
}

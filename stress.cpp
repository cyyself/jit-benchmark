#include <cstdio>
#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <cstdlib>
#include <random>

void generate_set_value(int x, int *place) {
#ifdef __riscv
    uint32_t instr[4] = {
        0x00053583, // ld a1,0(a0)
        0x00058593, // addi a1,a1,0
        0x00b53023, // sd a1,0(a0)
        0x00008067  // ret
    };
    instr[1] |= (x & 0xfff) << 20;
#else
    #error "Unsupported ISA"
#endif
    memcpy(place, instr, sizeof(instr));
}

int main(int argc, char *argv[]) {
    void* addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    int test = 0;
    if (argc < 2) {
        printf("usage: ./stress <num_loops>\n");
        exit(0);
    }
    for (int i=0;i<atoi(argv[1]);i++) {
        test = 0;
        generate_set_value(i % 2048, (int*)addr);
        asm volatile("fence.i");
        ((void(*)(int *x, int y))addr)(&test, 0);
    }
    return 0;
}
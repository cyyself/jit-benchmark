#include <cstdio>
#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <cstdlib>
#include <random>

#define PAGESIZE 16384

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
    if (argc < 2) {
        printf("usage: ./stress <num_loops> [use_w^x:0]\n");
        exit(0);
    }
    bool use_w_xor_x = false;
    if (argc >= 3) {
        use_w_xor_x = atoi(argv[2]) ? true : false;
    }
    void* addr = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE | (use_w_xor_x ? 0 : PROT_EXEC), MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    int test = 0;
    for (int i=0;i<atoi(argv[1]);i++) {
        test = 0;
        if(use_w_xor_x) mprotect(addr, PAGESIZE, PROT_READ | PROT_WRITE);
        generate_set_value(i % 2048, (int*)addr);
        asm volatile("fence.i");
        if(use_w_xor_x) mprotect(addr, PAGESIZE, PROT_READ | PROT_EXEC);
        ((void(*)(int *x, int y))addr)(&test, 0);
    }
    return 0;
}
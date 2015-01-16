#include <cstdint>
#include <cstdio>
#include <string>

#include "tests.h"
#include "Utils.h"

static u32 GetXER()
{
    u32 xer;
    asm volatile ("mfxer %[out]" : [out]"=b"(xer));
    return xer;
}

static u32 GetCR()
{
    u32 cr;
    asm volatile ("mfcr %[out]" : [out]"=b"(cr));
    return cr;
}

// Test for a 2-component instruction
// e.g. ADDME rD, rA
#define OPTEST_2_COMPONENTS(inst, rA)                                                    \
{                                                                                        \
    u32 output;                                                                          \
    u32 ra = rA;                                                                         \
                                                                                         \
    asm volatile ("mtxer %[val]" : : [val]"b"(0) : "xer");                               \
    asm volatile (inst " %[out], %[Ra]": [out]"=&r"(output) : [Ra]"r"(ra));              \
                                                                                         \
    printf("%-8s :: rD 0x%08X | rA 0x%08X | XER: 0x%08X\n", inst, output, rA, GetXER()); \
}

// Test for a 3-component instruction
// e.g. ADD rD, rA, rB
#define OPTEST_3_COMPONENTS(inst, rA, rB)                                                                \
{                                                                                                        \
    u32 output;                                                                                          \
    u32 ra = rA;                                                                                         \
    u32 rb = rB;                                                                                         \
                                                                                                         \
    asm volatile ("mtxer %[val]" : : [val]"b"(0) : "xer");                                               \
    asm volatile (inst " %[out], %[Ra], %[Rb]": [out]"=&r"(output) : [Ra]"r"(ra), [Rb]"r"(rb));          \
                                                                                                         \
    printf("%-8s :: rD 0x%08X | rA 0x%08X | rB 0x%08X | XER: 0x%08X\n", inst, output, rA, rB, GetXER()); \
}

void PPCIntegerTests()
{
    OPTEST_3_COMPONENTS("ADD", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADD", -1, 1);
    OPTEST_3_COMPONENTS("ADD", -1, -1);
    OPTEST_3_COMPONENTS("ADD", 1, 0);
    OPTEST_3_COMPONENTS("ADD", 0, -1);
    OPTEST_3_COMPONENTS("ADDO", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDO", -1, 1);
    OPTEST_3_COMPONENTS("ADDO", -1, -1);
    OPTEST_3_COMPONENTS("ADDO", 1, 0);
    OPTEST_3_COMPONENTS("ADDO", 0, -1);
    OPTEST_3_COMPONENTS("ADDE", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDE", -1, 1);
    OPTEST_3_COMPONENTS("ADDE", -1, -1);
    OPTEST_3_COMPONENTS("ADDE", 1, 0);
    OPTEST_3_COMPONENTS("ADDE", 0, -1);
    
}
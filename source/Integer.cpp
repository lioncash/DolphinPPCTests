#include <cstdint>
#include <cstdio>
#include <string>

#include "tests.h"
#include "Utils.h"

//
// TODO: Dump the state of the condition registers as well.
//       Can help with uncovering flag setting bugs.
//

static u32 GetXER()
{
    u32 xer;
    asm volatile ("mfxer %[out]" : [out]"=b"(xer));
    return xer;
}

static void SetXER(u32 value)
{
    asm volatile ("mtxer %[val]" : : [val]"b"(value) : "xer");
}

static u32 GetCR(u32 reg)
{
    if (reg > 7)
    {
        printf("Selected condition register is out of range: %u\n", reg);
        return 0;
    }

    asm volatile ("mfcr %[out]" : [out]"=b"(reg));
    return reg;
}

static void SetCR(u32 value)
{
    asm volatile ("mtcr %[val]" : : [val]"b"(value) : "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7");
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

// Test for a 3-component instruction, where the third component is an immediate.
#define OPTEST_3_COMPONENTS_IMM(inst, rA, imm)                                                             \
{                                                                                                          \
    u32 output;                                                                                            \
    u32 ra = rA;                                                                                           \
                                                                                                           \
    asm volatile ("mtxer %[val]" : : [val]"b"(0) : "xer");                                                 \
    asm volatile (inst " %[out], %[Ra], %[Imm]" : [out]"=&r"(output) : [Ra]"r"(ra), [Imm]"i"(imm));        \
                                                                                                           \
    printf("%-8s :: rD 0x%08X | rA 0x%08X | imm 0x%08X | XER: 0x%08X\n", inst, output, rA, imm, GetXER()); \
}

void PPCIntegerTests()
{
    printf("ADD\n");
    OPTEST_3_COMPONENTS("ADD", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADD", -1, 1);
    OPTEST_3_COMPONENTS("ADD", -1, -1);
    OPTEST_3_COMPONENTS("ADD", 1, 0);
    OPTEST_3_COMPONENTS("ADD", 0, -1);
    OPTEST_3_COMPONENTS("ADD.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADD.", -1, 1);
    OPTEST_3_COMPONENTS("ADD.", -1, -1);
    OPTEST_3_COMPONENTS("ADD.", 1, 0);
    OPTEST_3_COMPONENTS("ADD.", 0, -1);
    OPTEST_3_COMPONENTS("ADDC", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDC", -1, 1);
    OPTEST_3_COMPONENTS("ADDC", -1, -1);
    OPTEST_3_COMPONENTS("ADDC", 1, 0);
    OPTEST_3_COMPONENTS("ADDC", 0, -1);
    OPTEST_3_COMPONENTS("ADDC.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDC.", -1, 1);
    OPTEST_3_COMPONENTS("ADDC.", -1, -1);
    OPTEST_3_COMPONENTS("ADDC.", 1, 0);
    OPTEST_3_COMPONENTS("ADDC.", 0, -1);
    OPTEST_3_COMPONENTS("ADDCO", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDCO", -1, 1);
    OPTEST_3_COMPONENTS("ADDCO", -1, -1);
    OPTEST_3_COMPONENTS("ADDCO", 1, 0);
    OPTEST_3_COMPONENTS("ADDCO", 0, -1);
    OPTEST_3_COMPONENTS("ADDCO.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDCO.", -1, 1);
    OPTEST_3_COMPONENTS("ADDCO.", -1, -1);
    OPTEST_3_COMPONENTS("ADDCO.", 1, 0);
    OPTEST_3_COMPONENTS("ADDCO.", 0, -1);
    OPTEST_3_COMPONENTS("ADDO", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDO", -1, 1);
    OPTEST_3_COMPONENTS("ADDO", -1, -1);
    OPTEST_3_COMPONENTS("ADDO", 1, 0);
    OPTEST_3_COMPONENTS("ADDO", 0, -1);
    OPTEST_3_COMPONENTS("ADDO.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDO.", -1, 1);
    OPTEST_3_COMPONENTS("ADDO.", -1, -1);
    OPTEST_3_COMPONENTS("ADDO.", 1, 0);
    OPTEST_3_COMPONENTS("ADDO.", 0, -1);
    OPTEST_3_COMPONENTS("ADDE", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDE", -1, 1);
    OPTEST_3_COMPONENTS("ADDE", -1, -1);
    OPTEST_3_COMPONENTS("ADDE", 1, 0);
    OPTEST_3_COMPONENTS("ADDE", 0, -1);
    OPTEST_3_COMPONENTS("ADDE.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDE.", -1, 1);
    OPTEST_3_COMPONENTS("ADDE.", -1, -1);
    OPTEST_3_COMPONENTS("ADDE.", 1, 0);
    OPTEST_3_COMPONENTS("ADDE.", 0, -1);
    OPTEST_3_COMPONENTS("ADDEO", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDEO", -1, 1);
    OPTEST_3_COMPONENTS("ADDEO", -1, -1);
    OPTEST_3_COMPONENTS("ADDEO", 1, 0);
    OPTEST_3_COMPONENTS("ADDEO", 0, -1);
    OPTEST_3_COMPONENTS("ADDEO.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ADDEO.", -1, 1);
    OPTEST_3_COMPONENTS("ADDEO.", -1, -1);
    OPTEST_3_COMPONENTS("ADDEO.", 1, 0);
    OPTEST_3_COMPONENTS("ADDEO.", 0, -1);
    OPTEST_3_COMPONENTS_IMM("ADDI", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("ADDI", -1, 1);
    OPTEST_3_COMPONENTS_IMM("ADDI", -1, -1);
    OPTEST_3_COMPONENTS_IMM("ADDI", 1, 0);
    OPTEST_3_COMPONENTS_IMM("ADDI", 0, -1);
    OPTEST_3_COMPONENTS_IMM("ADDIC", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("ADDIC", -1, 1);
    OPTEST_3_COMPONENTS_IMM("ADDIC", -1, -1);
    OPTEST_3_COMPONENTS_IMM("ADDIC", 1, 0);
    OPTEST_3_COMPONENTS_IMM("ADDIC", 0, -1);
    OPTEST_3_COMPONENTS_IMM("ADDIS", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("ADDIS", -1, 1);
    OPTEST_3_COMPONENTS_IMM("ADDIS", -1, -1);
    OPTEST_3_COMPONENTS_IMM("ADDIS", 1, 0);
    OPTEST_3_COMPONENTS_IMM("ADDIS", 0, -1);
    OPTEST_2_COMPONENTS("ADDME", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDME", -1);
    OPTEST_2_COMPONENTS("ADDME", 1);
    OPTEST_2_COMPONENTS("ADDME", 0);
    OPTEST_2_COMPONENTS("ADDME.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDME.", -1);
    OPTEST_2_COMPONENTS("ADDME.", 1);
    OPTEST_2_COMPONENTS("ADDME.", 0);

// These manage to crash dolphin on the JIT.
// On the interpreter it fires panic alerts but does not crash.
// Guess overflow flags aren't implemented or something.
#if 0
    OPTEST_2_COMPONENTS("ADDMEO", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDMEO", -1);
    OPTEST_2_COMPONENTS("ADDMEO", 1);
    OPTEST_2_COMPONENTS("ADDMEO", 0);
    OPTEST_2_COMPONENTS("ADDMEO.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDMEO.", -1);
    OPTEST_2_COMPONENTS("ADDMEO.", 1);
    OPTEST_2_COMPONENTS("ADDMEO.", 0);
#endif
    OPTEST_2_COMPONENTS("ADDZE", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDZE", -1);
    OPTEST_2_COMPONENTS("ADDZE", 1);
    OPTEST_2_COMPONENTS("ADDZE", 0);
    OPTEST_2_COMPONENTS("ADDZE.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDZE.", -1);
    OPTEST_2_COMPONENTS("ADDZE.", 1);
    OPTEST_2_COMPONENTS("ADDZE.", 0);
    OPTEST_2_COMPONENTS("ADDZEO", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDZEO", -1);
    OPTEST_2_COMPONENTS("ADDZEO", 1);
    OPTEST_2_COMPONENTS("ADDZEO", 0);
    OPTEST_2_COMPONENTS("ADDZEO.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDZEO.", -1);
    OPTEST_2_COMPONENTS("ADDZEO.", 1);
    OPTEST_2_COMPONENTS("ADDZEO.", 0);

    printf("\nAND\n");
    OPTEST_3_COMPONENTS("AND", 0, 0);
    OPTEST_3_COMPONENTS("AND", 0, 1);
    OPTEST_3_COMPONENTS("AND", 1, 1);
    OPTEST_3_COMPONENTS("AND", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("AND", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("AND.", 0, 0);
    OPTEST_3_COMPONENTS("AND.", 0, 1);
    OPTEST_3_COMPONENTS("AND.", 1, 1);
    OPTEST_3_COMPONENTS("AND.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("AND.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ANDC", 0, 0);
    OPTEST_3_COMPONENTS("ANDC", 0, 1);
    OPTEST_3_COMPONENTS("ANDC", 1, 1);
    OPTEST_3_COMPONENTS("ANDC", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("ANDC", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("ANDC.", 0, 0);
    OPTEST_3_COMPONENTS("ANDC.", 0, 1);
    OPTEST_3_COMPONENTS("ANDC.", 1, 1);
    OPTEST_3_COMPONENTS("ANDC.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("ANDC.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("ANDI.", 0, 0);
    OPTEST_3_COMPONENTS_IMM("ANDI.", 0, 1);
    OPTEST_3_COMPONENTS_IMM("ANDI.", 1, 1);
    OPTEST_3_COMPONENTS_IMM("ANDI.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS_IMM("ANDI.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("ANDIS.", 0, 0);
    OPTEST_3_COMPONENTS_IMM("ANDIS.", 0, 1);
    OPTEST_3_COMPONENTS_IMM("ANDIS.", 1, 1);
    OPTEST_3_COMPONENTS_IMM("ANDIS.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS_IMM("ANDIS.", 0xFFFFFFFF, 1);
}

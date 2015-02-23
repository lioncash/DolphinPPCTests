#include <cstdint>
#include <cstdio>

#include "Tests.h"

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

// Test for a 2-component instruction
// e.g. ADDME rD, rA
#define OPTEST_2_COMPONENTS(inst, rA)                                        \
{                                                                            \
    u32 output;                                                              \
                                                                             \
    SetXER(0);                                                               \
    SetCR(0);                                                                \
    asm volatile (inst " %[out], %[Ra]": [out]"=&r"(output) : [Ra]"r"(rA));  \
                                                                             \
    printf("%-8s :: rD 0x%08X | rA 0x%08X | XER: 0x%08X | CR: 0x%08X\n",     \
           inst, output, rA, GetXER(), GetCR());                             \
}

// Test for a 3-component instruction
// e.g. ADD rD, rA, rB
#define OPTEST_3_COMPONENTS(inst, rA, rB)                                                       \
{                                                                                               \
    u32 output;                                                                                 \
                                                                                                \
    SetCR(0);                                                                                   \
    SetXER(0);                                                                                  \
    asm volatile (inst " %[out], %[Ra], %[Rb]": [out]"=&r"(output) : [Ra]"r"(rA), [Rb]"r"(rB)); \
                                                                                                \
    printf("%-8s :: rD 0x%08X | rA 0x%08X | rB 0x%08X | XER: 0x%08X | CR: 0x%08X\n",            \
           inst, output, rA, rB, GetXER(), GetCR());                                            \
}

// Test for a 3-component instruction, where the third component is an immediate.
#define OPTEST_3_COMPONENTS_IMM(inst, rA, imm)                                                     \
{                                                                                                  \
    u32 output;                                                                                    \
                                                                                                   \
    SetCR(0);                                                                                      \
    SetXER(0);                                                                                     \
    asm volatile (inst " %[out], %[Ra], %[Imm]": [out]"=&r"(output) : [Ra]"r"(rA), [Imm]"i"(imm)); \
                                                                                                   \
    printf("%-8s :: rD 0x%08X | rA 0x%08X | imm 0x%08X | XER: 0x%08X | CR: 0x%08X\n",              \
           inst, output, rA, imm, GetXER(), GetCR());                                              \
}

// Used for testing the CMP instructions.
// Stores result to cr0.
#define OPTEST_3_COMPONENTS_CMP(inst, rA, rB)                             \
{                                                                         \
    SetCR(0);                                                             \
    SetXER(0);                                                            \
    asm volatile (inst " cr0, %[Ra], %[Rb]": : [Ra]"r"(rA), [Rb]"r"(rB)); \
                                                                          \
    printf("%-8s :: rA 0x%08X | rB 0x%08X | XER: 0x%08X | CR: 0x%08X\n",  \
           inst, rA, rB, GetXER(), GetCR());                              \
}

// Used for testing the immediate variants of CMP.
#define OPTEST_3_COMPONENTS_CMP_IMM(inst, rA, imm)                           \
{                                                                            \
    SetCR(0);                                                                \
    SetXER(0);                                                               \
    asm volatile (inst " cr0, %[Ra], %[Imm]": : [Ra]"r"(rA), [Imm]"i"(imm)); \
                                                                             \
    printf("%-8s :: rA 0x%08X | imm 0x%08X | XER: 0x%08X | CR: 0x%08X\n",    \
           inst, rA, imm, GetXER(), GetCR());                                \
}

// Test for a 5-component instruction (sets the rD before the operation).
// e.g. RLWIMI rA, rS, SH, MB, ME
#define OPTEST_5_COMPONENTS(inst, rA, rS, SH, MB, ME)                                                          \
{                                                                                                              \
    u32 output = rA;                                                                                           \
                                                                                                               \
    SetCR(0);                                                                                                  \
    SetXER(0);                                                                                                 \
    asm volatile (inst " %[out], %[Rs], %[Sh], %[Mb], %[Me]"                                                   \
        : [out]"+&r"(output)                                                                                   \
        : [Rs]"r"(rS), [Sh]"r"(SH), [Mb]"r"(MB), [Me]"r"(ME));                                                 \
                                                                                                               \
    printf("%-8s :: rD 0x%08X | rS 0x%08X | SH 0x%08X | MB: 0x%08X | ME: 0x%08X | XER: 0x%08X | CR: 0x%08X\n", \
           inst, output, rS, SH, MB, ME, GetXER(), GetCR());                                                   \
}

void PPCIntegerTests()
{
    printf("ADD Variants\n");
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
    OPTEST_2_COMPONENTS("ADDMEO", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDMEO", -1);
    OPTEST_2_COMPONENTS("ADDMEO", 1);
    OPTEST_2_COMPONENTS("ADDMEO", 0);
    OPTEST_2_COMPONENTS("ADDMEO.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("ADDMEO.", -1);
    OPTEST_2_COMPONENTS("ADDMEO.", 1);
    OPTEST_2_COMPONENTS("ADDMEO.", 0);
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

    printf("\nAND Variants\n");
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

    printf("\nCMP Variants\n");
    OPTEST_3_COMPONENTS_CMP("CMP", 0, 0);
    OPTEST_3_COMPONENTS_CMP("CMP", 0, 1);
    OPTEST_3_COMPONENTS_CMP("CMP", 1, 0);
    OPTEST_3_COMPONENTS_CMP("CMP", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS_CMP("CMP", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPI", 0, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPI", 0, 1);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPI", 1, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPI", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPI", 0x2FFF, 0x1FFF);
    OPTEST_3_COMPONENTS_CMP("CMPL", 0, 0);
    OPTEST_3_COMPONENTS_CMP("CMPL", 0, 1);
    OPTEST_3_COMPONENTS_CMP("CMPL", 1, 0);
    OPTEST_3_COMPONENTS_CMP("CMPL", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS_CMP("CMPL", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPLI", 0, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPLI", 0, 1);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPLI", 1, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPLI", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS_CMP_IMM("CMPLI", 0x2FFF, 0x1FFF);

    printf("\nCNTLZW Variants\n");
    for (int i = 0; i < 32; i++)
    {
        OPTEST_2_COMPONENTS("CNTLZW", (1 << i));
    }
    for (int i = 0; i < 32; i++)
    {
        OPTEST_2_COMPONENTS("CNTLZW.", (1 << i));
    }

    //
    // TODO: Add tests for condition register instructions
    //       such as cror, crand, etc.
    //

    printf("\nDIVW Variants\n");
    OPTEST_3_COMPONENTS("DIVW", 0, 1);
    OPTEST_3_COMPONENTS("DIVW", 1, 1);
    OPTEST_3_COMPONENTS("DIVW", 10, 2);
    OPTEST_3_COMPONENTS("DIVW", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVW", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVW", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVW", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVW.", 0, 1);
    OPTEST_3_COMPONENTS("DIVW.", 1, 1);
    OPTEST_3_COMPONENTS("DIVW.", 10, 2);
    OPTEST_3_COMPONENTS("DIVW.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVW.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVW.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVW.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVWO", 0, 1);
    OPTEST_3_COMPONENTS("DIVWO", 1, 1);
    OPTEST_3_COMPONENTS("DIVWO", 10, 2);
    OPTEST_3_COMPONENTS("DIVWO", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVWO", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVWO", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVWO", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVWO.", 0, 1);
    OPTEST_3_COMPONENTS("DIVWO.", 1, 1);
    OPTEST_3_COMPONENTS("DIVWO.", 10, 2);
    OPTEST_3_COMPONENTS("DIVWO.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVWO.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVWO.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVWO.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVWU", 0, 1);
    OPTEST_3_COMPONENTS("DIVWU", 1, 1);
    OPTEST_3_COMPONENTS("DIVWU", 10, 2);
    OPTEST_3_COMPONENTS("DIVWU", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVWU", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVWU", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVWU", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVWU.", 0, 1);
    OPTEST_3_COMPONENTS("DIVWU.", 1, 1);
    OPTEST_3_COMPONENTS("DIVWU.", 10, 2);
    OPTEST_3_COMPONENTS("DIVWU.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVWU.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVWU.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVWU.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVWUO", 0, 1);
    OPTEST_3_COMPONENTS("DIVWUO", 1, 1);
    OPTEST_3_COMPONENTS("DIVWUO", 10, 2);
    OPTEST_3_COMPONENTS("DIVWUO", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVWUO", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVWUO", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVWUO", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("DIVWUO.", 0, 1);
    OPTEST_3_COMPONENTS("DIVWUO.", 1, 1);
    OPTEST_3_COMPONENTS("DIVWUO.", 10, 2);
    OPTEST_3_COMPONENTS("DIVWUO.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("DIVWUO.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("DIVWUO.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("DIVWUO.", 0xFFFFFFFF, 0xFFFFFFFF);

    //
    // TODO: Tests for ECIWX, ECOWX, and (old McDonald had a farm) EIEIO
    //

    printf("\nEQV Variants\n");
    OPTEST_3_COMPONENTS("EQV", 0, 0);
    OPTEST_3_COMPONENTS("EQV", 1, 1);
    OPTEST_3_COMPONENTS("EQV", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("EQV", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("EQV.", 0, 0);
    OPTEST_3_COMPONENTS("EQV.", 1, 1);
    OPTEST_3_COMPONENTS("EQV.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("EQV.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("\nEXTSB Variants\n");
    OPTEST_2_COMPONENTS("EXTSB", 0);
    OPTEST_2_COMPONENTS("EXTSB", 1);
    OPTEST_2_COMPONENTS("EXTSB", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("EXTSB", 0x80000000);
    OPTEST_2_COMPONENTS("EXTSB", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("EXTSB.", 0);
    OPTEST_2_COMPONENTS("EXTSB.", 1);
    OPTEST_2_COMPONENTS("EXTSB.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("EXTSB.", 0x80000000);
    OPTEST_2_COMPONENTS("EXTSB.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("EXTSH", 0);
    OPTEST_2_COMPONENTS("EXTSH", 1);
    OPTEST_2_COMPONENTS("EXTSH", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("EXTSH", 0x80000000);
    OPTEST_2_COMPONENTS("EXTSH", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("EXTSH.", 0);
    OPTEST_2_COMPONENTS("EXTSH.", 1);
    OPTEST_2_COMPONENTS("EXTSH.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("EXTSH.", 0x80000000);
    OPTEST_2_COMPONENTS("EXTSH.", 0xFFFFFFFF);

    //
    // TODO: Tests for load instructions.
    //

    //
    // TODO: Tests for "Move to [x]" instructions.
    //

    printf("\nMULHW Variants\n");
    OPTEST_3_COMPONENTS("MULHW", 0, 0);
    OPTEST_3_COMPONENTS("MULHW", 50, 50);
    OPTEST_3_COMPONENTS("MULHW", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULHW", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULHW", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULHW", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULHW", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("MULHW.", 0, 0);
    OPTEST_3_COMPONENTS("MULHW.", 50, 50);
    OPTEST_3_COMPONENTS("MULHW.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULHW.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULHW.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULHW.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULHW.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("MULHWU", 0, 0);
    OPTEST_3_COMPONENTS("MULHWU", 50, 50);
    OPTEST_3_COMPONENTS("MULHWU", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULHWU", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULHWU", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULHWU", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULHWU", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("MULHWU.", 0, 0);
    OPTEST_3_COMPONENTS("MULHWU.", 50, 50);
    OPTEST_3_COMPONENTS("MULHWU.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULHWU.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULHWU.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULHWU.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULHWU.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("\nMULLI\n");
    OPTEST_3_COMPONENTS_IMM("MULLI", 0, 0);
    OPTEST_3_COMPONENTS_IMM("MULLI", 50, 50);
    OPTEST_3_COMPONENTS_IMM("MULLI", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("MULLI", 0xFFFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("MULLI", 0x7FFFFFFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("MULLI", 0x80000000, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("MULLI", 0xFFFFFFFF, 0x7FFF);

    printf("\nMULLW Variants\n");
    OPTEST_3_COMPONENTS("MULLW", 0, 0);
    OPTEST_3_COMPONENTS("MULLW", 50, 50);
    OPTEST_3_COMPONENTS("MULLW", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULLW", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULLW", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULLW", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULLW", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("MULLW.", 0, 0);
    OPTEST_3_COMPONENTS("MULLW.", 50, 50);
    OPTEST_3_COMPONENTS("MULLW.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULLW.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULLW.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULLW.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULLW.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("MULLWO", 0, 0);
    OPTEST_3_COMPONENTS("MULLWO", 50, 50);
    OPTEST_3_COMPONENTS("MULLWO", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULLWO", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULLWO", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULLWO", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULLWO", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("MULLWO.", 0, 0);
    OPTEST_3_COMPONENTS("MULLWO.", 50, 50);
    OPTEST_3_COMPONENTS("MULLWO.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("MULLWO.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("MULLWO.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("MULLWO.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("MULLWO.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("\nNAND Variants\n");
    OPTEST_3_COMPONENTS("NAND", 0, 0);
    OPTEST_3_COMPONENTS("NAND", 0, 1);
    OPTEST_3_COMPONENTS("NAND", 1, 0);
    OPTEST_3_COMPONENTS("NAND", 1, 1);
    OPTEST_3_COMPONENTS("NAND", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("NAND", 0x8000, 0x8000);
    OPTEST_3_COMPONENTS("NAND", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("NAND", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("NAND", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("NAND", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("NAND.", 0, 0);
    OPTEST_3_COMPONENTS("NAND.", 0, 1);
    OPTEST_3_COMPONENTS("NAND.", 1, 0);
    OPTEST_3_COMPONENTS("NAND.", 1, 1);
    OPTEST_3_COMPONENTS("NAND.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("NAND.", 0x8000, 0x8000);
    OPTEST_3_COMPONENTS("NAND.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("NAND.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("NAND.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("NAND.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("\nNEG Variants\n");
    OPTEST_2_COMPONENTS("NEG", 0);
    OPTEST_2_COMPONENTS("NEG", 1);
    OPTEST_2_COMPONENTS("NEG", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("NEG", 0x80000000);
    OPTEST_2_COMPONENTS("NEG", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("NEG.", 0);
    OPTEST_2_COMPONENTS("NEG.", 1);
    OPTEST_2_COMPONENTS("NEG.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("NEG.", 0x80000000);
    OPTEST_2_COMPONENTS("NEG.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("NEGO", 0);
    OPTEST_2_COMPONENTS("NEGO", 1);
    OPTEST_2_COMPONENTS("NEGO", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("NEGO", 0x80000000);
    OPTEST_2_COMPONENTS("NEGO", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("NEGO.", 0);
    OPTEST_2_COMPONENTS("NEGO.", 1);
    OPTEST_2_COMPONENTS("NEGO.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("NEGO.", 0x80000000);
    OPTEST_2_COMPONENTS("NEGO.", 0xFFFFFFFF);

    printf("\nNOR Variants\n");
    OPTEST_3_COMPONENTS("NOR", 0, 0);
    OPTEST_3_COMPONENTS("NOR", 0, 1);
    OPTEST_3_COMPONENTS("NOR", 1, 0);
    OPTEST_3_COMPONENTS("NOR", 1, 1);
    OPTEST_3_COMPONENTS("NOR", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("NOR", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("NOR", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("NOR.", 0, 0);
    OPTEST_3_COMPONENTS("NOR.", 0, 1);
    OPTEST_3_COMPONENTS("NOR.", 1, 0);
    OPTEST_3_COMPONENTS("NOR.", 1, 1);
    OPTEST_3_COMPONENTS("NOR.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("NOR.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("NOR.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("\nOR Variants\n");
    OPTEST_3_COMPONENTS("OR", 0, 0);
    OPTEST_3_COMPONENTS("OR", 0, 1);
    OPTEST_3_COMPONENTS("OR", 1, 0);
    OPTEST_3_COMPONENTS("OR", 1, 1);
    OPTEST_3_COMPONENTS("OR", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("OR", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("OR", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("OR.", 0, 0);
    OPTEST_3_COMPONENTS("OR.", 0, 1);
    OPTEST_3_COMPONENTS("OR.", 1, 0);
    OPTEST_3_COMPONENTS("OR.", 1, 1);
    OPTEST_3_COMPONENTS("OR.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("OR.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("OR.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("ORC", 0, 0);
    OPTEST_3_COMPONENTS("ORC", 0, 1);
    OPTEST_3_COMPONENTS("ORC", 1, 0);
    OPTEST_3_COMPONENTS("ORC", 1, 1);
    OPTEST_3_COMPONENTS("ORC", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("ORC", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("ORC", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("ORC.", 0, 0);
    OPTEST_3_COMPONENTS("ORC.", 0, 1);
    OPTEST_3_COMPONENTS("ORC.", 1, 0);
    OPTEST_3_COMPONENTS("ORC.", 1, 1);
    OPTEST_3_COMPONENTS("ORC.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("ORC.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("ORC.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS_IMM("ORI", 0, 0);
    OPTEST_3_COMPONENTS_IMM("ORI", 0, 1);
    OPTEST_3_COMPONENTS_IMM("ORI", 1, 0);
    OPTEST_3_COMPONENTS_IMM("ORI", 1, 1);
    OPTEST_3_COMPONENTS_IMM("ORI", 1, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("ORI", 1, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("ORI", 1, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("ORIS", 0, 0);
    OPTEST_3_COMPONENTS_IMM("ORIS", 0, 1);
    OPTEST_3_COMPONENTS_IMM("ORIS", 1, 0);
    OPTEST_3_COMPONENTS_IMM("ORIS", 1, 1);
    OPTEST_3_COMPONENTS_IMM("ORIS", 1, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("ORIS", 1, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("ORIS", 1, 0x7FFF);

    printf("\nRL[x] Variants\n");
    for (int i = 0; i <= 31; i++)
    {
        OPTEST_5_COMPONENTS("RLWIMI", 0x7FFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWIMI", 0x80000000, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWIMI", 0xFFFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWIMI", 0x7FFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWIMI", 0x80000000, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWIMI", 0xFFFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWIMI", 0x7FFFFFFF, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWIMI", 0x80000000, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWIMI", 0xFFFFFFFF, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWIMI.", 0x7FFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWIMI.", 0x80000000, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWIMI.", 0xFFFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWIMI.", 0x7FFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWIMI.", 0x80000000, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWIMI.", 0xFFFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWIMI.", 0x7FFFFFFF, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWIMI.", 0x80000000, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWIMI.", 0xFFFFFFFF, 30, i, 20, 30);
    }
    for (int i = 0; i <= 31; i++)
    {
        OPTEST_5_COMPONENTS("RLWINM", 0x7FFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWINM", 0x80000000, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWINM", 0xFFFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWINM", 0x7FFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWINM", 0x80000000, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWINM", 0xFFFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWINM", 0x7FFFFFFF, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWINM", 0x80000000, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWINM", 0xFFFFFFFF, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWINM.", 0x7FFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWINM.", 0x80000000, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWINM.", 0xFFFFFFFF, 30, i, 0, 10);
        OPTEST_5_COMPONENTS("RLWINM.", 0x7FFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWINM.", 0x80000000, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWINM.", 0xFFFFFFFF, 30, i, 10, 20);
        OPTEST_5_COMPONENTS("RLWINM.", 0x7FFFFFFF, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWINM.", 0x80000000, 30, i, 20, 30);
        OPTEST_5_COMPONENTS("RLWINM.", 0xFFFFFFFF, 30, i, 20, 30);
    }

    printf("\nShift Variants\n");
    // Shifts greater than 31 for 32-bit should produce zero.
    for (int i = 0; i <= 32; i++)
    {
        OPTEST_3_COMPONENTS("SLW", i, i);
        OPTEST_3_COMPONENTS("SLW", 1, i);
        OPTEST_3_COMPONENTS("SLW", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SLW", 0xFFFFFFFF, i);
        OPTEST_3_COMPONENTS("SLW.", i, i);
        OPTEST_3_COMPONENTS("SLW.", 1, i);
        OPTEST_3_COMPONENTS("SLW.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SLW.", 0xFFFFFFFF, i);
    }
    for (int i = 0; i <= 32; i++)
    {
        OPTEST_3_COMPONENTS("SRAW", i, i);
        OPTEST_3_COMPONENTS("SRAW", 1, i);
        OPTEST_3_COMPONENTS("SRAW", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SRAW", 0xFFFFFFFF, i);
        OPTEST_3_COMPONENTS("SRAW.", i, i);
        OPTEST_3_COMPONENTS("SRAW.", 1, i);
        OPTEST_3_COMPONENTS("SRAW.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SRAW.", 0xFFFFFFFF, i);
    }
    for (int i = 0; i <= 32; i++)
    {
        OPTEST_3_COMPONENTS("SRAWI", i, i);
        OPTEST_3_COMPONENTS("SRAWI", 1, i);
        OPTEST_3_COMPONENTS("SRAWI", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SRAWI", 0xFFFFFFFF, i);
        OPTEST_3_COMPONENTS("SRAWI.", i, i);
        OPTEST_3_COMPONENTS("SRAWI.", 1, i);
        OPTEST_3_COMPONENTS("SRAWI.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SRAWI.", 0xFFFFFFFF, i);
    }
    for (int i = 0; i <= 32; i++)
    {
        OPTEST_3_COMPONENTS("SRW", i, i);
        OPTEST_3_COMPONENTS("SRW", 1, i);
        OPTEST_3_COMPONENTS("SRW", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SRW", 0xFFFFFFFF, i);
        OPTEST_3_COMPONENTS("SRW.", i, i);
        OPTEST_3_COMPONENTS("SRW.", 1, i);
        OPTEST_3_COMPONENTS("SRW.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("SRW.", 0xFFFFFFFF, i);
    }

    //
    // TODO: Tests for storage and string instructions.
    //

    printf("\nSUBF Variants\n");
    OPTEST_3_COMPONENTS("SUBF", 0, 0);
    OPTEST_3_COMPONENTS("SUBF", 0, 1);
    OPTEST_3_COMPONENTS("SUBF", 1, 0);
    OPTEST_3_COMPONENTS("SUBF", 1, 1);
    OPTEST_3_COMPONENTS("SUBF", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBF", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBF.", 0, 0);
    OPTEST_3_COMPONENTS("SUBF.", 0, 1);
    OPTEST_3_COMPONENTS("SUBF.", 1, 0);
    OPTEST_3_COMPONENTS("SUBF.", 1, 1);
    OPTEST_3_COMPONENTS("SUBF.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBF.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFO", 0, 0);
    OPTEST_3_COMPONENTS("SUBFO", 0, 1);
    OPTEST_3_COMPONENTS("SUBFO", 1, 0);
    OPTEST_3_COMPONENTS("SUBFO", 1, 1);
    OPTEST_3_COMPONENTS("SUBFO", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFO", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFO.", 0, 0);
    OPTEST_3_COMPONENTS("SUBFO.", 0, 1);
    OPTEST_3_COMPONENTS("SUBFO.", 1, 0);
    OPTEST_3_COMPONENTS("SUBFO.", 1, 1);
    OPTEST_3_COMPONENTS("SUBFO.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFO.", 0xFFFFFFFF, 0xFFFFFFFF);

    //
    // TODO: Make this a loop, but alternate the carry flag for SUBFC
    //
    OPTEST_3_COMPONENTS("SUBFC", 0, 0);
    OPTEST_3_COMPONENTS("SUBFC", 0, 1);
    OPTEST_3_COMPONENTS("SUBFC", 1, 0);
    OPTEST_3_COMPONENTS("SUBFC", 1, 1);
    OPTEST_3_COMPONENTS("SUBFC", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFC", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFC.", 0, 0);
    OPTEST_3_COMPONENTS("SUBFC.", 0, 1);
    OPTEST_3_COMPONENTS("SUBFC.", 1, 0);
    OPTEST_3_COMPONENTS("SUBFC.", 1, 1);
    OPTEST_3_COMPONENTS("SUBFC.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFC.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFCO", 0, 0);
    OPTEST_3_COMPONENTS("SUBFCO", 0, 1);
    OPTEST_3_COMPONENTS("SUBFCO", 1, 0);
    OPTEST_3_COMPONENTS("SUBFCO", 1, 1);
    OPTEST_3_COMPONENTS("SUBFCO", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFCO", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFCO.", 0, 0);
    OPTEST_3_COMPONENTS("SUBFCO.", 0, 1);
    OPTEST_3_COMPONENTS("SUBFCO.", 1, 0);
    OPTEST_3_COMPONENTS("SUBFCO.", 1, 1);
    OPTEST_3_COMPONENTS("SUBFCO.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFCO.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFE", 0, 0);
    OPTEST_3_COMPONENTS("SUBFE", 0, 1);
    OPTEST_3_COMPONENTS("SUBFE", 1, 0);
    OPTEST_3_COMPONENTS("SUBFE", 1, 1);
    OPTEST_3_COMPONENTS("SUBFE", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFE", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFE.", 0, 0);
    OPTEST_3_COMPONENTS("SUBFE.", 0, 1);
    OPTEST_3_COMPONENTS("SUBFE.", 1, 0);
    OPTEST_3_COMPONENTS("SUBFE.", 1, 1);
    OPTEST_3_COMPONENTS("SUBFE.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFE.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFEO", 0, 0);
    OPTEST_3_COMPONENTS("SUBFEO", 0, 1);
    OPTEST_3_COMPONENTS("SUBFEO", 1, 0);
    OPTEST_3_COMPONENTS("SUBFEO", 1, 1);
    OPTEST_3_COMPONENTS("SUBFEO", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFEO", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFEO.", 0, 0);
    OPTEST_3_COMPONENTS("SUBFEO.", 0, 1);
    OPTEST_3_COMPONENTS("SUBFEO.", 1, 0);
    OPTEST_3_COMPONENTS("SUBFEO.", 1, 1);
    OPTEST_3_COMPONENTS("SUBFEO.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("SUBFEO.", 0xFFFFFFFF, 0xFFFFFFFF);

    OPTEST_3_COMPONENTS_IMM("SUBFIC", 0, 0);
    OPTEST_3_COMPONENTS_IMM("SUBFIC", 0, 1);
    OPTEST_3_COMPONENTS_IMM("SUBFIC", 1, 0);
    OPTEST_3_COMPONENTS_IMM("SUBFIC", 1, 1);
    OPTEST_3_COMPONENTS_IMM("SUBFIC", 0x7FFFFFFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("SUBFIC", 0xFFFFFFFF, 0x7FFF);

    OPTEST_2_COMPONENTS("SUBFME", 0);
    OPTEST_2_COMPONENTS("SUBFME", 1);
    OPTEST_2_COMPONENTS("SUBFME", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFME", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFME", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("SUBFME.", 0);
    OPTEST_2_COMPONENTS("SUBFME.", 1);
    OPTEST_2_COMPONENTS("SUBFME.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFME.", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFME.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("SUBFMEO", 0);
    OPTEST_2_COMPONENTS("SUBFMEO", 1);
    OPTEST_2_COMPONENTS("SUBFMEO", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFMEO", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFMEO", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("SUBFMEO.", 0);
    OPTEST_2_COMPONENTS("SUBFMEO.", 1);
    OPTEST_2_COMPONENTS("SUBFMEO.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFMEO.", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFMEO.", 0xFFFFFFFF);

    OPTEST_2_COMPONENTS("SUBFZE", 0);
    OPTEST_2_COMPONENTS("SUBFZE", 1);
    OPTEST_2_COMPONENTS("SUBFZE", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZE", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFZE", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZE.", 0);
    OPTEST_2_COMPONENTS("SUBFZE.", 1);
    OPTEST_2_COMPONENTS("SUBFZE.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZE.", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFZE.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZEO", 0);
    OPTEST_2_COMPONENTS("SUBFZEO", 1);
    OPTEST_2_COMPONENTS("SUBFZEO", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZEO", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFZEO", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZEO.", 0);
    OPTEST_2_COMPONENTS("SUBFZEO.", 1);
    OPTEST_2_COMPONENTS("SUBFZEO.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("SUBFZEO.", 0x80000000);
    OPTEST_2_COMPONENTS("SUBFZEO.", 0xFFFFFFFF);

    //
    // TODO: Maybe do tests for SYNC/TLBIE/TLBSYNC
    // TODO: Somehow test TW/TWI
    //

    printf("\nXOR Variants\n");
    OPTEST_3_COMPONENTS("XOR", 0, 0);
    OPTEST_3_COMPONENTS("XOR", 0, 1);
    OPTEST_3_COMPONENTS("XOR", 1, 0);
    OPTEST_3_COMPONENTS("XOR", 1, 1);
    OPTEST_3_COMPONENTS("XOR", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS("XOR", 0xFFFFFFFF, 0x3FFF);
    OPTEST_3_COMPONENTS("XOR.", 0, 0);
    OPTEST_3_COMPONENTS("XOR.", 0, 1);
    OPTEST_3_COMPONENTS("XOR.", 1, 0);
    OPTEST_3_COMPONENTS("XOR.", 1, 1);
    OPTEST_3_COMPONENTS("XOR.", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS("XOR.", 0xFFFFFFFF, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("XORI", 0, 0);
    OPTEST_3_COMPONENTS_IMM("XORI", 0, 1);
    OPTEST_3_COMPONENTS_IMM("XORI", 1, 0);
    OPTEST_3_COMPONENTS_IMM("XORI", 1, 1);
    OPTEST_3_COMPONENTS_IMM("XORI", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("XORI", 0xFFFFFFFF, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("XORIS", 0, 0);
    OPTEST_3_COMPONENTS_IMM("XORIS", 0, 1);
    OPTEST_3_COMPONENTS_IMM("XORIS", 1, 0);
    OPTEST_3_COMPONENTS_IMM("XORIS", 1, 1);
    OPTEST_3_COMPONENTS_IMM("XORIS", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("XORIS", 0xFFFFFFFF, 0x3FFF);
}

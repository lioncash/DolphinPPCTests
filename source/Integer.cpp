#ifdef MACOS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>
#else
#include <cinttypes>
#include <cstdint>
#endif
#include <cstdio>

#include "Tests.h"
#include "Utils.h"

// Test for a 2-component instruction
// e.g. ADDME rD, rA
#define OPTEST_2_COMPONENTS(inst, rA)                                                                            \
{                                                                                                                \
    uint32_t output;                                                                                             \
                                                                                                                 \
    SetXER(0);                                                                                                   \
    SetCR(0);                                                                                                    \
    asm volatile (inst " %[out], %[Ra]": [out]"=&r"(output) : [Ra]"r"(rA));                                      \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                    \
                                                                                                                 \
    printf("%-8s :: rD 0x%08" PRIX32 " | rA 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",     \
           upper(inst), output, static_cast<uint32_t>(rA), xer, cr);                                             \
}

// Test for a 3-component instruction
// e.g. ADD rD, rA, rB
#define OPTEST_3_COMPONENTS(inst, rA, rB)                                                                                                    \
{                                                                                                                                            \
    uint32_t output;                                                                                                                         \
                                                                                                                                             \
    SetXER(0);                                                                                                                               \
    SetCR(0);                                                                                                                                \
    asm volatile (inst " %[out], %[Ra], %[Rb]": [out]"=&r"(output) : [Ra]"r"(rA), [Rb]"r"(rB));                                              \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                                                \
                                                                                                                                             \
    printf("%-8s :: rD 0x%08" PRIX32 " | rA 0x%08" PRIX32 " | rB 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",            \
           upper(inst), output, static_cast<uint32_t>(rA), static_cast<uint32_t>(rB), xer, cr);                                              \
}

// Test for a 3-component instruction
// e.g. SRAWI rA, rS, SH
#define OPTEST_3_COMPONENTS2(inst, rS, SH)                                                                                                   \
{                                                                                                                                            \
    uint32_t output;                                                                                                                         \
                                                                                                                                             \
    SetXER(0);                                                                                                                               \
    SetCR(0);                                                                                                                                \
    asm volatile (inst " %[out], %[Rs], " #SH "": [out]"=&r"(output) : [Rs]"r"(rS));                                                         \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                                                \
                                                                                                                                             \
    printf("%-8s :: rD 0x%08" PRIX32 " | rS 0x%08" PRIX32 " | SH 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",            \
           upper(inst), output, static_cast<uint32_t>(rS), static_cast<uint32_t>(SH), xer, cr);                                              \
}

// Test for a 3-component instruction, where the third component is an immediate.
#define OPTEST_3_COMPONENTS_IMM(inst, rA, imm)                                                                                               \
{                                                                                                                                            \
    uint32_t output;                                                                                                                         \
                                                                                                                                             \
    SetXER(0);                                                                                                                               \
    SetCR(0);                                                                                                                                \
    asm volatile (inst " %[out], %[Ra], %[Imm]": [out]"=&r"(output) : [Ra]"b"(rA), [Imm]"i"(imm));                                           \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                                                \
                                                                                                                                             \
    printf("%-8s :: rD 0x%08" PRIX32 " | rA 0x%08" PRIX32 " | imm 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",           \
           upper(inst), output, static_cast<uint32_t>(rA), static_cast<uint32_t>(imm), xer, cr);                                             \
}

// Used for testing the CMP instructions.
// Stores result to cr0.
#define OPTEST_3_COMPONENTS_CMP(inst, rA, rB)                                                                 \
{                                                                                                             \
    SetXER(0);                                                                                                \
    SetCR(0);                                                                                                 \
    asm volatile (inst " cr0, %[Ra], %[Rb]": : [Ra]"r"(rA), [Rb]"r"(rB));                                     \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                 \
                                                                                                              \
    printf("%-8s :: rA 0x%08" PRIX32 " | rB 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",  \
           upper(inst), static_cast<uint32_t>(rA), static_cast<uint32_t>(rB), xer, cr);                       \
}

// Used for testing the immediate variants of CMP.
#define OPTEST_3_COMPONENTS_CMP_IMM(inst, rA, imm)                                                               \
{                                                                                                                \
    SetXER(0);                                                                                                   \
    SetCR(0);                                                                                                    \
    asm volatile (inst " cr0, %[Ra], %[Imm]": : [Ra]"r"(rA), [Imm]"i"(imm));                                     \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                    \
                                                                                                                 \
    printf("%-8s :: rA 0x%08" PRIX32 " | imm 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",    \
           upper(inst), static_cast<uint32_t>(rA), static_cast<uint32_t>(imm), xer, cr);                         \
}

// Test for a 5-component instruction (sets the rD before the operation).
// e.g. RLWIMI rA, rS, SH, MB, ME ; rA is an input and output
//      RLWINM rA, rS, SH, MB, ME ; rA is only an output
#define OPTEST_5_COMPONENTS(inst, rA, rS, SH, MB, ME)                                                                                                                         \
{                                                                                                                                                                             \
    uint32_t output = rA;                                                                                                                                                     \
                                                                                                                                                                              \
    SetXER(0);                                                                                                                                                                \
    SetCR(0);                                                                                                                                                                 \
    asm volatile (inst " %[out], %[Rs], " #SH ", " #MB ", " #ME ""                                                                                                            \
        : [out]"+&r"(output)                                                                                                                                                  \
        : [Rs]"r"(rS));                                                                                                                                                       \
    uint32_t xer, cr; GetXER(xer); GetCR(cr);                                                                                                                                 \
                                                                                                                                                                              \
    printf("%-8s :: rD 0x%08" PRIX32 " | rA 0x%08" PRIX32 " | rS 0x%08" PRIX32 " | SH 0x%08" PRIX32 " | MB: 0x%08" PRIX32 " | ME: 0x%08" PRIX32 " | XER: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
           upper(inst), output,                                                                                                                                               \
           static_cast<uint32_t>(rA),                                                                                                                                         \
           static_cast<uint32_t>(rS),                                                                                                                                         \
           static_cast<uint32_t>(SH),                                                                                                                                         \
           static_cast<uint32_t>(MB),                                                                                                                                         \
           static_cast<uint32_t>(ME), xer, cr);                                                                                                                               \
}

static void XEROverflowClearTest()
{
    printf("XER Overflow Clear Test (OV bit should not be set)\n");

    uint32_t result = 0;
    const uint32_t x = 2;
    const uint32_t y = 2;
    uint32_t d = 0;

    SetXER(0xFFFFFFFF);
    asm volatile (
        "addo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("addo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "addco %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("addco: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "addeo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("addeo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "addmeo %[d], %[x]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x));

    printf("addmeo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "addzeo %[d], %[x]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x));

    printf("addzeo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "divwo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("divwo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "divwuo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("divwuo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "mullwo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("mullwo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "nego %[d], %[x]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x));

    printf("nego: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "subfo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("subfo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "subfco %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("subfco: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "subfeo %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("subfeo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "subfmeo %[d], %[x]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x));

    printf("subfmeo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "subfzeo %[d], %[x]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x));

    printf("subfzeo: Resulting XER: 0x%08" PRIX32 "\n", result);

    SetXER(0xFFFFFFFF);
    asm volatile (
        "add %[d], %[x], %[y]\n"
        "mfxer %[result]"
        : [d]"=r"(d), [result]"=r"(result)
        : [x]"r"(x), [y]"r"(y));

    printf("add: Resulting XER: 0x%08" PRIX32 "\n", result);
}

void PPCIntegerTests()
{
    // Run specialized tests first
    XEROverflowClearTest();

    printf("General Instruction Test\n");
    printf("ADD Variants\n");
    OPTEST_3_COMPONENTS("add", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("add", 0x80000000, 1);
    OPTEST_3_COMPONENTS("add", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("add", -1, 1);
    OPTEST_3_COMPONENTS("add", -1, -1);
    OPTEST_3_COMPONENTS("add", 1, 0);
    OPTEST_3_COMPONENTS("add", 0, -1);
    OPTEST_3_COMPONENTS("add.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("add.", 0x80000000, 1);
    OPTEST_3_COMPONENTS("add.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("add.", -1, 1);
    OPTEST_3_COMPONENTS("add.", -1, -1);
    OPTEST_3_COMPONENTS("add.", 1, 0);
    OPTEST_3_COMPONENTS("add.", 0, -1);
    OPTEST_3_COMPONENTS("addc", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addc", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addc", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addc", -1, 1);
    OPTEST_3_COMPONENTS("addc", -1, -1);
    OPTEST_3_COMPONENTS("addc", 1, 0);
    OPTEST_3_COMPONENTS("addc", 0, -1);
    OPTEST_3_COMPONENTS("addc.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addc.", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addc.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addc.", -1, 1);
    OPTEST_3_COMPONENTS("addc.", -1, -1);
    OPTEST_3_COMPONENTS("addc.", 1, 0);
    OPTEST_3_COMPONENTS("addc.", 0, -1);
    OPTEST_3_COMPONENTS("addco", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addco", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addco", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addco", -1, 1);
    OPTEST_3_COMPONENTS("addco", -1, -1);
    OPTEST_3_COMPONENTS("addco", 1, 0);
    OPTEST_3_COMPONENTS("addco", 0, -1);
    OPTEST_3_COMPONENTS("addco.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addco.", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addco.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addco.", -1, 1);
    OPTEST_3_COMPONENTS("addco.", -1, -1);
    OPTEST_3_COMPONENTS("addco.", 1, 0);
    OPTEST_3_COMPONENTS("addco.", 0, -1);
    OPTEST_3_COMPONENTS("addo", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addo", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addo", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addo", -1, 1);
    OPTEST_3_COMPONENTS("addo", -1, -1);
    OPTEST_3_COMPONENTS("addo", 1, 0);
    OPTEST_3_COMPONENTS("addo", 0, -1);
    OPTEST_3_COMPONENTS("addo.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addo.", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addo.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addo.", -1, 1);
    OPTEST_3_COMPONENTS("addo.", -1, -1);
    OPTEST_3_COMPONENTS("addo.", 1, 0);
    OPTEST_3_COMPONENTS("addo.", 0, -1);
    OPTEST_3_COMPONENTS("adde", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("adde", 0x80000000, 1);
    OPTEST_3_COMPONENTS("adde", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("adde", -1, 1);
    OPTEST_3_COMPONENTS("adde", -1, -1);
    OPTEST_3_COMPONENTS("adde", 1, 0);
    OPTEST_3_COMPONENTS("adde", 0, -1);
    OPTEST_3_COMPONENTS("adde.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("adde.", 0x80000000, 1);
    OPTEST_3_COMPONENTS("adde.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("adde.", -1, 1);
    OPTEST_3_COMPONENTS("adde.", -1, -1);
    OPTEST_3_COMPONENTS("adde.", 1, 0);
    OPTEST_3_COMPONENTS("adde.", 0, -1);
    OPTEST_3_COMPONENTS("addeo", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addeo", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addeo", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("addeo", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addeo", -1, 1);
    OPTEST_3_COMPONENTS("addeo", -1, -1);
    OPTEST_3_COMPONENTS("addeo", 1, 0);
    OPTEST_3_COMPONENTS("addeo", 0, -1);
    OPTEST_3_COMPONENTS("addeo.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS("addeo.", 0x80000000, 1);
    OPTEST_3_COMPONENTS("addeo.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("addeo.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("addeo.", -1, 1);
    OPTEST_3_COMPONENTS("addeo.", -1, -1);
    OPTEST_3_COMPONENTS("addeo.", 1, 0);
    OPTEST_3_COMPONENTS("addeo.", 0, -1);
    OPTEST_3_COMPONENTS_IMM("addi", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addi", 0x80000000, 1);
    OPTEST_3_COMPONENTS_IMM("addi", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addi", -1, 1);
    OPTEST_3_COMPONENTS_IMM("addi", -1, -1);
    OPTEST_3_COMPONENTS_IMM("addi", 1, 0);
    OPTEST_3_COMPONENTS_IMM("addi", 0, -1);
    OPTEST_3_COMPONENTS_IMM("addic", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addic", 0x80000000, 1);
    OPTEST_3_COMPONENTS_IMM("addic", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addic", -1, 1);
    OPTEST_3_COMPONENTS_IMM("addic", -1, -1);
    OPTEST_3_COMPONENTS_IMM("addic", 1, 0);
    OPTEST_3_COMPONENTS_IMM("addic", 0, -1);
    OPTEST_3_COMPONENTS_IMM("addic.", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addic.", 0x80000000, 1);
    OPTEST_3_COMPONENTS_IMM("addic.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addic.", -1, 1);
    OPTEST_3_COMPONENTS_IMM("addic.", -1, -1);
    OPTEST_3_COMPONENTS_IMM("addic.", 1, 0);
    OPTEST_3_COMPONENTS_IMM("addic.", 0, -1);
    OPTEST_3_COMPONENTS_IMM("addis", 0x7FFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addis", 0x80000000, 1);
    OPTEST_3_COMPONENTS_IMM("addis", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("addis", -1, 1);
    OPTEST_3_COMPONENTS_IMM("addis", -1, -1);
    OPTEST_3_COMPONENTS_IMM("addis", 1, 0);
    OPTEST_3_COMPONENTS_IMM("addis", 0, -1);
    OPTEST_2_COMPONENTS("addme", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addme", 0x80000000);
    OPTEST_2_COMPONENTS("addme", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addme", -1);
    OPTEST_2_COMPONENTS("addme", 1);
    OPTEST_2_COMPONENTS("addme", 0);
    OPTEST_2_COMPONENTS("addme.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addme.", 0x80000000);
    OPTEST_2_COMPONENTS("addme.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addme.", -1);
    OPTEST_2_COMPONENTS("addme.", 1);
    OPTEST_2_COMPONENTS("addme.", 0);
    OPTEST_2_COMPONENTS("addmeo", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addmeo", 0x80000000);
    OPTEST_2_COMPONENTS("addmeo", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addmeo", -1);
    OPTEST_2_COMPONENTS("addmeo", 1);
    OPTEST_2_COMPONENTS("addmeo", 0);
    OPTEST_2_COMPONENTS("addmeo.", 0x7FFFFFFF)
    OPTEST_2_COMPONENTS("addmeo.", 0x80000000)
    OPTEST_2_COMPONENTS("addmeo.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addmeo.", -1);
    OPTEST_2_COMPONENTS("addmeo.", 1);
    OPTEST_2_COMPONENTS("addmeo.", 0);
    OPTEST_2_COMPONENTS("addze", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addze", 0x80000000);
    OPTEST_2_COMPONENTS("addze", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addze", -1);
    OPTEST_2_COMPONENTS("addze", 1);
    OPTEST_2_COMPONENTS("addze", 0);
    OPTEST_2_COMPONENTS("addze.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addze.", 0x80000000);
    OPTEST_2_COMPONENTS("addze.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addze.", -1);
    OPTEST_2_COMPONENTS("addze.", 1);
    OPTEST_2_COMPONENTS("addze.", 0);
    OPTEST_2_COMPONENTS("addzeo", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addzeo", 0x80000000);
    OPTEST_2_COMPONENTS("addzeo", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addzeo", -1);
    OPTEST_2_COMPONENTS("addzeo", 1);
    OPTEST_2_COMPONENTS("addzeo", 0);
    OPTEST_2_COMPONENTS("addzeo.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("addzeo.", 0x80000000);
    OPTEST_2_COMPONENTS("addzeo.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("addzeo.", -1);
    OPTEST_2_COMPONENTS("addzeo.", 1);
    OPTEST_2_COMPONENTS("addzeo.", 0);

    printf("AND Variants\n");
    OPTEST_3_COMPONENTS("and", 0, 0);
    OPTEST_3_COMPONENTS("and", 0, 1);
    OPTEST_3_COMPONENTS("and", 1, 1);
    OPTEST_3_COMPONENTS("and", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("and", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("and.", 0, 0);
    OPTEST_3_COMPONENTS("and.", 0, 1);
    OPTEST_3_COMPONENTS("and.", 1, 1);
    OPTEST_3_COMPONENTS("and.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("and.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("andc", 0, 0);
    OPTEST_3_COMPONENTS("andc", 0, 1);
    OPTEST_3_COMPONENTS("andc", 1, 1);
    OPTEST_3_COMPONENTS("andc", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("andc", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS("andc.", 0, 0);
    OPTEST_3_COMPONENTS("andc.", 0, 1);
    OPTEST_3_COMPONENTS("andc.", 1, 1);
    OPTEST_3_COMPONENTS("andc.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS("andc.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("andi.", 0, 0);
    OPTEST_3_COMPONENTS_IMM("andi.", 0, 1);
    OPTEST_3_COMPONENTS_IMM("andi.", 1, 1);
    OPTEST_3_COMPONENTS_IMM("andi.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS_IMM("andi.", 0xFFFFFFFF, 1);
    OPTEST_3_COMPONENTS_IMM("andis.", 0, 0);
    OPTEST_3_COMPONENTS_IMM("andis.", 0, 1);
    OPTEST_3_COMPONENTS_IMM("andis.", 1, 1);
    OPTEST_3_COMPONENTS_IMM("andis.", 0xFFFFFFFF, 0);
    OPTEST_3_COMPONENTS_IMM("andis.", 0xFFFFFFFF, 1);

    printf("CMP Variants\n");
    OPTEST_3_COMPONENTS_CMP("cmp", 0, 0);
    OPTEST_3_COMPONENTS_CMP("cmp", 0, 1);
    OPTEST_3_COMPONENTS_CMP("cmp", 1, 0);
    OPTEST_3_COMPONENTS_CMP("cmp", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS_CMP("cmp", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpi", 0, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpi", 0, 1);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpi", 1, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpi", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpi", 0x2FFF, 0x1FFF);
    OPTEST_3_COMPONENTS_CMP("cmpl", 0, 0);
    OPTEST_3_COMPONENTS_CMP("cmpl", 0, 1);
    OPTEST_3_COMPONENTS_CMP("cmpl", 1, 0);
    OPTEST_3_COMPONENTS_CMP("cmpl", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS_CMP("cmpl", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpli", 0, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpli", 0, 1);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpli", 1, 0);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpli", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS_CMP_IMM("cmpli", 0x2FFF, 0x1FFF);

    printf("CNTLZW Variants\n");
    for (uint32_t i = 0; i < 32; i++)
    {
        OPTEST_2_COMPONENTS("cntlzw", (1U << i));
    }
    for (uint32_t i = 0; i < 32; i++)
    {
        OPTEST_2_COMPONENTS("cntlzw.", (1U << i));
    }

    printf("DIVW Variants\n");
    OPTEST_3_COMPONENTS("divw", 0, 0);
    OPTEST_3_COMPONENTS("divw", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divw", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divw", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divw", 0, 1);
    OPTEST_3_COMPONENTS("divw", 1, 1);
    OPTEST_3_COMPONENTS("divw", 10, 2);
    OPTEST_3_COMPONENTS("divw", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divw", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divw", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divw", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divw.", 0, 0);
    OPTEST_3_COMPONENTS("divw.", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divw.", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divw.", 0, 1);
    OPTEST_3_COMPONENTS("divw.", 1, 1);
    OPTEST_3_COMPONENTS("divw.", 10, 2);
    OPTEST_3_COMPONENTS("divw.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divw.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divw.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divw.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwo", 0, 0);
    OPTEST_3_COMPONENTS("divwo", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divwo", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwo", 0, 1);
    OPTEST_3_COMPONENTS("divwo", 1, 1);
    OPTEST_3_COMPONENTS("divwo", 10, 2);
    OPTEST_3_COMPONENTS("divwo", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divwo", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divwo", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divwo", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwo.", 0, 0);
    OPTEST_3_COMPONENTS("divwo.", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divwo.", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwo.", 0, 1);
    OPTEST_3_COMPONENTS("divwo.", 1, 1);
    OPTEST_3_COMPONENTS("divwo.", 10, 2);
    OPTEST_3_COMPONENTS("divwo.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divwo.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divwo.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divwo.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwu", 0, 0);
    OPTEST_3_COMPONENTS("divwu", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divwu", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwu", 0, 1);
    OPTEST_3_COMPONENTS("divwu", 1, 1);
    OPTEST_3_COMPONENTS("divwu", 10, 2);
    OPTEST_3_COMPONENTS("divwu", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divwu", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divwu", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divwu", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwu.", 0, 0);
    OPTEST_3_COMPONENTS("divwu.", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divwu.", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwu.", 0, 1);
    OPTEST_3_COMPONENTS("divwu.", 1, 1);
    OPTEST_3_COMPONENTS("divwu.", 10, 2);
    OPTEST_3_COMPONENTS("divwu.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divwu.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divwu.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divwu.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwuo", 0, 0);
    OPTEST_3_COMPONENTS("divwuo", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divwuo", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwuo", 0, 1);
    OPTEST_3_COMPONENTS("divwuo", 1, 1);
    OPTEST_3_COMPONENTS("divwuo", 10, 2);
    OPTEST_3_COMPONENTS("divwuo", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divwuo", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divwuo", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divwuo", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwuo.", 0, 0);
    OPTEST_3_COMPONENTS("divwuo.", 0x7FFFFFFF, 0);
    OPTEST_3_COMPONENTS("divwuo.", 0x80000000, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("divwuo.", 0, 1);
    OPTEST_3_COMPONENTS("divwuo.", 1, 1);
    OPTEST_3_COMPONENTS("divwuo.", 10, 2);
    OPTEST_3_COMPONENTS("divwuo.", 0x000001A4, 0x00000045);
    OPTEST_3_COMPONENTS("divwuo.", 0xFFFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("divwuo.", 0xFFFFFFFF, 0x80000000);
    OPTEST_3_COMPONENTS("divwuo.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("EQV Variants\n");
    OPTEST_3_COMPONENTS("eqv", 0, 0);
    OPTEST_3_COMPONENTS("eqv", 1, 1);
    OPTEST_3_COMPONENTS("eqv", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("eqv", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("eqv.", 0, 0);
    OPTEST_3_COMPONENTS("eqv.", 1, 1);
    OPTEST_3_COMPONENTS("eqv.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("eqv.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("EXTSB Variants\n");
    OPTEST_2_COMPONENTS("extsb", 0);
    OPTEST_2_COMPONENTS("extsb", 1);
    OPTEST_2_COMPONENTS("extsb", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("extsb", 0x80000000);
    OPTEST_2_COMPONENTS("extsb", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("extsb.", 0);
    OPTEST_2_COMPONENTS("extsb.", 1);
    OPTEST_2_COMPONENTS("extsb.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("extsb.", 0x80000000);
    OPTEST_2_COMPONENTS("extsb.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("extsh", 0);
    OPTEST_2_COMPONENTS("extsh", 1);
    OPTEST_2_COMPONENTS("extsh", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("extsh", 0x80000000);
    OPTEST_2_COMPONENTS("extsh", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("extsh.", 0);
    OPTEST_2_COMPONENTS("extsh.", 1);
    OPTEST_2_COMPONENTS("extsh.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("extsh.", 0x80000000);
    OPTEST_2_COMPONENTS("extsh.", 0xFFFFFFFF);

    //
    // TODO: Tests for load instructions.
    //

    //
    // TODO: Tests for "Move to [x]" instructions.
    //

    printf("MULHW Variants\n");
    OPTEST_3_COMPONENTS("mulhw", 0, 0);
    OPTEST_3_COMPONENTS("mulhw", 50, 50);
    OPTEST_3_COMPONENTS("mulhw", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mulhw", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mulhw", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mulhw", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mulhw", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("mulhw.", 0, 0);
    OPTEST_3_COMPONENTS("mulhw.", 50, 50);
    OPTEST_3_COMPONENTS("mulhw.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mulhw.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mulhw.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mulhw.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mulhw.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("mulhwu", 0, 0);
    OPTEST_3_COMPONENTS("mulhwu", 50, 50);
    OPTEST_3_COMPONENTS("mulhwu", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mulhwu", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mulhwu", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mulhwu", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mulhwu", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("mulhwu.", 0, 0);
    OPTEST_3_COMPONENTS("mulhwu.", 50, 50);
    OPTEST_3_COMPONENTS("mulhwu.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mulhwu.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mulhwu.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mulhwu.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mulhwu.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("MULLI\n");
    OPTEST_3_COMPONENTS_IMM("mulli", 0, 0);
    OPTEST_3_COMPONENTS_IMM("mulli", 50, 50);
    OPTEST_3_COMPONENTS_IMM("mulli", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("mulli", 0xFFFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("mulli", 0x7FFFFFFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("mulli", 0x80000000, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("mulli", 0xFFFFFFFF, 0x7FFF);

    printf("MULLW Variants\n");
    OPTEST_3_COMPONENTS("mullw", 0, 0);
    OPTEST_3_COMPONENTS("mullw", 50, 50);
    OPTEST_3_COMPONENTS("mullw", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mullw", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mullw", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mullw", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mullw", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("mullw.", 0, 0);
    OPTEST_3_COMPONENTS("mullw.", 50, 50);
    OPTEST_3_COMPONENTS("mullw.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mullw.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mullw.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mullw.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mullw.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("mullwo", 0, 0);
    OPTEST_3_COMPONENTS("mullwo", 50, 50);
    OPTEST_3_COMPONENTS("mullwo", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mullwo", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mullwo", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mullwo", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mullwo", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("mullwo.", 0, 0);
    OPTEST_3_COMPONENTS("mullwo.", 50, 50);
    OPTEST_3_COMPONENTS("mullwo.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("mullwo.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("mullwo.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("mullwo.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("mullwo.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("NAND Variants\n");
    OPTEST_3_COMPONENTS("nand", 0, 0);
    OPTEST_3_COMPONENTS("nand", 0, 1);
    OPTEST_3_COMPONENTS("nand", 1, 0);
    OPTEST_3_COMPONENTS("nand", 1, 1);
    OPTEST_3_COMPONENTS("nand", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("nand", 0x8000, 0x8000);
    OPTEST_3_COMPONENTS("nand", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("nand", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("nand", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("nand", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("nand.", 0, 0);
    OPTEST_3_COMPONENTS("nand.", 0, 1);
    OPTEST_3_COMPONENTS("nand.", 1, 0);
    OPTEST_3_COMPONENTS("nand.", 1, 1);
    OPTEST_3_COMPONENTS("nand.", 0x7FFF, 0x7FFF);
    OPTEST_3_COMPONENTS("nand.", 0x8000, 0x8000);
    OPTEST_3_COMPONENTS("nand.", 0xFFFF, 0xFFFF);
    OPTEST_3_COMPONENTS("nand.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("nand.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("nand.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("NEG Variants\n");
    OPTEST_2_COMPONENTS("neg", 0);
    OPTEST_2_COMPONENTS("neg", 1);
    OPTEST_2_COMPONENTS("neg", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("neg", 0x80000000);
    OPTEST_2_COMPONENTS("neg", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("neg.", 0);
    OPTEST_2_COMPONENTS("neg.", 1);
    OPTEST_2_COMPONENTS("neg.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("neg.", 0x80000000);
    OPTEST_2_COMPONENTS("neg.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("nego", 0);
    OPTEST_2_COMPONENTS("nego", 1);
    OPTEST_2_COMPONENTS("nego", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("nego", 0x80000000);
    OPTEST_2_COMPONENTS("nego", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("nego.", 0);
    OPTEST_2_COMPONENTS("nego.", 1);
    OPTEST_2_COMPONENTS("nego.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("nego.", 0x80000000);
    OPTEST_2_COMPONENTS("nego.", 0xFFFFFFFF);

    printf("NOR Variants\n");
    OPTEST_3_COMPONENTS("nor", 0, 0);
    OPTEST_3_COMPONENTS("nor", 0, 1);
    OPTEST_3_COMPONENTS("nor", 1, 0);
    OPTEST_3_COMPONENTS("nor", 1, 1);
    OPTEST_3_COMPONENTS("nor", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("nor", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("nor", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("nor.", 0, 0);
    OPTEST_3_COMPONENTS("nor.", 0, 1);
    OPTEST_3_COMPONENTS("nor.", 1, 0);
    OPTEST_3_COMPONENTS("nor.", 1, 1);
    OPTEST_3_COMPONENTS("nor.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("nor.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("nor.", 0xFFFFFFFF, 0xFFFFFFFF);

    printf("OR Variants\n");
    OPTEST_3_COMPONENTS("or", 0, 0);
    OPTEST_3_COMPONENTS("or", 0, 1);
    OPTEST_3_COMPONENTS("or", 1, 0);
    OPTEST_3_COMPONENTS("or", 1, 1);
    OPTEST_3_COMPONENTS("or", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("or", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("or", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("or.", 0, 0);
    OPTEST_3_COMPONENTS("or.", 0, 1);
    OPTEST_3_COMPONENTS("or.", 1, 0);
    OPTEST_3_COMPONENTS("or.", 1, 1);
    OPTEST_3_COMPONENTS("or.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("or.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("or.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("orc", 0, 0);
    OPTEST_3_COMPONENTS("orc", 0, 1);
    OPTEST_3_COMPONENTS("orc", 1, 0);
    OPTEST_3_COMPONENTS("orc", 1, 1);
    OPTEST_3_COMPONENTS("orc", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("orc", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("orc", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("orc.", 0, 0);
    OPTEST_3_COMPONENTS("orc.", 0, 1);
    OPTEST_3_COMPONENTS("orc.", 1, 0);
    OPTEST_3_COMPONENTS("orc.", 1, 1);
    OPTEST_3_COMPONENTS("orc.", 0x7FFFFFFF, 0x7FFFFFFF);
    OPTEST_3_COMPONENTS("orc.", 0x80000000, 0x80000000);
    OPTEST_3_COMPONENTS("orc.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS_IMM("ori", 0, 0);
    OPTEST_3_COMPONENTS_IMM("ori", 0, 1);
    OPTEST_3_COMPONENTS_IMM("ori", 1, 0);
    OPTEST_3_COMPONENTS_IMM("ori", 1, 1);
    OPTEST_3_COMPONENTS_IMM("ori", 1, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("ori", 1, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("ori", 1, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("oris", 0, 0);
    OPTEST_3_COMPONENTS_IMM("oris", 0, 1);
    OPTEST_3_COMPONENTS_IMM("oris", 1, 0);
    OPTEST_3_COMPONENTS_IMM("oris", 1, 1);
    OPTEST_3_COMPONENTS_IMM("oris", 1, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("oris", 1, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("oris", 1, 0x7FFF);

    printf("RL[x] Variants\n");

#define test1(op, i) \
    OPTEST_5_COMPONENTS(op, 0x7FFFFFFF, 30, i, 0, 10); \
    OPTEST_5_COMPONENTS(op, 0x80000000, 30, i, 0, 10); \
    OPTEST_5_COMPONENTS(op, 0xFFFFFFFF, 30, i, 0, 10); \
    OPTEST_5_COMPONENTS(op, 0x7FFFFFFF, 30, i, 10, 20); \
    OPTEST_5_COMPONENTS(op, 0x80000000, 30, i, 10, 20); \
    OPTEST_5_COMPONENTS(op, 0xFFFFFFFF, 30, i, 10, 20); \
    OPTEST_5_COMPONENTS(op, 0x7FFFFFFF, 30, i, 20, 30); \
    OPTEST_5_COMPONENTS(op, 0x80000000, 30, i, 20, 30); \
    OPTEST_5_COMPONENTS(op, 0xFFFFFFFF, 30, i, 20, 30);

#define test2(op) \
    test1(op, 0); \
    test1(op, 1); \
    test1(op, 2); \
    test1(op, 3); \
    test1(op, 4); \
    test1(op, 5); \
    test1(op, 6); \
    test1(op, 7); \
    test1(op, 8); \
    test1(op, 9); \
    test1(op,10); \
    test1(op,11); \
    test1(op,12); \
    test1(op,13); \
    test1(op,14); \
    test1(op,15); \
    test1(op,16); \
    test1(op,17); \
    test1(op,18); \
    test1(op,19); \
    test1(op,20); \
    test1(op,21); \
    test1(op,22); \
    test1(op,23); \
    test1(op,24); \
    test1(op,25); \
    test1(op,26); \
    test1(op,27); \
    test1(op,28); \
    test1(op,29); \
    test1(op,30); \
    test1(op,31);

    test2("rlwimi");
    test2("rlwimi.");
    test2("rlwinm");
    test2("rlwinm.");

    printf("Shift Variants\n");
    // Shifts greater than 31 for 32-bit should produce zero.
    for (uint32_t i = 0; i <= 64; i++)
    {
        OPTEST_3_COMPONENTS("slw", i, i);
        OPTEST_3_COMPONENTS("slw", 1, i);
        OPTEST_3_COMPONENTS("slw", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("slw", 0xFFFFFFFF, i);
    }
    for (uint32_t i = 0; i <= 64; i++)
    {
        OPTEST_3_COMPONENTS("slw.", i, i);
        OPTEST_3_COMPONENTS("slw.", 1, i);
        OPTEST_3_COMPONENTS("slw.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("slw.", 0xFFFFFFFF, i);
    }
    for (uint32_t i = 0; i <= 64; i++)
    {
        OPTEST_3_COMPONENTS("sraw", i, i);
        OPTEST_3_COMPONENTS("sraw", 1, i);
        OPTEST_3_COMPONENTS("sraw", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("sraw", 0xFFFFFFFF, i);
    }
    for (uint32_t i = 0; i <= 64; i++)
    {
        OPTEST_3_COMPONENTS("sraw.", i, i);
        OPTEST_3_COMPONENTS("sraw.", 1, i);
        OPTEST_3_COMPONENTS("sraw.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("sraw.", 0xFFFFFFFF, i);
    }

#define test3(op, SH) \
    OPTEST_3_COMPONENTS2(op, 1, SH); \
    OPTEST_3_COMPONENTS2(op, 0x10300, SH); \
    OPTEST_3_COMPONENTS2(op, 0x7FFFFFFF, SH); \
    OPTEST_3_COMPONENTS2(op, 0xFFFFFFFF, SH); \
    OPTEST_3_COMPONENTS2(op, 0x8FFF0300, SH); \
    OPTEST_3_COMPONENTS2(op, 0x80000000, SH);

#define test4(op) \
    test3(op,  0); \
    test3(op,  1); \
    test3(op,  2); \
    test3(op,  3); \
    test3(op,  4); \
    test3(op,  5); \
    test3(op,  6); \
    test3(op,  7); \
    test3(op,  8); \
    test3(op,  9); \
    test3(op, 10); \
    test3(op, 11); \
    test3(op, 12); \
    test3(op, 13); \
    test3(op, 14); \
    test3(op, 15); \
    test3(op, 16); \
    test3(op, 17); \
    test3(op, 18); \
    test3(op, 19); \
    test3(op, 20); \
    test3(op, 21); \
    test3(op, 22); \
    test3(op, 23); \
    test3(op, 24); \
    test3(op, 25); \
    test3(op, 26); \
    test3(op, 27); \
    test3(op, 28); \
    test3(op, 29); \
    test3(op, 30); \
    test3(op, 31);

    test4("srawi");
    test4("srawi.");

    for (uint32_t i = 0; i <= 64; i++)
    {
        OPTEST_3_COMPONENTS("srw", i, i);
        OPTEST_3_COMPONENTS("srw", 1, i);
        OPTEST_3_COMPONENTS("srw", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("srw", 0xFFFFFFFF, i);
    }
    for (uint32_t i = 0; i <= 64; i++)
    {
        OPTEST_3_COMPONENTS("srw.", i, i);
        OPTEST_3_COMPONENTS("srw.", 1, i);
        OPTEST_3_COMPONENTS("srw.", 0x7FFFFFFF, i);
        OPTEST_3_COMPONENTS("srw.", 0xFFFFFFFF, i);
    }

    printf("SUBF Variants\n");
    OPTEST_3_COMPONENTS("subf", 0, 0);
    OPTEST_3_COMPONENTS("subf", 0, 1);
    OPTEST_3_COMPONENTS("subf", 1, 0);
    OPTEST_3_COMPONENTS("subf", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subf", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subf", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subf.", 0, 0);
    OPTEST_3_COMPONENTS("subf.", 0, 1);
    OPTEST_3_COMPONENTS("subf.", 1, 0);
    OPTEST_3_COMPONENTS("subf.", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subf.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subf.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfo", 0, 0);
    OPTEST_3_COMPONENTS("subfo", 0, 1);
    OPTEST_3_COMPONENTS("subfo", 1, 0);
    OPTEST_3_COMPONENTS("subfo", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfo", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfo", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfo.", 0, 0);
    OPTEST_3_COMPONENTS("subfo.", 0, 1);
    OPTEST_3_COMPONENTS("subfo.", 1, 0);
    OPTEST_3_COMPONENTS("subfo.", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfo.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfo.", 0xFFFFFFFF, 0xFFFFFFFF);

    //
    // TODO: Make this a loop, but alternate the carry flag for SUBFC
    //
    OPTEST_3_COMPONENTS("subfc", 0, 0);
    OPTEST_3_COMPONENTS("subfc", 0, 1);
    OPTEST_3_COMPONENTS("subfc", 1, 0);
    OPTEST_3_COMPONENTS("subfc", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfc", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfc", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfc.", 0, 0);
    OPTEST_3_COMPONENTS("subfc.", 0, 1);
    OPTEST_3_COMPONENTS("subfc.", 1, 0);
    OPTEST_3_COMPONENTS("subfc.", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfc.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfc.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfco", 0, 0);
    OPTEST_3_COMPONENTS("subfco", 0, 1);
    OPTEST_3_COMPONENTS("subfco", 1, 0);
    OPTEST_3_COMPONENTS("subfco", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfco", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfco", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfco.", 0, 0);
    OPTEST_3_COMPONENTS("subfco.", 0, 1);
    OPTEST_3_COMPONENTS("subfco.", 1, 0);
    OPTEST_3_COMPONENTS("subfco.", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfco.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfco.", 0xFFFFFFFF, 0xFFFFFFFF);

    OPTEST_3_COMPONENTS("subfe", 0, 0);
    OPTEST_3_COMPONENTS("subfe", 0, 1);
    OPTEST_3_COMPONENTS("subfe", 1, 0);
    OPTEST_3_COMPONENTS("subfe", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfe", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfe", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfe.", 0, 0);
    OPTEST_3_COMPONENTS("subfe.", 0, 1);
    OPTEST_3_COMPONENTS("subfe.", 1, 0);
    OPTEST_3_COMPONENTS("subfe.", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfe.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfe.", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfeo", 0, 0);
    OPTEST_3_COMPONENTS("subfeo", 0, 1);
    OPTEST_3_COMPONENTS("subfeo", 1, 0);
    OPTEST_3_COMPONENTS("subfeo", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfeo", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfeo", 0xFFFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfeo.", 0, 0);
    OPTEST_3_COMPONENTS("subfeo.", 0, 1);
    OPTEST_3_COMPONENTS("subfeo.", 1, 0);
    OPTEST_3_COMPONENTS("subfeo.", 0x00000001, 0x80000000);
    OPTEST_3_COMPONENTS("subfeo.", 0x7FFFFFFF, 0xFFFFFFFF);
    OPTEST_3_COMPONENTS("subfeo.", 0xFFFFFFFF, 0xFFFFFFFF);

    OPTEST_3_COMPONENTS_IMM("subfic", 0, 0);
    OPTEST_3_COMPONENTS_IMM("subfic", 0, 1);
    OPTEST_3_COMPONENTS_IMM("subfic", 1, 0);
    OPTEST_3_COMPONENTS_IMM("subfic", 1, 1);
    OPTEST_3_COMPONENTS_IMM("subfic", 0x7FFFFFFF, 0x7FFF);
    OPTEST_3_COMPONENTS_IMM("subfic", 0xFFFFFFFF, 0x7FFF);

    OPTEST_2_COMPONENTS("subfme", 0);
    OPTEST_2_COMPONENTS("subfme", 1);
    OPTEST_2_COMPONENTS("subfme", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfme", 0x80000000);
    OPTEST_2_COMPONENTS("subfme", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("subfme.", 0);
    OPTEST_2_COMPONENTS("subfme.", 1);
    OPTEST_2_COMPONENTS("subfme.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfme.", 0x80000000);
    OPTEST_2_COMPONENTS("subfme.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("subfmeo", 0);
    OPTEST_2_COMPONENTS("subfmeo", 1);
    OPTEST_2_COMPONENTS("subfmeo", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfmeo", 0x80000000);
    OPTEST_2_COMPONENTS("subfmeo", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("subfmeo.", 0);
    OPTEST_2_COMPONENTS("subfmeo.", 1);
    OPTEST_2_COMPONENTS("subfmeo.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfmeo.", 0x80000000);
    OPTEST_2_COMPONENTS("subfmeo.", 0xFFFFFFFF);

    OPTEST_2_COMPONENTS("subfze", 0);
    OPTEST_2_COMPONENTS("subfze", 1);
    OPTEST_2_COMPONENTS("subfze", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfze", 0x80000000);
    OPTEST_2_COMPONENTS("subfze", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("subfze.", 0);
    OPTEST_2_COMPONENTS("subfze.", 1);
    OPTEST_2_COMPONENTS("subfze.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfze.", 0x80000000);
    OPTEST_2_COMPONENTS("subfze.", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("subfzeo", 0);
    OPTEST_2_COMPONENTS("subfzeo", 1);
    OPTEST_2_COMPONENTS("subfzeo", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfzeo", 0x80000000);
    OPTEST_2_COMPONENTS("subfzeo", 0xFFFFFFFF);
    OPTEST_2_COMPONENTS("subfzeo.", 0);
    OPTEST_2_COMPONENTS("subfzeo.", 1);
    OPTEST_2_COMPONENTS("subfzeo.", 0x7FFFFFFF);
    OPTEST_2_COMPONENTS("subfzeo.", 0x80000000);
    OPTEST_2_COMPONENTS("subfzeo.", 0xFFFFFFFF);

    printf("XOR Variants\n");
    OPTEST_3_COMPONENTS("xor", 0, 0);
    OPTEST_3_COMPONENTS("xor", 0, 1);
    OPTEST_3_COMPONENTS("xor", 1, 0);
    OPTEST_3_COMPONENTS("xor", 1, 1);
    OPTEST_3_COMPONENTS("xor", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS("xor", 0xFFFFFFFF, 0x3FFF);
    OPTEST_3_COMPONENTS("xor.", 0, 0);
    OPTEST_3_COMPONENTS("xor.", 0, 1);
    OPTEST_3_COMPONENTS("xor.", 1, 0);
    OPTEST_3_COMPONENTS("xor.", 1, 1);
    OPTEST_3_COMPONENTS("xor.", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS("xor.", 0xFFFFFFFF, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("xori", 0, 0);
    OPTEST_3_COMPONENTS_IMM("xori", 0, 1);
    OPTEST_3_COMPONENTS_IMM("xori", 1, 0);
    OPTEST_3_COMPONENTS_IMM("xori", 1, 1);
    OPTEST_3_COMPONENTS_IMM("xori", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("xori", 0xFFFFFFFF, 0x3FFF);
    OPTEST_3_COMPONENTS_IMM("xoris", 0, 0);
    OPTEST_3_COMPONENTS_IMM("xoris", 0, 1);
    OPTEST_3_COMPONENTS_IMM("xoris", 1, 0);
    OPTEST_3_COMPONENTS_IMM("xoris", 1, 1);
    OPTEST_3_COMPONENTS_IMM("xoris", 0xFFFFFFFF, 0x1FFF);
    OPTEST_3_COMPONENTS_IMM("xoris", 0xFFFFFFFF, 0x3FFF);
}

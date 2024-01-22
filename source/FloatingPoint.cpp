#include <cfloat>
#ifdef MACOS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
#include <cinttypes>
#endif
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#ifdef MACOS
#else
#include <type_traits>
#endif

#include "Tests.h"
#include "Utils.h"

// NaN variants.
#define FLOAT_SNAN  std::numeric_limits<float>::signaling_NaN()
#define FLOAT_QNAN  std::numeric_limits<float>::quiet_NaN()
#define DOUBLE_SNAN std::numeric_limits<double>::signaling_NaN()
#define DOUBLE_QNAN std::numeric_limits<double>::quiet_NaN()

#define ClearFPSCR() \
    asm volatile ("mtfsf 0xFF, %[reg]" : : [reg]"f"(0.0))

typedef union {
    uint64_t i;
    double f;
} f64;

#define GetFPSCR(fpscr) \
do { \
    f64 d; \
    asm volatile ("mffs %[out]" : [out]"=f"(d.f)); \
    fpscr = d.i; \
} while (0)

static void SetRoundingMode(uint32_t index)
{
    if (index == 0)
        asm volatile ("mtfsb0 30\nmtfsb0 31\n");
    else if (index == 1)
        asm volatile ("mtfsb0 30\nmtfsb1 31\n");
    else if (index == 2)
        asm volatile ("mtfsb1 30\nmtfsb0 31\n");
    else if (index == 3)
        asm volatile ("mtfsb1 30\nmtfsb1 31\n");
}

#define EnableInvalidOperationExceptions() \
    asm volatile ("mtfsb1 24")

static const char* GetRoundingModeString(uint32_t index)
{
    switch (index)
    {
    case 0: // Round to nearest
        return "(RTN)";
    case 1: // Round to zero
        return "(RTZ)";
    case 2: // Round to +inf
        return "(RTPI)";
    case 3: // Round to -inf
        return "(RTNI)";
    }

    return "Unknown";
}

#define CleanTestState() \
    ClearFPSCR(); SetCR(0);

// Test for a 2-component instruction
// e.g. FABS frD, frB
#define OPTEST_2_COMPONENTS(inst, frA)                                                                      \
{                                                                                                           \
    f64 output; uint32_t fpscr, cr;                                                                         \
                                                                                                            \
    output.i = 0x123456789abcdef0ULL;                                                                       \
    CleanTestState();                                                                                       \
    asm volatile (                                                                                          \
        inst " %[out], %[Fra]": [out]"+&f"(output.f) : [Fra]"f"(frA));                                      \
    GetFPSCR(fpscr); GetCR(cr);                                                                             \
                                                                                                            \
    printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",        \
           upper(inst), output.i, frA, fpscr, cr);                                                          \
                                                                                                            \
    /* Test with invalid exceptions enabled */                                                              \
    output.i = 0x123456789abcdef0ULL;                                                                       \
    CleanTestState();                                                                                       \
    EnableInvalidOperationExceptions();                                                                     \
    asm volatile(                                                                                           \
        inst " %[out], %[Fra]": [out]"+&f"(output.f) : [Fra]"f"(frA));                                      \
    GetFPSCR(fpscr); GetCR(cr);                                                                             \
                                                                                                            \
    printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",    \
           upper(inst), "(VE)", output.i, frA, fpscr, cr);                                                  \
}

// Test for a 2-component instruction which tests all rounding modes.
#define OPTEST_2_COMPONENTS_WITH_ROUND(inst, frA)                                                            \
{                                                                                                            \
    f64 output; uint32_t fpscr, cr;                                                                          \
                                                                                                             \
    for (int i = 0; i <= 3; i++)                                                                             \
    {                                                                                                        \
        output.i = 0x123456789abcdef0ULL;                                                                    \
        ClearFPSCR();                                                                                        \
        SetRoundingMode(i);                                                                                  \
        SetCR(0);                                                                                            \
                                                                                                             \
        asm volatile (                                                                                       \
            inst " %[out], %[Fra]" : [out]"+&f"(output.f) : [Fra]"f"(frA));                                  \
        GetFPSCR(fpscr); GetCR(cr);                                                                          \
                                                                                                             \
        printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
               upper(inst), GetRoundingModeString(i), output.i, frA, fpscr, cr);                             \
    }                                                                                                        \
                                                                                                             \
    /* Test with invalid exceptions enabled */                                                               \
    output.i = 0x123456789abcdef0ULL;                                                                        \
    CleanTestState();                                                                                        \
    EnableInvalidOperationExceptions();                                                                      \
    asm volatile(                                                                                            \
        inst " %[out], %[Fra]": [out]"+&f"(output.f) : [Fra]"f"(frA));                                       \
    GetFPSCR(fpscr); GetCR(cr);                                                                              \
                                                                                                             \
    printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",     \
           upper(inst), "(VE)", output.i, frA, fpscr, cr);                                                   \
}

// Test for a 3-component instruction with all rounding modes.
// e.g. FADDS frD, frA, frB
#define OPTEST_3_COMPONENTS_WITH_ROUND(inst, frA, frB)                                                                \
{                                                                                                                     \
    f64 output; uint32_t fpscr, cr;                                                                                   \
                                                                                                                      \
    for (int i = 0; i <= 3; i++)                                                                                      \
    {                                                                                                                 \
        output.i = 0x123456789abcdef0ULL;                                                                             \
        ClearFPSCR();                                                                                                 \
        SetRoundingMode(i);                                                                                           \
        SetCR(0);                                                                                                     \
                                                                                                                      \
        asm volatile (                                                                                                \
            inst " %[out], %[Fra], %[Frb]"                                                                            \
            : [out]"+&f"(output.f)                                                                                    \
            : [Fra]"f"(frA), [Frb]"f"(frB));                                                                          \
        GetFPSCR(fpscr); GetCR(cr);                                                                                   \
                                                                                                                      \
        printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
               upper(inst), GetRoundingModeString(i), output.i, frA, frB, fpscr, cr);                                 \
    }                                                                                                                 \
                                                                                                                      \
    /* Also perform one test of the instruction value with invalid operation exceptions on */                         \
    output.i = 0x123456789abcdef0ULL;                                                                                 \
    CleanTestState();                                                                                                 \
    EnableInvalidOperationExceptions();                                                                               \
                                                                                                                      \
    asm volatile (                                                                                                    \
        inst " %[out], %[Fra], %[Frb]"                                                                                \
        : [out]"+&f"(output.f)                                                                                        \
        : [Fra]"f"(frA), [Frb]"f"(frB));                                                                              \
    GetFPSCR(fpscr); GetCR(cr);                                                                                       \
                                                                                                                      \
    printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08"                  \
           PRIX32 "\n", upper(inst), "(VE)", output.i, frA, frB, fpscr, cr);                                          \
}

// Used for testing CMP instructions.
#define OPTEST_3_COMPONENTS_CMP(inst, frA, frB)                                                   \
{                                                                                                 \
    uint32_t fpscr, cr;                                                                           \
    CleanTestState();                                                                             \
    asm volatile (inst " cr1, %[Fra], %[Frb]": : [Fra]"f"(frA), [Frb]"f"(frB));                   \
    GetFPSCR(fpscr); GetCR(cr);                                                                   \
                                                                                                  \
    printf("%-8s :: frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",            \
           upper(inst), frA, frB, fpscr, cr);                                                     \
}

// Test for a 4-component instruction.
#define OPTEST_4_COMPONENTS(inst, frA, frC, frB)                                                                       \
{                                                                                                                      \
    f64 output; uint32_t fpscr, cr;                                                                                    \
    output.i = 0x123456789abcdef0ULL;                                                                                  \
    CleanTestState();                                                                                                  \
    asm volatile (inst " %[out], %[Fra], %[Frc], %[Frb]"                                                               \
        : [out]"+&f"(output.f)                                                                                         \
        : [Fra]"f"(frA), [Frc]"f"(frC), [Frb]"f"(frB));                                                                \
    GetFPSCR(fpscr); GetCR(cr);                                                                                        \
                                                                                                                       \
    printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | frC %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
           upper(inst), output.i, frA, frC, frB, fpscr, cr);                                                           \
}

// Test for a 4-component instruction with all rounding modes.
// e.g. FMADD frD, frA, frC, frB
#define OPTEST_4_COMPONENTS_WITH_ROUND(inst, frA, frC, frB)                                                                    \
{                                                                                                                              \
    f64 output; uint32_t fpscr, cr;                                                                                            \
                                                                                                                               \
    for (int i = 0; i <= 3; i++)                                                                                               \
    {                                                                                                                          \
        output.i = 0x123456789abcdef0ULL;                                                                                      \
        ClearFPSCR();                                                                                                          \
        SetRoundingMode(i);                                                                                                    \
        SetCR(0);                                                                                                              \
                                                                                                                               \
        asm volatile (                                                                                                         \
            inst " %[out], %[Fra], %[Frc], %[Frb]"                                                                             \
            : [out]"+&f"(output.f)                                                                                             \
            : [Fra]"f"(frA), [Frc]"f"(frC), [Frb]"f"(frB));                                                                    \
        GetFPSCR(fpscr); GetCR(cr);                                                                                            \
                                                                                                                               \
        printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | frC %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
               upper(inst), GetRoundingModeString(i), output.i, frA, frC, frB, fpscr, cr);                                     \
    }                                                                                                                          \
                                                                                                                               \
    /* Also perform one test of the instruction value with invalid operation exceptions on */                                  \
    output.i = 0x123456789abcdef0ULL;                                                                                          \
    CleanTestState();                                                                                                          \
    EnableInvalidOperationExceptions();                                                                                        \
                                                                                                                               \
    asm volatile (                                                                                                             \
        inst " %[out], %[Fra], %[Frc], %[Frb]"                                                                                 \
        : [out]"+&f"(output.f)                                                                                                 \
        : [Fra]"f"(frA), [Frc]"f"(frC), [Frb]"f"(frB));                                                                        \
    GetFPSCR(fpscr); GetCR(cr);                                                                                                \
                                                                                                                               \
    printf("%-8s %6s :: frD 0x%016" PRIX64 " | frA %e | frC %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",     \
           upper(inst), "(VE)", output.i, frA, frC, frB, fpscr, cr);                                                           \
}

// Tests if floating point comparison functions (FCMPO/FCMPU) preserve the class bit when setting the FPCC bits.
static void FPRFClassBitTest()
{
    double qnan_1 = DOUBLE_QNAN;
    double qnan_2 = DOUBLE_QNAN;
    uint32_t fpscr, cr;

    // "MTFSB1 15" sets the class bit in the FPRF. If FCMPO and FCMPU are implemented wrong, then this
    // value will get steamrolled.

    CleanTestState();
    asm volatile ("mtfsb1 15\n"
                  "fcmpo cr1, %[frA], %[frB]" :: [frA]"f"(qnan_1), [frB]"f"(qnan_2));
    GetFPSCR(fpscr); GetCR(cr);
    printf("FCMPO :: frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", qnan_1, qnan_2, fpscr, cr);

    CleanTestState();
    asm volatile ("mtfsb1 15\n"
                  "fcmpu cr1, %[frA], %[frB]" :: [frA]"f"(qnan_1), [frB]"f"(qnan_2));
    GetFPSCR(fpscr); GetCR(cr);
    printf("FCMPU :: frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", qnan_1, qnan_2, fpscr, cr);
}

void PPCFloatingPointTests()
{
    // Run specialized tests first.
    printf("FPRF Class Bit Preservation Tests\n");
    FPRFClassBitTest();

    printf("FABS Variants\n");
    OPTEST_2_COMPONENTS("fabs", 0.0);
    OPTEST_2_COMPONENTS("fabs", -0.0);
    OPTEST_2_COMPONENTS("fabs", -1.0);
    OPTEST_2_COMPONENTS("fabs", -FLT_MIN);
    OPTEST_2_COMPONENTS("fabs", -FLT_MAX);
    OPTEST_2_COMPONENTS("fabs", -DBL_MIN);
    OPTEST_2_COMPONENTS("fabs", -DBL_MAX);
    OPTEST_2_COMPONENTS("fabs", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("fabs", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("fabs", INFINITY);
    OPTEST_2_COMPONENTS("fabs", -INFINITY);
    OPTEST_2_COMPONENTS("fabs.", 0.0);
    OPTEST_2_COMPONENTS("fabs.", -0.0);
    OPTEST_2_COMPONENTS("fabs.", -1.0);
    OPTEST_2_COMPONENTS("fabs.", -FLT_MIN);
    OPTEST_2_COMPONENTS("fabs.", -FLT_MAX);
    OPTEST_2_COMPONENTS("fabs.", -DBL_MIN);
    OPTEST_2_COMPONENTS("fabs.", -DBL_MAX);
    OPTEST_2_COMPONENTS("fabs.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("fabs.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("fabs.", INFINITY);
    OPTEST_2_COMPONENTS("fabs.", -INFINITY);

    printf("FADD Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadd.", -INFINITY, -INFINITY);

    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", FLT_MAX, FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", -FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", 0.0, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", FLT_MAX, FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", -FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", 0.0, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fadds.", -INFINITY, -INFINITY);

    printf("FCMP variants\n");
    OPTEST_3_COMPONENTS_CMP("fcmpo", 0.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("fcmpo", 0.0, 1.0);
    OPTEST_3_COMPONENTS_CMP("fcmpo", 1.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("fcmpo", 0.5, 0.5);
    OPTEST_3_COMPONENTS_CMP("fcmpo", 0.0, 0.5);
    OPTEST_3_COMPONENTS_CMP("fcmpo", 0.5, 0.0);
    OPTEST_3_COMPONENTS_CMP("fcmpo", DOUBLE_QNAN, DOUBLE_QNAN);
    OPTEST_3_COMPONENTS_CMP("fcmpo", DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_3_COMPONENTS_CMP("fcmpo", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpo", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpo", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpo", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpu", 0.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("fcmpu", 0.0, 1.0);
    OPTEST_3_COMPONENTS_CMP("fcmpu", 1.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("fcmpu", 0.5, 0.5);
    OPTEST_3_COMPONENTS_CMP("fcmpu", 0.0, 0.5);
    OPTEST_3_COMPONENTS_CMP("fcmpu", 0.5, 0.0);
    OPTEST_3_COMPONENTS_CMP("fcmpu", DOUBLE_QNAN, DOUBLE_QNAN);
    OPTEST_3_COMPONENTS_CMP("fcmpu", DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_3_COMPONENTS_CMP("fcmpu", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpu", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpu", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("fcmpu", -INFINITY, -INFINITY);

    printf("FCTI Variants\n");
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", 0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", -0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", 2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", -2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", 6.30584e-044);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", FLOAT_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", FLOAT_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw", -INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", 0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", -0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", 2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", -2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", 6.30584e-044);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("fctiw.", -INFINITY);
    OPTEST_2_COMPONENTS("fctiwz", 0.0);
    OPTEST_2_COMPONENTS("fctiwz", 0.5);
    OPTEST_2_COMPONENTS("fctiwz", -0.5);
    OPTEST_2_COMPONENTS("fctiwz", 2.4679999352);
    OPTEST_2_COMPONENTS("fctiwz", -2.4679999352);
    OPTEST_2_COMPONENTS("fctiwz", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("fctiwz", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("fctiwz", INFINITY);
    OPTEST_2_COMPONENTS("fctiwz", -INFINITY);
    OPTEST_2_COMPONENTS("fctiwz.", 0.0);
    OPTEST_2_COMPONENTS("fctiwz.", 0.5);
    OPTEST_2_COMPONENTS("fctiwz.", -0.5);
    OPTEST_2_COMPONENTS("fctiwz.", 2.4679999352);
    OPTEST_2_COMPONENTS("fctiwz.", -2.4679999352);
    OPTEST_2_COMPONENTS("fctiwz.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("fctiwz.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("fctiwz.", INFINITY);
    OPTEST_2_COMPONENTS("fctiwz.", -INFINITY);

    printf("FDIV Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdiv.", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fdivs.", -INFINITY, FLOAT_SNAN);

    printf("FMADD Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadd.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmadds.", 1.0, 1.0, -INFINITY);

    printf("FMSUB Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsub.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fmsubs.", 1.0, 1.0, -INFINITY);

    printf("FMUL Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmul.", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fmuls.", -INFINITY, FLOAT_SNAN);

    printf("FNABS Variants\n");
    OPTEST_2_COMPONENTS("fnabs", 0.0);
    OPTEST_2_COMPONENTS("fnabs", -0.0);
    OPTEST_2_COMPONENTS("fnabs", -1.0);
    OPTEST_2_COMPONENTS("fnabs", -FLT_MIN);
    OPTEST_2_COMPONENTS("fnabs", -FLT_MAX);
    OPTEST_2_COMPONENTS("fnabs", -DBL_MIN);
    OPTEST_2_COMPONENTS("fnabs", -DBL_MAX);
    OPTEST_2_COMPONENTS("fnabs", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("fnabs", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("fnabs", INFINITY);
    OPTEST_2_COMPONENTS("fnabs", -INFINITY);
    OPTEST_2_COMPONENTS("fnabs.", 0.0);
    OPTEST_2_COMPONENTS("fnabs.", -0.0);
    OPTEST_2_COMPONENTS("fnabs.", -1.0);
    OPTEST_2_COMPONENTS("fnabs.", -FLT_MIN);
    OPTEST_2_COMPONENTS("fnabs.", -FLT_MAX);
    OPTEST_2_COMPONENTS("fnabs.", -DBL_MIN);
    OPTEST_2_COMPONENTS("fnabs.", -DBL_MAX);
    OPTEST_2_COMPONENTS("fnabs.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("fnabs.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("fnabs.", INFINITY);
    OPTEST_2_COMPONENTS("fnabs.", -INFINITY);

    printf("FNEG Variants\n");
    OPTEST_2_COMPONENTS("fneg", 0.0);
    OPTEST_2_COMPONENTS("fneg", -0.0);
    OPTEST_2_COMPONENTS("fneg", DBL_MAX);
    OPTEST_2_COMPONENTS("fneg", -DBL_MAX);
    OPTEST_2_COMPONENTS("fneg", DBL_MIN);
    OPTEST_2_COMPONENTS("fneg", -DBL_MIN);
    OPTEST_2_COMPONENTS("fneg", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("fneg", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("fneg", INFINITY);
    OPTEST_2_COMPONENTS("fneg", INFINITY);
    OPTEST_2_COMPONENTS("fneg.", 0.0);
    OPTEST_2_COMPONENTS("fneg.", -0.0);
    OPTEST_2_COMPONENTS("fneg.", DBL_MAX);
    OPTEST_2_COMPONENTS("fneg.", -DBL_MAX);
    OPTEST_2_COMPONENTS("fneg.", DBL_MIN);
    OPTEST_2_COMPONENTS("fneg.", -DBL_MIN);
    OPTEST_2_COMPONENTS("fneg.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("fneg.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("fneg.", INFINITY);
    OPTEST_2_COMPONENTS("fneg.", INFINITY);

    printf("FNMADD Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadd.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmadds.", 1.0, 1.0, -INFINITY);

    printf("FNMSUB Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsub.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("fnmsubs.", 1.0, 1.0, -INFINITY);

    printf("FRES Variants\n");
    OPTEST_2_COMPONENTS("fres", 0.0);
    OPTEST_2_COMPONENTS("fres", -0.0);
    OPTEST_2_COMPONENTS("fres", 100.986352178);
    OPTEST_2_COMPONENTS("fres", 7.3233339282);
    OPTEST_2_COMPONENTS("fres", 6.30584e-044);
    OPTEST_2_COMPONENTS("fres", 69.0);
    OPTEST_2_COMPONENTS("fres", 420.0);
    OPTEST_2_COMPONENTS("fres", DBL_MAX);
    OPTEST_2_COMPONENTS("fres", DBL_MIN);
    OPTEST_2_COMPONENTS("fres", FLT_MAX);
    OPTEST_2_COMPONENTS("fres", FLT_MIN);
    OPTEST_2_COMPONENTS("fres", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("fres", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("fres", INFINITY);
    OPTEST_2_COMPONENTS("fres", -INFINITY);
    OPTEST_2_COMPONENTS("fres.", 0.0);
    OPTEST_2_COMPONENTS("fres.", -0.0);
    OPTEST_2_COMPONENTS("fres.", 100.986352178);
    OPTEST_2_COMPONENTS("fres.", 7.3233339282);
    OPTEST_2_COMPONENTS("fres.", 6.30584e-044);
    OPTEST_2_COMPONENTS("fres.", 69.0);
    OPTEST_2_COMPONENTS("fres.", 420.0);
    OPTEST_2_COMPONENTS("fres.", DBL_MAX);
    OPTEST_2_COMPONENTS("fres.", DBL_MIN);
    OPTEST_2_COMPONENTS("fres.", FLT_MAX);
    OPTEST_2_COMPONENTS("fres.", FLT_MIN);
    OPTEST_2_COMPONENTS("fres.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("fres.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("fres.", INFINITY);
    OPTEST_2_COMPONENTS("fres.", -INFINITY);

    printf("FRSP Variants\n");
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", -0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", DBL_MIN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", DBL_MAX);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", FLT_MIN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", FLT_MAX);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", 5.6519082319399);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", 21321.94923489023);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp", -INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", -0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", DBL_MIN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", DBL_MAX);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", FLT_MIN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", FLT_MAX);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", 5.6519082319399);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", 21321.94923489023);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("frsp.", -INFINITY);

    printf("FRSQRTE Variants\n");
    OPTEST_2_COMPONENTS("frsqrte", 0.0);
    OPTEST_2_COMPONENTS("frsqrte", -0.0);
    OPTEST_2_COMPONENTS("frsqrte", -1.0);
    OPTEST_2_COMPONENTS("frsqrte", 36.0);
    OPTEST_2_COMPONENTS("frsqrte", 100.0);
    OPTEST_2_COMPONENTS("frsqrte", 52324.23123123212);
    OPTEST_2_COMPONENTS("frsqrte", 2.66247e-044);
    OPTEST_2_COMPONENTS("frsqrte", DBL_MIN);
    OPTEST_2_COMPONENTS("frsqrte", DBL_MAX);
    OPTEST_2_COMPONENTS("frsqrte", FLT_MIN);
    OPTEST_2_COMPONENTS("frsqrte", FLT_MAX);
    OPTEST_2_COMPONENTS("frsqrte", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("frsqrte", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("frsqrte", INFINITY);
    OPTEST_2_COMPONENTS("frsqrte", -INFINITY);
    OPTEST_2_COMPONENTS("frsqrte.", 0.0);
    OPTEST_2_COMPONENTS("frsqrte.", -0.0);
    OPTEST_2_COMPONENTS("frsqrte.", -1.0);
    OPTEST_2_COMPONENTS("frsqrte.", 36.0);
    OPTEST_2_COMPONENTS("frsqrte.", 100.0);
    OPTEST_2_COMPONENTS("frsqrte.", 52324.23123123212);
    OPTEST_2_COMPONENTS("frsqrte.", 2.66247e-044);
    OPTEST_2_COMPONENTS("frsqrte.", DBL_MIN);
    OPTEST_2_COMPONENTS("frsqrte.", DBL_MAX);
    OPTEST_2_COMPONENTS("frsqrte.", FLT_MIN);
    OPTEST_2_COMPONENTS("frsqrte.", FLT_MAX);
    OPTEST_2_COMPONENTS("frsqrte.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("frsqrte.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("frsqrte.", INFINITY);
    OPTEST_2_COMPONENTS("frsqrte.", -INFINITY);

    printf("FSEL Variants\n");
    OPTEST_4_COMPONENTS("fsel", 0.0, 0.0, 0.0);
    OPTEST_4_COMPONENTS("fsel", 10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("fsel", -10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("fsel", 2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("fsel", -2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("fsel", DOUBLE_SNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_QNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_SNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_QNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_SNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_QNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("fsel", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel", INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("fsel", INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("fsel", INFINITY, INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("fsel", -INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel", -INFINITY, -INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel", -INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("fsel", -INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_SNAN, DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("fsel", DOUBLE_QNAN, DOUBLE_QNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("fsel.", 0.0, 0.0, 0.0);
    OPTEST_4_COMPONENTS("fsel.", 10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("fsel.", -10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("fsel.", 2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("fsel.", -2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_SNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_QNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_SNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_QNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_SNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_QNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("fsel.", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel.", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel.", INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("fsel.", INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("fsel.", INFINITY, INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("fsel.", -INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel.", -INFINITY, -INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("fsel.", -INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("fsel.", -INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_SNAN, DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("fsel.", DOUBLE_QNAN, DOUBLE_QNAN, DOUBLE_QNAN);

    printf("FSUB Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsub.", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", INFINITY, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", 0.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("fsubs.", -INFINITY, -INFINITY);
}

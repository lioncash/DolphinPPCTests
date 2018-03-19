#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <type_traits>

#include "Tests.h"

// NaN variants.
#define FLOAT_SNAN  std::numeric_limits<float>::signaling_NaN()
#define FLOAT_QNAN  std::numeric_limits<float>::quiet_NaN()
#define DOUBLE_SNAN std::numeric_limits<double>::signaling_NaN()
#define DOUBLE_QNAN std::numeric_limits<double>::quiet_NaN()

static void ClearFPSCR()
{
    asm volatile ("mtfsf 0xFF, %[reg]" : : [reg]"f"(0.0));
}
static uint32_t GetFPSCR()
{
    double d = 0.0;
    asm volatile ("mffs %[out]" : [out]"=f"(d));

    uint64_t i = 0;
    std::memcpy(&i, &d, sizeof(uint64_t));

    // Lower 32 bits are undefined according to the PPC reference.
    return static_cast<uint32_t>(i >> 32);
}

// Test for a 2-component instruction
// e.g. FABS frD, frB
#define OPTEST_2_COMPONENTS(inst, frA)                                                                  \
{                                                                                                       \
    uint64_t output;                                                                                    \
                                                                                                        \
    ClearFPSCR();                                                                                       \
    SetCR(0);                                                                                           \
    asm volatile (inst " %[out], %[Fra]": [out]"=&f"(output) : [Fra]"f"(frA));                          \
                                                                                                        \
    printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",    \
           inst, output, frA, GetFPSCR(), GetCR());                                                     \
}

// Test for a 2-component instruction which tests all rounding modes.
#define OPTEST_2_COMPONENTS_WITH_ROUND(inst, frA)                                                        \
{                                                                                                        \
    uint64_t output;                                                                                     \
                                                                                                         \
                                                                                                         \
    for (int i = 0; i <= 3; i++)                                                                         \
    {                                                                                                    \
        ClearFPSCR();                                                                                    \
        SetCR(0);                                                                                        \
                                                                                                         \
        if (i == 0)                                                                                      \
            asm volatile ("mtfsb0 30\nmtfsb0 31\n");                                                     \
        else if (i == 1)                                                                                 \
            asm volatile ("mtfsb0 30\nmtfsb1 31\n");                                                     \
        else if (i == 2)                                                                                 \
            asm volatile ("mtfsb1 30\nmtfsb0 31\n");                                                     \
        else if (i == 3)                                                                                 \
            asm volatile ("mtfsb1 30\nmtfsb1 31\n");                                                     \
                                                                                                         \
        asm volatile (inst " %[out], %[Fra]"                                                             \
            : [out]"=&f"(output)                                                                         \
            : [Fra]"f"(frA));                                                                            \
                                                                                                         \
        printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
               inst, output, frA, GetFPSCR(), GetCR());                                                  \
    }                                                                                                    \
}

// Test for a 3-component instruction with all rounding modes.
// e.g. FADDS frD, frA, frB
#define OPTEST_3_COMPONENTS_WITH_ROUND(inst, frA, frB)                                                            \
{                                                                                                                 \
    uint64_t output;                                                                                              \
                                                                                                                  \
    for (int i = 0; i <= 3; i++)                                                                                  \
    {                                                                                                             \
        ClearFPSCR();                                                                                             \
        SetCR(0);                                                                                                 \
                                                                                                                  \
        if (i == 0)                                                                                               \
            asm volatile ("mtfsb0 30\nmtfsb0 31\n");                                                              \
        else if (i == 1)                                                                                          \
            asm volatile ("mtfsb0 30\nmtfsb1 31\n");                                                              \
        else if (i == 2)                                                                                          \
            asm volatile ("mtfsb1 30\nmtfsb0 31\n");                                                              \
        else if (i == 3)                                                                                          \
            asm volatile ("mtfsb1 30\nmtfsb1 31\n");                                                              \
                                                                                                                  \
        asm volatile (inst " %[out], %[Fra], %[Frb]"                                                              \
            : [out]"=&f"(output)                                                                                  \
            : [Fra]"f"(frA), [Frb]"f"(frB));                                                                      \
                                                                                                                  \
        printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
               inst, output, frA, frB, GetFPSCR(), GetCR());                                                      \
    }                                                                                                             \
}

// Used for testing CMP instructions.
#define OPTEST_3_COMPONENTS_CMP(inst, frA, frB)                                                   \
{                                                                                                 \
    ClearFPSCR();                                                                                 \
    SetCR(0);                                                                                     \
    asm volatile (inst " cr1, %[Fra], %[Frb]": : [Fra]"f"(frA), [Frb]"f"(frB));                   \
                                                                                                  \
    printf("%-8s :: frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n",            \
           inst, frA, frB, GetFPSCR(), GetCR());                                                  \
}

// Test for a 4-component instruction.
#define OPTEST_4_COMPONENTS(inst, frA, frC, frB)                                                                       \
{                                                                                                                      \
    uint64_t output;                                                                                                   \
                                                                                                                       \
    asm volatile (inst " %[out], %[Fra], %[Frc], %[Frb]"                                                               \
        : [out]"=&f"(output)                                                                                           \
        : [Fra]"f"(frA), [Frc]"f"(frC), [Frb]"f"(frB));                                                                \
                                                                                                                       \
    printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | frC %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
           inst, output, frA, frC, frB, GetFPSCR(), GetCR());                                                          \
}

// Test for a 4-component instruction with all rounding modes.
// e.g. FMADD frD, frA, frC, frB
#define OPTEST_4_COMPONENTS_WITH_ROUND(inst, frA, frC, frB)                                                                \
{                                                                                                                          \
    uint64_t output;                                                                                                       \
                                                                                                                           \
    for (int i = 0; i <= 3; i++)                                                                                           \
    {                                                                                                                      \
        ClearFPSCR();                                                                                                      \
        SetCR(0);                                                                                                          \
                                                                                                                           \
        if (i == 0)                                                                                                        \
            asm volatile ("mtfsb0 30\nmtfsb0 31\n");                                                                       \
        else if (i == 1)                                                                                                   \
            asm volatile ("mtfsb0 30\nmtfsb1 31\n");                                                                       \
        else if (i == 2)                                                                                                   \
            asm volatile ("mtfsb1 30\nmtfsb0 31\n");                                                                       \
        else if (i == 3)                                                                                                   \
            asm volatile ("mtfsb1 30\nmtfsb1 31\n");                                                                       \
                                                                                                                           \
        asm volatile (inst " %[out], %[Fra], %[Frc], %[Frb]"                                                               \
            : [out]"=&f"(output)                                                                                           \
            : [Fra]"f"(frA), [Frc]"f"(frC), [Frb]"f"(frB));                                                                \
                                                                                                                           \
        printf("%-8s :: frD 0x%016" PRIX64 " | frA %e | frC %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", \
               inst, output, frA, frC, frB, GetFPSCR(), GetCR());                                                          \
    }                                                                                                                      \
}

// Tests if floating point comparison functions (FCMPO/FCMPU) preserve the class bit when setting the FPCC bits.
static void FPRFClassBitTest()
{
    double qnan_1 = DOUBLE_QNAN;
    double qnan_2 = DOUBLE_QNAN;

    // "MTFSB1 15" sets the class bit in the FPRF. If FCMPO and FCMPU are implemented wrong, then this
    // value will get steamrolled.

    ClearFPSCR();
    asm volatile ("MTFSB1 15\n"
                  "FCMPO cr1, %[frA], %[frB]" :: [frA]"f"(qnan_1), [frB]"f"(qnan_2));
    printf("FCMPO :: frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", qnan_1, qnan_2, GetFPSCR(), GetCR());

    ClearFPSCR();
    asm volatile ("MTFSB1 15\n"
                  "FCMPU cr1, %[frA], %[frB]" :: [frA]"f"(qnan_1), [frB]"f"(qnan_2));
    printf("FCMPU :: frA %e | frB %e | FPSCR: 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", qnan_1, qnan_2, GetFPSCR(), GetCR());
}

void PPCFloatingPointTests()
{
    // Run specialized tests first.
    printf("FPRF Class Bit Preservation Tests\n");
    FPRFClassBitTest();

    printf("FABS Variants\n");
    OPTEST_2_COMPONENTS("FABS", 0.0);
    OPTEST_2_COMPONENTS("FABS", -0.0);
    OPTEST_2_COMPONENTS("FABS", -1.0);
    OPTEST_2_COMPONENTS("FABS", -FLT_MIN);
    OPTEST_2_COMPONENTS("FABS", -FLT_MAX);
    OPTEST_2_COMPONENTS("FABS", -DBL_MIN);
    OPTEST_2_COMPONENTS("FABS", -DBL_MAX);
    OPTEST_2_COMPONENTS("FABS", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FABS", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FABS", INFINITY);
    OPTEST_2_COMPONENTS("FABS", -INFINITY);
    OPTEST_2_COMPONENTS("FABS.", 0.0);
    OPTEST_2_COMPONENTS("FABS.", -0.0);
    OPTEST_2_COMPONENTS("FABS.", -1.0);
    OPTEST_2_COMPONENTS("FABS.", -FLT_MIN);
    OPTEST_2_COMPONENTS("FABS.", -FLT_MAX);
    OPTEST_2_COMPONENTS("FABS.", -DBL_MIN);
    OPTEST_2_COMPONENTS("FABS.", -DBL_MAX);
    OPTEST_2_COMPONENTS("FABS.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FABS.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FABS.", INFINITY);
    OPTEST_2_COMPONENTS("FABS.", -INFINITY);

    printf("\nFADD Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADD.", -INFINITY, -INFINITY);

    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", FLT_MAX, FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", -FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", 0.0, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", FLT_MAX, FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", -FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", FLT_MAX, -FLT_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", 0.0, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FADDS.", -INFINITY, -INFINITY);

    printf("\nFCMP variants\n");
    OPTEST_3_COMPONENTS_CMP("FCMPO", 0.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("FCMPO", 0.0, 1.0);
    OPTEST_3_COMPONENTS_CMP("FCMPO", 1.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("FCMPO", 0.5, 0.5);
    OPTEST_3_COMPONENTS_CMP("FCMPO", 0.0, 0.5);
    OPTEST_3_COMPONENTS_CMP("FCMPO", 0.5, 0.0);
    OPTEST_3_COMPONENTS_CMP("FCMPO", DOUBLE_QNAN, DOUBLE_QNAN);
    OPTEST_3_COMPONENTS_CMP("FCMPO", DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_3_COMPONENTS_CMP("FCMPO", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPO", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPO", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPO", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPU", 0.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("FCMPU", 0.0, 1.0);
    OPTEST_3_COMPONENTS_CMP("FCMPU", 1.0, 0.0);
    OPTEST_3_COMPONENTS_CMP("FCMPU", 0.5, 0.5);
    OPTEST_3_COMPONENTS_CMP("FCMPU", 0.0, 0.5);
    OPTEST_3_COMPONENTS_CMP("FCMPU", 0.5, 0.0);
    OPTEST_3_COMPONENTS_CMP("FCMPU", DOUBLE_QNAN, DOUBLE_QNAN);
    OPTEST_3_COMPONENTS_CMP("FCMPU", DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_3_COMPONENTS_CMP("FCMPU", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPU", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPU", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_CMP("FCMPU", -INFINITY, -INFINITY);

    printf("\nFCTI Variants\n");
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", 0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", -0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", 2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", -2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", 6.30584e-044);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", FLOAT_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", FLOAT_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW", -INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", 0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", -0.5);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", 2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", -2.4679999352);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", 6.30584e-044);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("FCTIW.", -INFINITY);
    OPTEST_2_COMPONENTS("FCTIWZ", 0.0);
    OPTEST_2_COMPONENTS("FCTIWZ", 0.5);
    OPTEST_2_COMPONENTS("FCTIWZ", -0.5);
    OPTEST_2_COMPONENTS("FCTIWZ", 2.4679999352);
    OPTEST_2_COMPONENTS("FCTIWZ", -2.4679999352);
    OPTEST_2_COMPONENTS("FCTIWZ", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FCTIWZ", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FCTIWZ", INFINITY);
    OPTEST_2_COMPONENTS("FCTIWZ", -INFINITY);
    OPTEST_2_COMPONENTS("FCTIWZ.", 0.0);
    OPTEST_2_COMPONENTS("FCTIWZ.", 0.5);
    OPTEST_2_COMPONENTS("FCTIWZ.", -0.5);
    OPTEST_2_COMPONENTS("FCTIWZ.", 2.4679999352);
    OPTEST_2_COMPONENTS("FCTIWZ.", -2.4679999352);
    OPTEST_2_COMPONENTS("FCTIWZ.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FCTIWZ.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FCTIWZ.", INFINITY);
    OPTEST_2_COMPONENTS("FCTIWZ.", -INFINITY);

    printf("\nFDIV Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIV.", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 10.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 4.9359998704, 2.4679999352);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 2.4679999352, 4.9359998704);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", 2.10195e-044, 2.45208e-029);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_SNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_SNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_QNAN, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_QNAN, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_SNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FDIVS.", -INFINITY, FLOAT_SNAN);

    printf("\nFMADD Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADD.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMADDS.", 1.0, 1.0, -INFINITY);

    printf("\nFMSUB Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUB.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FMSUBS.", 1.0, 1.0, -INFINITY);
    
    printf("\nFMUL Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMUL.", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS", -INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", -0.0, -0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 5.0, 5.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 0.25, 0.35);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 2.999999984523, 6.888239210233);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 5.0, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 5.0, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 5.0, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", 5.0, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", INFINITY, FLOAT_SNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FMULS.", -INFINITY, FLOAT_SNAN);

    printf("\nFNABS Variants\n");
    OPTEST_2_COMPONENTS("FNABS", 0.0);
    OPTEST_2_COMPONENTS("FNABS", -0.0);
    OPTEST_2_COMPONENTS("FNABS", -1.0);
    OPTEST_2_COMPONENTS("FNABS", -FLT_MIN);
    OPTEST_2_COMPONENTS("FNABS", -FLT_MAX);
    OPTEST_2_COMPONENTS("FNABS", -DBL_MIN);
    OPTEST_2_COMPONENTS("FNABS", -DBL_MAX);
    OPTEST_2_COMPONENTS("FNABS", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FNABS", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FNABS", INFINITY);
    OPTEST_2_COMPONENTS("FNABS", -INFINITY);
    OPTEST_2_COMPONENTS("FNABS.", 0.0);
    OPTEST_2_COMPONENTS("FNABS.", -0.0);
    OPTEST_2_COMPONENTS("FNABS.", -1.0);
    OPTEST_2_COMPONENTS("FNABS.", -FLT_MIN);
    OPTEST_2_COMPONENTS("FNABS.", -FLT_MAX);
    OPTEST_2_COMPONENTS("FNABS.", -DBL_MIN);
    OPTEST_2_COMPONENTS("FNABS.", -DBL_MAX);
    OPTEST_2_COMPONENTS("FNABS.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FNABS.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FNABS.", INFINITY);
    OPTEST_2_COMPONENTS("FNABS.", -INFINITY);

    printf("\nFNEG Variants\n");
    OPTEST_2_COMPONENTS("FNEG", 0.0);
    OPTEST_2_COMPONENTS("FNEG", -0.0);
    OPTEST_2_COMPONENTS("FNEG", DBL_MAX);
    OPTEST_2_COMPONENTS("FNEG", -DBL_MAX);
    OPTEST_2_COMPONENTS("FNEG", DBL_MIN);
    OPTEST_2_COMPONENTS("FNEG", -DBL_MIN);
    OPTEST_2_COMPONENTS("FNEG", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("FNEG", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("FNEG", INFINITY);
    OPTEST_2_COMPONENTS("FNEG", INFINITY);
    OPTEST_2_COMPONENTS("FNEG.", 0.0);
    OPTEST_2_COMPONENTS("FNEG.", -0.0);
    OPTEST_2_COMPONENTS("FNEG.", DBL_MAX);
    OPTEST_2_COMPONENTS("FNEG.", -DBL_MAX);
    OPTEST_2_COMPONENTS("FNEG.", DBL_MIN);
    OPTEST_2_COMPONENTS("FNEG.", -DBL_MIN);
    OPTEST_2_COMPONENTS("FNEG.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("FNEG.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("FNEG.", INFINITY);
    OPTEST_2_COMPONENTS("FNEG.", INFINITY);

    printf("\nFNMADD Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADD.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMADDS.", 1.0, 1.0, -INFINITY);

    printf("\nFNMSUB Variants\n");
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUB.", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS", 1.0, 1.0, -INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 1.0, 1.0, 1.0);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 5.5, 5.5, 5.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 7.3233339282, 7.9999999234, 3.5);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 6.30584e-044, 3.08286e-044, 1.54143e-044);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 1.0, 1.0, FLOAT_QNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 1.0, 1.0, FLOAT_SNAN);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 1.0, 1.0, INFINITY);
    OPTEST_4_COMPONENTS_WITH_ROUND("FNMSUBS.", 1.0, 1.0, -INFINITY);

    printf("\nFRES Variants\n");
    OPTEST_2_COMPONENTS("FRES", 0.0);
    OPTEST_2_COMPONENTS("FRES", -0.0);
    OPTEST_2_COMPONENTS("FRES", 100.986352178);
    OPTEST_2_COMPONENTS("FRES", 7.3233339282);
    OPTEST_2_COMPONENTS("FRES", 6.30584e-044);
    OPTEST_2_COMPONENTS("FRES", 69.0);
    OPTEST_2_COMPONENTS("FRES", 420.0);
    OPTEST_2_COMPONENTS("FRES", DBL_MAX);
    OPTEST_2_COMPONENTS("FRES", FLT_MAX);
    OPTEST_2_COMPONENTS("FRES", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("FRES", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("FRES", INFINITY);
    OPTEST_2_COMPONENTS("FRES", -INFINITY);
    OPTEST_2_COMPONENTS("FRES.", 0.0);
    OPTEST_2_COMPONENTS("FRES.", -0.0);
    OPTEST_2_COMPONENTS("FRES.", 100.986352178);
    OPTEST_2_COMPONENTS("FRES.", 7.3233339282);
    OPTEST_2_COMPONENTS("FRES.", 6.30584e-044);
    OPTEST_2_COMPONENTS("FRES.", 69.0);
    OPTEST_2_COMPONENTS("FRES.", 420.0);
    OPTEST_2_COMPONENTS("FRES.", DBL_MAX);
    OPTEST_2_COMPONENTS("FRES.", FLT_MAX);
    OPTEST_2_COMPONENTS("FRES.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("FRES.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("FRES.", INFINITY);
    OPTEST_2_COMPONENTS("FRES.", -INFINITY);

    printf("\nFRSP Variants\n");
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", -0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", DBL_MIN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", DBL_MAX);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", 5.6519082319399);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", 21321.94923489023);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP", -INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", 0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", -0.0);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", DBL_MIN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", DBL_MAX);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", 5.6519082319399);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", 21321.94923489023);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", INFINITY);
    OPTEST_2_COMPONENTS_WITH_ROUND("FRSP.", -INFINITY);

    printf("\nFRSQRTE Variants\n");
    OPTEST_2_COMPONENTS("FRSQRTE", 0.0);
    OPTEST_2_COMPONENTS("FRSQRTE", -0.0);
    OPTEST_2_COMPONENTS("FRSQRTE", 36.0);
    OPTEST_2_COMPONENTS("FRSQRTE", 100.0);
    OPTEST_2_COMPONENTS("FRSQRTE", 52324.23123123212);
    OPTEST_2_COMPONENTS("FRSQRTE", 2.66247e-044);
    OPTEST_2_COMPONENTS("FRSQRTE", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("FRSQRTE", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("FRSQRTE", INFINITY);
    OPTEST_2_COMPONENTS("FRSQRTE", -INFINITY);
    OPTEST_2_COMPONENTS("FRSQRTE.", 0.0);
    OPTEST_2_COMPONENTS("FRSQRTE.", -0.0);
    OPTEST_2_COMPONENTS("FRSQRTE.", 36.0);
    OPTEST_2_COMPONENTS("FRSQRTE.", 100.0);
    OPTEST_2_COMPONENTS("FRSQRTE.", 52324.23123123212);
    OPTEST_2_COMPONENTS("FRSQRTE.", 2.66247e-044);
    OPTEST_2_COMPONENTS("FRSQRTE.", DOUBLE_SNAN);
    OPTEST_2_COMPONENTS("FRSQRTE.", DOUBLE_QNAN);
    OPTEST_2_COMPONENTS("FRSQRTE.", INFINITY);
    OPTEST_2_COMPONENTS("FRSQRTE.", -INFINITY);

    printf("\nFSEL Variants\n");
    OPTEST_4_COMPONENTS("FSEL", 0.0, 0.0, 0.0);
    OPTEST_4_COMPONENTS("FSEL", 10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("FSEL", -10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("FSEL", 2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("FSEL", -2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_SNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_QNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_SNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_QNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_SNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_QNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("FSEL", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL", INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("FSEL", INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("FSEL", INFINITY, INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("FSEL", -INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL", -INFINITY, -INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL", -INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("FSEL", -INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_SNAN, DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("FSEL", DOUBLE_QNAN, DOUBLE_QNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("FSEL.", 0.0, 0.0, 0.0);
    OPTEST_4_COMPONENTS("FSEL.", 10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("FSEL.", -10.0, 50.0, 100.0);
    OPTEST_4_COMPONENTS("FSEL.", 2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("FSEL.", -2.66247e-044, 1.0, 2.0)
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_SNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_QNAN, 1.0, 2.0);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_SNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_QNAN, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_SNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_QNAN, -INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("FSEL.", INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("FSEL.", INFINITY, INFINITY, INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", -INFINITY, INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", -INFINITY, -INFINITY, -INFINITY);
    OPTEST_4_COMPONENTS("FSEL.", -INFINITY, DOUBLE_SNAN, DOUBLE_QNAN);
    OPTEST_4_COMPONENTS("FSEL.", -INFINITY, DOUBLE_QNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_SNAN, DOUBLE_SNAN, DOUBLE_SNAN);
    OPTEST_4_COMPONENTS("FSEL.", DOUBLE_QNAN, DOUBLE_QNAN, DOUBLE_QNAN);

    printf("\nFSUB Variants\n");
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUB.", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", 0.0, 0.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", 0.5, 0.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", 1.0, 1.0);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", 3.5, 3.5);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", 3.92364e-044, 3.92364e-044);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", DBL_MAX, DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", -DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", DBL_MAX, -DBL_MAX);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS_WITH_ROUND("FSUBS.", -INFINITY, -INFINITY);
}

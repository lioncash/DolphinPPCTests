#include <cmath>
#include <cstdio>
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
static u32 GetFPSCR()
{
    union {
        double f;
        unsigned int u[2];
    } cvt;

    cvt.f = 0.0;
    asm volatile ("mffs %[out]" : [out]"=f"(cvt.f));

    // Lower 32 bits are undefined according to the PPC reference.
    return cvt.u[1];
}

// Test for a 2-component instruction
// e.g. FABS frD, frB
#define OPTEST_2_COMPONENTS(inst, frA)                                         \
{                                                                              \
    double output;                                                             \
                                                                               \
    ClearFPSCR();                                                              \
    SetCR(0);                                                                  \
    asm volatile (inst " %[out], %[Fra]": [out]"=&f"(output) : [Fra]"f"(frA)); \
                                                                               \
    printf("%-8s :: frD %e | frA %e | FPSCR: 0x%08X | CR: 0x%08X\n",           \
           inst, output, frA, GetFPSCR(), GetCR());                            \
}

// Test for a 3-component instruction
// e.g. FADDS frD, frA, frB
#define OPTEST_3_COMPONENTS(inst, frA, frB)                                                            \
{                                                                                                      \
    double output;                                                                                     \
                                                                                                       \
    ClearFPSCR();                                                                                      \
    SetCR(0);                                                                                          \
    asm volatile (inst " %[out], %[Fra], %[Frb]" : [out]"=&f"(output) : [Fra]"f"(frA), [Frb]"f"(frB)); \
                                                                                                       \
    printf("%-8s :: frD %e | frA %e | frB %e | FPSCR: 0x%08X | CR: 0x%08X\n",                          \
           inst, output, frA, frB, GetFPSCR(), GetCR());                                               \
}

// Used for testing CMP instructions.
#define OPTEST_3_COMPONENTS_CMP(inst, frA, frB)                                 \
{                                                                               \
    ClearFPSCR();                                                               \
    SetCR(0);                                                                   \
    asm volatile (inst " cr1, %[Fra], %[Frb]": : [Fra]"f"(frA), [Frb]"f"(frB)); \
                                                                                \
    printf("%-8s :: frA %e | frB %e | FPSCR: 0x%08X | CR: 0x%08X\n",            \
           inst, frA, frB, GetFPSCR(), GetCR());                                \
}


void PPCFloatingPointTests()
{
    printf("FABS Variants\n");
    OPTEST_2_COMPONENTS("FABS", 0.0);
    OPTEST_2_COMPONENTS("FABS", -0.0);
    OPTEST_2_COMPONENTS("FABS", -1.0);
    OPTEST_2_COMPONENTS("FABS", -std::numeric_limits<float>::min());
    OPTEST_2_COMPONENTS("FABS", -std::numeric_limits<float>::max());
    OPTEST_2_COMPONENTS("FABS", -std::numeric_limits<double>::min());
    OPTEST_2_COMPONENTS("FABS", -std::numeric_limits<double>::max());
    OPTEST_2_COMPONENTS("FABS", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FABS", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FABS", INFINITY);
    OPTEST_2_COMPONENTS("FABS", -INFINITY);
    OPTEST_2_COMPONENTS("FABS.", 0.0);
    OPTEST_2_COMPONENTS("FABS.", -0.0);
    OPTEST_2_COMPONENTS("FABS.", -1.0);
    OPTEST_2_COMPONENTS("FABS.", -std::numeric_limits<float>::min());
    OPTEST_2_COMPONENTS("FABS.", -std::numeric_limits<float>::max());
    OPTEST_2_COMPONENTS("FABS.", -std::numeric_limits<double>::min());
    OPTEST_2_COMPONENTS("FABS.", -std::numeric_limits<double>::max());
    OPTEST_2_COMPONENTS("FABS.", FLOAT_QNAN);
    OPTEST_2_COMPONENTS("FABS.", FLOAT_SNAN);
    OPTEST_2_COMPONENTS("FABS.", INFINITY);
    OPTEST_2_COMPONENTS("FABS.", -INFINITY);

    printf("\nFADD Variants\n");
    OPTEST_3_COMPONENTS("FADD", 0.0, 0.0);
    OPTEST_3_COMPONENTS("FADD", 0.5, 0.5);
    OPTEST_3_COMPONENTS("FADD", 1.0, 1.0);
    OPTEST_3_COMPONENTS("FADD", 3.5, 3.5);
    OPTEST_3_COMPONENTS("FADD", std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADD", -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADD", std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADD", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADD", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADD", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS("FADD", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADD", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADD", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADD", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS("FADD", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADD", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS("FADD.", 0.0, 0.0);
    OPTEST_3_COMPONENTS("FADD.", 0.5, 0.5);
    OPTEST_3_COMPONENTS("FADD.", 1.0, 1.0);
    OPTEST_3_COMPONENTS("FADD.", 3.5, 3.5);
    OPTEST_3_COMPONENTS("FADD.", std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADD.", -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADD.", std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADD.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADD.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADD.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS("FADD.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADD.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADD.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADD.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS("FADD.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADD.", -INFINITY, -INFINITY);

    OPTEST_3_COMPONENTS("FADDS", 0.0, 0.0);
    OPTEST_3_COMPONENTS("FADDS", 0.5, 0.5);
    OPTEST_3_COMPONENTS("FADDS", 1.0, 1.0);
    OPTEST_3_COMPONENTS("FADDS", 3.5, 3.5);
    OPTEST_3_COMPONENTS("FADDS", std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    OPTEST_3_COMPONENTS("FADDS", -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    OPTEST_3_COMPONENTS("FADDS", std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    OPTEST_3_COMPONENTS("FADDS", 0.0, std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADDS", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADDS", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADDS", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS("FADDS", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADDS", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADDS", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADDS", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS("FADDS", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADDS", -INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", 0.0, 0.0);
    OPTEST_3_COMPONENTS("FADDS.", 0.5, 0.5);
    OPTEST_3_COMPONENTS("FADDS.", 1.0, 1.0);
    OPTEST_3_COMPONENTS("FADDS.", 3.5, 3.5);
    OPTEST_3_COMPONENTS("FADDS.", std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    OPTEST_3_COMPONENTS("FADDS.", -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    OPTEST_3_COMPONENTS("FADDS.", std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    OPTEST_3_COMPONENTS("FADDS.", 0.0, std::numeric_limits<double>::max());
    OPTEST_3_COMPONENTS("FADDS.", FLOAT_SNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", FLOAT_QNAN, INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", FLOAT_QNAN, -INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADDS.", -INFINITY, FLOAT_QNAN);
    OPTEST_3_COMPONENTS("FADDS.", INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", INFINITY, -INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", -INFINITY, INFINITY);
    OPTEST_3_COMPONENTS("FADDS.", -INFINITY, -INFINITY);

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
}

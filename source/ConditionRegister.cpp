#ifdef MACOS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
#include <cinttypes>
#endif
#include <cstdio>

#include "Tests.h"
#include "Utils.h"

// CR fields are only 4 bits in size.
// The silver lining with this is that
// it's sensibly possible to do exhaustive input
// for several condition field instructions.
#define CR_FIELD_MAX_VALUE 15

// Defines for specific CR field masks.
// These are necessary because CR fields are defined as a mask,
// as such, it's considered an immediate. So we use preprocessor
// copy-paste magic to keep the readability around.
#define MASK_CR0 0x80
#define MASK_CR1 0x40
#define MASK_CR2 0x20
#define MASK_CR3 0x10
#define MASK_CR4 0x08
#define MASK_CR5 0x04
#define MASK_CR6 0x02
#define MASK_CR7 0x01

// Note: This'll generate wrong values on little-endian PPC.
//       So, uhh... don't use this on little-endian.
#define GetShiftValue(Field) \
    (28-((Field) * 4))

#define SetCRField(cr_field, value) \
    asm volatile ("mtcrf %[mask], %[val]" :: [mask]"I"(1<<(7-(cr_field))), [val]"r"(value << GetShiftValue(cr_field)))

// Since we evaluate all of the bits of a 4-bit field, this simplifies resetting the clean test state.
#define SetupPreTest(CRAField, CRBField, CRAValue, CRBValue) \
{                                                          \
    SetXER(0);                                             \
    SetCR(0);                                              \
    SetCRField(CRAField, CRAValue);                        \
    SetCRField(CRBField, CRBValue);                        \
}

// Tests a 3 component instruction
// e.g. CROR crD, crA, crB
// Destination is always CR0
#define OPTEST_3_COMPONENTS(inst, CRAField, CRBField)                                                                   \
{                                                                                                                       \
    uint32_t cr;                                                                                                        \
    printf("%s\n", upper(inst));                                                                                        \
                                                                                                                        \
    for (uint32_t i = 0; i <= CR_FIELD_MAX_VALUE; i++)                                                                  \
    {                                                                                                                   \
        for (uint32_t j = 0; j <= CR_FIELD_MAX_VALUE; j++)                                                              \
        {                                                                                                               \
            SetupPreTest(CRAField, CRBField, i, j);                                                                     \
            asm volatile (inst " 0, 4*" #CRAField "+0, 4*" #CRBField "+0" ::: "cr0");                                   \
            GetCR(cr);                                                                                                  \
            printf("     Bit 0 ::  crA 0x%08" PRIX32 " | crB 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", i, j, cr);       \
                                                                                                                        \
            SetupPreTest(CRAField, CRBField, i, j);                                                                     \
            asm volatile (inst " 0, 4*" #CRAField "+1, 4*" #CRBField "+1" ::: "cr0");                                   \
            GetCR(cr);                                                                                                  \
            printf("     Bit 1 ::  crA 0x%08" PRIX32 " | crB 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", i, j, cr);       \
                                                                                                                        \
            SetupPreTest(CRAField, CRBField, i, j);                                                                     \
            asm volatile (inst " 0, 4*" #CRAField "+2, 4*" #CRBField "+2" ::: "cr0");                                   \
            GetCR(cr);                                                                                                  \
            printf("     Bit 2 ::  crA 0x%08" PRIX32 " | crB 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", i, j, cr);       \
                                                                                                                        \
            SetupPreTest(CRAField, CRBField, i, j);                                                                     \
            asm volatile (inst " 0, 4*" #CRAField "+3, 4*" #CRBField "+3" ::: "cr0");                                   \
            GetCR(cr);                                                                                                  \
            printf("     Bit 3 ::  crA 0x%08" PRIX32 " | crB 0x%08" PRIX32 " | CR: 0x%08" PRIX32 "\n", i, j, cr);       \
        }                                                                                                               \
    }                                                                                                                   \
}

void PPCConditionRegisterTests()
{
    printf("Condition Register Tests\n");

    OPTEST_3_COMPONENTS("crand" , 1, 2);
    OPTEST_3_COMPONENTS("crandc", 1, 2);
    OPTEST_3_COMPONENTS("creqv" , 1, 2);
    OPTEST_3_COMPONENTS("crnand", 1, 2);
    OPTEST_3_COMPONENTS("crnor" , 1, 2);
    OPTEST_3_COMPONENTS("cror"  , 1, 2);
    OPTEST_3_COMPONENTS("crorc" , 1, 2);
    OPTEST_3_COMPONENTS("crxor" , 1, 2);
}

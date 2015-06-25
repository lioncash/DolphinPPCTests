#include <cstdio>

#include "Tests.h"

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
static inline u32 GetShiftValue(u32 mask)
{
    switch (mask)
    {
    case 0x80:
        return 28U;
    case 0x40:
        return 24U;
    case 0x20:
        return 20U;
    case 0x10:
        return 16U;
    case 0x08:
        return 12U;
    case 0x04:
        return 8U;
    case 0x02:
        return 4U;
    default: // 0x01
        return 0U;
    }
}

#define SetCRField(cr_mask, value) \
    asm volatile ("mtcrf %[mask], %[val]" :: [mask]"I"(cr_mask), [val]"r"(value << GetShiftValue(cr_mask)))

// Since we evaluate all of the bits of a 4-bit field, this simplifies resetting the clean test state.
#define SetupPreTest(CRAMask, CRBMask, CRAValue, CRBValue) \
{                                                          \
    SetXER(0);                                             \
    SetCR(0);                                              \
    SetCRField(CRAMask, CRAValue);                         \
    SetCRField(CRBMask, CRBValue);                         \
}

// Tests a 3 component instruction
// e.g. CROR crD, crA, crB
// Destination is always CR0
#define OPTEST_3_COMPONENTS(inst, CRAMask, CRBMask, CRAField, CRBField)                      \
{                                                                                            \
    printf("%s\n", inst);                                                                    \
                                                                                             \
    for (int i = 0; i <= CR_FIELD_MAX_VALUE; i++)                                            \
    {                                                                                        \
        for (int j = 0; j <= CR_FIELD_MAX_VALUE; j++)                                        \
        {                                                                                    \
            SetupPreTest(CRAMask, CRBMask, i, j);                                            \
            asm volatile (inst " cr0, 4*cr" #CRAField "+0, 4*cr" #CRBField "+0" ::: "cr0");  \
            printf("     Bit 0 ::  crA 0x%08X | crB 0x%08X | CR: 0x%08X\n", i, j, GetCR());  \
                                                                                             \
            SetupPreTest(CRAMask, CRBMask, i, j);                                            \
            asm volatile (inst " cr0, 4*cr" #CRAField "+1, 4*cr" #CRBField "+1" ::: "cr0");  \
            printf("     Bit 1 ::  crA 0x%08X | crB 0x%08X | CR: 0x%08X\n", i, j, GetCR());  \
                                                                                             \
            SetupPreTest(CRAMask, CRBMask, i, j);                                            \
            asm volatile (inst " cr0, 4*cr" #CRAField "+2, 4*cr" #CRBField "+2" ::: "cr0");  \
            printf("     Bit 2 ::  crA 0x%08X | crB 0x%08X | CR: 0x%08X\n", i, j, GetCR());  \
                                                                                             \
            SetupPreTest(CRAMask, CRBMask, i, j);                                            \
            asm volatile (inst " cr0, 4*cr" #CRAField "+3, 4*cr" #CRBField "+3" ::: "cr0");  \
            printf("     Bit 3 ::  crA 0x%08X | crB 0x%08X | CR: 0x%08X\n", i, j, GetCR());  \
        }                                                                                    \
    }                                                                                        \
}

void PPCConditionRegisterTests()
{
    printf("\n\nCondition Register Tests\n\n");

    OPTEST_3_COMPONENTS("CRAND",  MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CRANDC", MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CREQV",  MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CRNAND", MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CRNOR",  MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CROR",   MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CRORC",  MASK_CR1, MASK_CR2, 1, 2);
    OPTEST_3_COMPONENTS("CRXOR",  MASK_CR1, MASK_CR2, 1, 2);
}
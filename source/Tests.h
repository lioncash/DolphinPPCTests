#pragma once

#include <cstdint>

static inline uint32_t GetCR()
{
    uint32_t reg;
    asm volatile ("mfcr %[out]" : [out]"=&r"(reg));
    return reg;
}

static inline void SetCR(uint32_t value)
{
    asm volatile ("mtcr %[val]" : : [val]"r"(value) : "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7");
}

static inline uint32_t GetXER()
{
    uint32_t xer;
    asm volatile ("mfxer %[out]" : [out]"=&r"(xer));
    return xer;
}

static inline void SetXER(uint32_t value)
{
    asm volatile ("mtxer %[val]" : : [val]"r"(value) : "xer");
}

void PPCFloatingPointTests();
void PPCIntegerTests();
void PPCConditionRegisterTests();
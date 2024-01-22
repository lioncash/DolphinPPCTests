#pragma once

#ifdef MACOS
#include <stdint.h>
#else
#include <cstdint>
#endif

#define GetCR(reg) \
    asm volatile ("mfcr %[out]" : [out]"=&r"(reg))

#define SetCR(value) \
    asm volatile ("mtcr %[val]" : : [val]"r"(value) : "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7")

#define GetXER(xer) \
    asm volatile ("mfxer %[out]" : [out]"=&r"(xer))

#define SetXER(value) \
    asm volatile ("mtxer %[val]" : : [val]"r"(value) : "xer")

void PPCFloatingPointTests();
void PPCIntegerTests();
void PPCConditionRegisterTests();

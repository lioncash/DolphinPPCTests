#pragma once

#include <cstdint>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

static inline u32 GetCR()
{
    u32 reg;
    asm volatile ("mfcr %[out]" : [out]"=b"(reg));
    return reg;
}

static inline void SetCR(u32 value)
{
    asm volatile ("mtcr %[val]" : : [val]"b"(value) : "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7");
}

void PPCFloatingPointTests();
void PPCIntegerTests();
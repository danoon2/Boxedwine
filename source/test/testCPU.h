#ifndef __TEST_CPU_H__
#define __TEST_CPU_H__

void newInstruction(int flags);
void pushCode8(int value);
void pushCode16(int value);
void pushCode32(int value);
void runTestCPU();
void failed(const char* msg, ...);

extern CPU* cpu;

#define FLAG_MASK (AF|CF|SF|PF|ZF|OF)

#define STACK_ADDRESS 0xE0000000
#define HEAP_ADDRESS 0xF0000000
#define CODE_ADDRESS 0xD0000000
#define HEAP_SEG 0x213
#define CODE_SEG 0x223
#define STACK_SEG 0x233
#define CODE_SEG_16 0x243

#define DEFAULT 0xDEADBEEF

struct Test_Float {
    union {
        U32   i;
        float f;
    };
};

struct TestDouble {
    union {
        U64   i;
        double d;
    };
};

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)   
__m128i floatTo128(float f1, float f2, float f3, float f4);
#endif

extern const U32 FLOAT_POSITIVE_INFINITY_BITS;
extern const U32 FLOAT_NEGATIVE_INFINITY_BITS;
extern const U32 FLOAT_QUIET_NAN_BITS;
extern const U64 DOUBLE_QUIET_NAN_BITS;

extern const float POSITIVE_INFINITY;
extern const float NEGATIVE_INFINITY;
extern const float TEST_NAN;
extern const double TEST_NAN_DOUBLE;

#endif

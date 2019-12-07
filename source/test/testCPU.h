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

#define STACK_ADDRESS 0xE0011000
#define HEAP_ADDRESS 0xF0000000
#define CODE_ADDRESS 0xD0000000

#define DEFAULT 0xDEADBEEF

struct Test_Float {
    union {
        float f;
        U32   i;
    };
};

struct TestDouble {
    union {
        double d;
        U64   i;
    };
};

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)   
__m128i floatTo128(float f1, float f2, float f3, float f4);
#endif

extern const U32 FLOAT_POSITIVE_INFINITY_BITS;
extern const U32 FLOAT_NEGATIVE_INFINITY_BITS;
extern const U32 FLOAT_NAN_BITS;

#define POSITIVE_INFINITY *(const float *)&FLOAT_POSITIVE_INFINITY_BITS
#define NEGATIVE_INFINITY *(const float *)&FLOAT_NEGATIVE_INFINITY_BITS
#define TEST_NAN *(const float *)&FLOAT_NAN_BITS

#endif
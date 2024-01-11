#include "boxedwine.h"

#ifdef __TEST

#ifdef BOXEDWINE_MSVC
#include <nmmintrin.h>
#endif

#include "testCPU.h"
#include "testSSE.h"

extern KMemory* memory;

void testSse2MovUps110() {  
    testSse128(0, 0x66, 0x10, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSse2MovSd310() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x24680bdf13579acel));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        movsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movsd failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x0000000000000000l));
    __asm {
        movups xmm1, d1
        movsd xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x10, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xaabbccddeeff2468l, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x0000000000000000l);
}

void testSse2MovPd111() {  
    testSse128r(0, 0x66, 0x11, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSse2MovSd311() {
    // 8 byte write when going to memory
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x24680bdf13579acel));

    __asm {
        movups xmm1, d
        movsd result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movsd failed");
    }
#endif 
    testSse128r(0, 0xf2, 0x11, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xaabbccddeeff2468l, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x24680bdf13579acel);
}

void testSse2MovLpd112() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;

    __asm {
        movups xmm1, d2
        movlpd xmm1, d1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x12, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0x1234567890abcdefl, 0x1122334455667788l);
}

void testSse2MovLpd113() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i result = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x1122334455667788l));

    __asm {
        movups xmm1, d1
        movlpd result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlpd failed");
    }
#endif 
    testSse128r(0, 0x66, 0x13, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0x1234567890abcdefl, 0x1122334455667788l);
}

void testSse2Unpcklpd114() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468), _mm_set_pi64x(0x1234567890abcdef));
    __m128i result;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        unpcklpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("unpcklpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x14, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468, 0x1234567890abcdef);
}

void testSse2Unpckhpd115() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x1122334455667788), _mm_set_pi64x(0x24680bdf13579ace));
    __m128i result;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        unpckhpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("unpckhp failed");
    }
#endif 
    testSse128(0, 0x66, 0x15, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0x1122334455667788, 0x24680bdf13579ace);
}

void testSse2Movhpd116() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468), _mm_set_pi64x(0x1234567890abcdef));
    __m128i result;

    __asm {
        movups xmm1, d1
        movhpd xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movhpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x16, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel,  0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0xaabbccddeeff2468, 0x1234567890abcdef);
}

void testSse2Movhpd117() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x24680bdf13579ace), _mm_set_pi64x(0x1122334455667788));

    __asm {
        movups xmm1, d2
        movhpd d1, xmm1
        emms
    }
    if (memcmp(&d1, &expected, 16)) {
        failed("Movhpd failed");
    }
#endif 
    testSse128r(0, 0x66, 0x17, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0x24680bdf13579ace, 0x1122334455667788);
}

void testSse2Movapd128() {
    testSse128(0, 0x66, 0x28, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSse2Movapd129() {
    testSse128r(0, 0x66, 0x29, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSse2Cvtpi2pd12a() {
    U64 d2 = ((U64)((U32)(-5000)) << 32) | 5000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));    
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x40b3880000000000), _mm_set_pi64x(0xc0b3880000000000));
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtpi2pd xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cvtpi2pd failed");
    }
#endif 
    testSseMmx64(0, 0x66, 0x2a, 0xaabbccddeeff2468l, 0x1122334455667788l, d2, 0x40b3880000000000, 0xc0b3880000000000);
}

void testSse2Cvtsi2sd32a() {
    U32 d2 = (U32)(-5000);
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));    
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xc0b3880000000000), _mm_set_pi64x(0x1122334455667788));
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtsi2sd xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtsi2sd failed");
    }
#endif 
    testSseReg32(0, 0xF2, 0x2a, 0xaabbccddeeff2468l, 0x1122334455667788l, d2, 0xc0b3880000000000, 0x1122334455667788);
}

void testSse2Movntpd12b() {
    testSse128r(0, 0x66, 0x2b, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSseCvttpd2pi12c() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 12345678900.0; // should result in indefinite integer, 80000000
    f2.d = -5000.6; // should tuncate

    U64 expected = 0xffffec7880000000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    U64 result;
    __asm {
        movups xmm1, d1
        cvttpd2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result!=expected) {
        failed("Cvttpd2pi failed");
    }
#endif 
    testSseMmx64r(0, 0x66, 0x2c, 0, f1.i, f2.i, expected);

    f1.d = -12345678900.0; // should result in indefinite integer, 80000000
    f2.d = 5000.6; // should tuncate

    expected = 0x0000138880000000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __asm {
        movups xmm1, d1
        cvttpd2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result != expected) {
        failed("Cvttpd2pi failed");
    }
#endif 
    testSseMmx64r(0, 0x66, 0x2c, 0, f1.i, f2.i, expected);
}

void testSse2Cvttsd2si32c() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;

    f1.d = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.d = -5000.6f; // should truncate
    f3.d = -12345678900.0f; // should result in indefinite integer, 80000000

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(0, 1, 2, 3);    
    d1.m128i_u64[0] = f1.i;
    U32 expected = 0x80000000;
    U32 result;

    __asm {
        movups xmm1, d1
        cvttsd2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Cvttss2si failed");
    }

    d1.m128i_u64[0] = f2.i;
    expected = 0xffffec78;

    __asm {
        movups xmm1, d1
        cvttsd2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Cvttss2si failed");
    }
    d1.m128i_u64[0] = f3.i;
    expected = 0x80000000;

    __asm {
        movups xmm1, d1
        cvttsd2si eax, xmm1
        mov result, eax
        emms
    }
    if (result != expected) {
        failed("Cvttss2si failed");
    }
#endif
    testSseReg32r(0, 0xF2, 0x2c, 0,   f1.i, ((U64)2) << 32 | 1, 0x80000000);
    testSseReg32r(0, 0xF2, 0x2c, 0,  f2.i, ((U64)2) << 32 | 1, 0xffffec78);
    testSseReg32r(0, 0xF2, 0x2c, 0, f3.i, ((U64)2) << 32 | 1, 0x80000000);
}

void testSse2Cvtpd2pi12d() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;

    f1.d = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.d = -5000.0f; // :TODO: test rounding

    f3.d = -12345678900.0f; // should result in indefinite integer, 80000000
    f4.d = 5000.0f; // :TODO: test rounding

    U64 expected = 0xffffec7880000000;
    U64 expected2 = 0x0000138880000000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    U64 result;

    __asm {
        movups xmm1, d1
        cvtpd2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result!=expected) {
        failed("Cvtpd2pi failed");
    }

    d1.m128i_u64[0] = f3.i;
    d1.m128i_u64[1] = f4.i;

    __asm {
        movups xmm1, d1
        cvtpd2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result != expected2) {
        failed("Cvtpd2pi failed");
    }
#endif 
    testSseMmx64r(0, 0x66, 0x2d, 0, f1.i, f2.i, expected);
    testSseMmx64r(0, 0x66, 0x2d, 0, f3.i, f4.i, expected2);
}

void testSse2Cvtsd2si32d() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.d = -5000.0f; // :TODO: test rounding

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    U32 expected = 0x80000000;
    U32 result;

    __asm {
        movups xmm1, d1
        cvtsd2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("cvtsd2si failed");
    }

    d1.m128i_u64[0] = f2.i;
    expected = 0xffffec78;

    __asm {
        movups xmm1, d1
        cvttsd2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("cvtsd2si failed");
    }
#endif
    testSseReg32r(0, 0xF2, 0x2d, 0,  f1.i, ((U64)2) << 32 | 1, 0x80000000);
    testSseReg32r(0, 0xF2, 0x2d, 0,  f2.i, ((U64)2) << 32 | 1, 0xffffec78);
}

// :TODO: test exceptions?
void testSse2Ucomisd12e() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 1;
    f2.d = 2;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(0, 1, 2, 3);    
    __m128i d2 = _mm_setr_epi32(0, 1, 2, 3);    
    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i;
    U32 result;
    U32 expected = CF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("ucomisd failed");
    }

    f1.d = -1;
    f2.d = -1;

    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i;   
    expected = ZF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("ucomisd failed");
    }

    f1.d = 1;
    f2.d = -1;

    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i; 
    expected = 0;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("ucomisd failed");
    }
 
    f1.i = 0x7ff8000000000000;
    f2.d = -1;

    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i;   
    expected = ZF | PF | CF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("ucomisd failed");
    }
#endif
    f1.d = 1;
    f2.d = 2;
    testSse128f(0, 0x66, 0x2e, f1.i, 0, f2.i, 0, CF);
    f1.d = -1;
    f2.d = -1;
    testSse128f(0, 0x66, 0x2e, f1.i, 0, f2.i, 0, ZF);
    f1.d = 1;
    f2.d = -1;
    testSse128f(0, 0x66, 0x2e, f1.i, 0, f2.i, 0, 0);
    f1.i = 0x7ff8000000000000;
    f2.d = -1;
    testSse128f(0, 0x66, 0x2e, f1.i, 0, f2.i, 0, ZF | PF | CF);
}

void testSse2Comisd12f() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 1;
    f2.d = 2;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(0, 1, 2, 3);    
    __m128i d2 = _mm_setr_epi32(0, 1, 2, 3);    
    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i; 
    U32 result;
    U32 expected = CF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        comisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Comisd failed");
    }

    f1.d = -1;
    f2.d = -1;

    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i; 
    expected = ZF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        comisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Comisd failed");
    }

    f1.d = 1;
    f2.d = -1;

    d1.m128i_u64[0] = f1.i;
    d2.m128i_u64[0] = f2.i; 
    expected = 0;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        comisd xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Comisd failed");
    }
#endif
    f1.d = 1;
    f2.d = 2;
    testSse128f(0, 0x66, 0x2f, f1.i, 0, f2.i, 0, CF);
    f1.d = -1;
    f2.d = -1;
    testSse128f(0, 0x66, 0x2f, f1.i, 0, f2.i, 0, ZF);
    f1.d = 1;
    f2.d = -1;
    testSse128f(0, 0x66, 0x2f, f1.i, 0, f2.i, 0, 0);
}

void testSse2Movmskpd150() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;

    f1.i = 0x7ff8000000000000;
    f2.d = -5000.0f;
    f3.d = -1.0;
    f4.d = 0.0f;
    
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i; 
    U32 expected = 2;
    U32 result;

    __asm {
        movups xmm1, d1
        movmskpd eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Movmskpd failed");
    }

    d1.m128i_u64[0] = f3.i;
    d1.m128i_u64[1] = f4.i; 
    expected = 1;
    __asm {
        movups xmm1, d1
        movmskpd eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Movmskpd failed");
    }
#endif
    testSseReg32r(0, 0x66, 0x50, 0, f1.i, f2.i, 2, 0xFFFFFFFF);
    testSseReg32r(0, 0x66, 0x50, 0, f3.i, f4.i, 1, 0xFFFFFFFF);
}

void testSse2Sqrtpd151() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;

    f1.d = 4.0;
    f2.d = 2.0;
    f3.d = sqrt(2.0);
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected;
    __m128i result;
    __m128i d1;
    __m128i d2;
    d2.m128i_u64[0] = f1.i;
    d2.m128i_u64[1] = f2.i;
    expected.m128i_u64[0] = f2.i;
    expected.m128i_u64[1] = f3.i;
    __asm {
        movups xmm1, d1
        movups xmm2, d2
        sqrtpd xmm1, xmm2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Sqrtps failed");
    }
#endif
    testSse128(0, 0x66, 0x51, 0, 0, f1.i, f2.i, f2.i, f3.i);
}

void testSse2Sqrtsd351() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128d expected;
    expected.m128d_f64[0] = 16.0;
    expected.m128d_f64[1] = 2.0;
    __m128d result;
    __m128d d1;
    d1.m128d_f64[0] = 256.0;
    d1.m128d_f64[1] = 2.0;

    __asm {
        movups xmm1, d1
        sqrtsd xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Sqrtsd failed");
    }
#endif
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;

    f1.d = 256.0;
    f2.d = 2.0;
    f3.d = 16.0;
    testSse128(0, 0xf2, 0x51, f2.i, f2.i, f1.i, f2.i, f3.i, f2.i);
}

void testSse2Andpd154() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x0230445880ab0468), _mm_set_pi64x(0x0020034411461288));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        andpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Andpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x54, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x0230445880ab0468, 0x0020034411461288);
}

void testSse2Andnpd155() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xa88b88856e542000), _mm_set_pi64x(0x1102300044206500));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        andnpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Andnpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x55, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xa88b88856e542000, 0x1102300044206500);
}

void testSse2Orpd156() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xbabfdefdfeffedef), _mm_set_pi64x(0x356a3bdf5777ffce));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        orpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Orpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x56, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xbabfdefdfeffedef, 0x356a3bdf5777ffce);
}

void testSse2Xorpd157() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xb88f9aa57e54e987), _mm_set_pi64x(0x354a389b4631ed46));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        xorpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Xorpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x57, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xb88f9aa57e54e987, 0x354a389b4631ed46);
}

void testSse2Addpd158() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;
    TestDouble f6;

    f1.d = 4.0;
    f2.d = 9.0;
    f3.d = -4.0;
    f4.d = 10.0;
    f5.d = 0.0;
    f6.d = 19.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __m128i d2;
    d2.m128i_u64[0] = f3.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f6.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        addpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Addpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x58, f1.i, f2.i, f3.i, f4.i, f5.i, f6.i);
}

void testSse2Addsd358() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;

    f1.d = 4.0;
    f2.d = 9.0;
    f3.d = -4.0;
    f4.d = 10.0;
    f5.d = 0.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __m128i d2;
    d2.m128i_u64[0] = f3.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f2.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        addsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Addsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x58, f1.i, f2.i, f3.i, f4.i, f5.i, f2.i);
}

void testSse2Mulpd159() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;
    TestDouble f6;

    f1.d = 4.0;
    f2.d = 9.0;
    f3.d = -4.0;
    f4.d = 10.0;
    f5.d = -16.0;
    f6.d = 90.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __m128i d2;
    d2.m128i_u64[0] = f3.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f6.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        mulpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("mulpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x59, f1.i, f2.i, f3.i, f4.i, f5.i, f6.i);
}

void testSse2Mulsd359() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;

    f1.d = 4.0;
    f2.d = 9.0;
    f3.d = -4.0;
    f4.d = 10.0;
    f5.d = -16.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __m128i d2;
    d2.m128i_u64[0] = f3.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f2.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        mulsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("mulsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x59, f1.i, f2.i, f3.i, f4.i, f5.i, f2.i);
}

void testSse2Cvtpd2ps15a() {
    TestDouble f1;
    TestDouble f2;
    Test_Float f3;
    Test_Float f4;

    f1.d = -50.0;
    f2.d = 175.0;
    f3.f = -50.0;
    f4.f = 175.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __m128i expected;
    expected.m128i_u32[0] = f3.i;
    expected.m128i_u32[1] = f4.i;
    expected.m128i_u32[2] = 0;
    expected.m128i_u32[3] = 0;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtpd2ps xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cvtpd2ps failed");
    }
#endif 
    testSse128(0, 0x66, 0x5a, 0, 0, f1.i, f2.i, 0x432f0000c2480000, 0);
}

void testSse2Cvtps2pd35a() {
    TestDouble f1;
    TestDouble f2;
    Test_Float f3;
    Test_Float f4;

    f1.d = -50.0;
    f2.d = 175.0;
    f3.f = -50.0;
    f4.f = 175.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u32[0] = f3.i;
    d1.m128i_u32[1] = f4.i;
    d1.m128i_u64[1] = 0;
    __m128i expected;
    expected.m128i_u64[0] = f1.i;
    expected.m128i_u64[1] = f2.i;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtps2pd xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtps2pd failed");
    }
#endif 
    testSse128(0, 0, 0x5a, 0, 0, f3.i | (U64)f4.i << 32, 0, f1.i, f2.i);
}

void testSse2Cvtsd2ss35a() {
    TestDouble f1;
    TestDouble f2;
    Test_Float f3;
    Test_Float f4;

    f1.d = -50.0;
    f2.d = 175.0;
    f3.f = -50.0;
    f4.f = 175.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f2.i;
    __m128i expected;
    expected.m128i_u32[0] = f3.i;
    expected.m128i_u32[1] = d1.m128i_i32[1]; // remains unchanged
    expected.m128i_u64[1] = f2.i;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtsd2ss xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtsd2ss failed");
    }
#endif 
    testSse128(0, 0xf2, 0x5a, 0x1122334455667788, 0x99aabbccddeeff00, f1.i, f2.i, 0x11223344c2480000, 0x99aabbccddeeff00);
}

void testSse2Cvtss2sd35a() {
    TestDouble f1;
    TestDouble f2;
    Test_Float f3;
    Test_Float f4;

    f1.d = -50.0;
    f2.d = 175.0;
    f3.f = -50.0;
    f4.f = 175.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u32[0] = f3.i;
    d1.m128i_u32[1] = f4.i;
    d1.m128i_u64[1] = 0x1122334455667788;
    __m128i expected;
    expected.m128i_u64[0] = f1.i;
    expected.m128i_u64[1] = 0x1122334455667788;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtss2sd xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtss2sd failed");
    }
#endif 
    testSse128(0, 0xf3, 0x5a, 0, 0x1122334455667788, f3.i | (U64)f4.i << 32, 0, f1.i, 0x1122334455667788);
}

void testSse2Cvtps2dq15b() {
    Test_Float f1;
    Test_Float f2;
    Test_Float f3;
    Test_Float f4;
    
    f1.f = 2147483648.0f;
    f2.f = -50.0f;
    f3.f = 0.0f;
    f4.f = -2147483649.0f;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u32[0] = f1.i;
    d1.m128i_u32[1] = f2.i;
    d1.m128i_u32[2] = f3.i;
    d1.m128i_u32[3] = f4.i;
    __m128i expected;
    expected.m128i_i32[0] = 0x80000000;
    expected.m128i_i32[1] = -50;
    expected.m128i_i32[2] = 0;
    expected.m128i_u32[3] = 0x80000000;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtps2dq xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtps2dq failed");
    }
#endif 
    testSse128(0, 0x66, 0x5b, 0, 0, f1.i | (U64)f2.i << 32, f3.i | (U64)f4.i << 32, 0xffffffce80000000, 0x8000000000000000);
}

void testSse2Cvtdq2ps35b() {
    Test_Float f1;
    Test_Float f2;
    Test_Float f3;
    Test_Float f4;

    f1.f = -50.0;
    f2.f = 175.0;
    f3.f = 0.0;
    f4.f = 1000000.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;    
    __m128i expected;
    d1.m128i_i32[0] = -50;
    d1.m128i_i32[1] = 175;
    d1.m128i_i32[2] = 0;
    d1.m128i_i32[3] = 1000000;
    expected.m128i_u32[0] = f1.i;
    expected.m128i_u32[1] = f2.i;
    expected.m128i_u32[2] = f3.i;
    expected.m128i_u32[3] = f4.i;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtdq2ps xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtdq2ps failed");
    }
#endif 
    testSse128(0, 0, 0x5b, 0, 0, 0x000000afffffffce, 0x000f424000000000, f1.i | (U64)f2.i << 32, f3.i | (U64)f4.i << 32);
}

void testSse2Cvttps2dq35b() {
    Test_Float f1;
    Test_Float f2;
    Test_Float f3;
    Test_Float f4;

    f1.f = -50.1f;
    f2.f = 2147483648.0f;
    f3.f = -2147483649.0f;
    f4.f = 1000000.4f;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u32[0] = f1.i;
    d1.m128i_u32[1] = f2.i;
    d1.m128i_u32[2] = f3.i;
    d1.m128i_u32[3] = f4.i;
    __m128i expected;
    expected.m128i_i32[0] = -50;
    expected.m128i_u32[1] = 0x80000000;
    expected.m128i_u32[2] = 0x80000000;
    expected.m128i_u32[3] = 1000000;
    __m128i result;

    __asm {
        movups xmm1, d1
        cvttps2dq xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtps2dq failed");
    }
#endif 
    testSse128(0, 0xf3, 0x5b, 0, 0, f1.i | (U64)f2.i << 32, f3.i | (U64)f4.i << 32, 0x80000000ffffffce, 0x000f424080000000);
}

void testSse2Subpd15c() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;
    TestDouble f6;

    f1.d = 4.0;
    f2.d = -5.0;
    f3.d = -0.1;
    f4.d = 2.0;
    f5.d = 9.0;
    f6.d = -2.1;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f6.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        subpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Subpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x5c, f1.i, f3.i, f2.i, f4.i, f5.i, f6.i);
}

void testSse2Subsd35c() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;

    f1.d = 4.0;
    f2.d = -5.0;
    f3.d = -0.1;
    f4.d = 2.0;
    f5.d = 9.0;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f3.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        subsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Subsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x5c, f1.i, f3.i, f2.i, f4.i, f5.i, f3.i);
}

void testSse2Minpd15d() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;
    TestDouble f6;
    TestDouble f7;
    TestDouble f8;
    TestDouble f9;
    TestDouble f10;
    TestDouble f11;
    TestDouble f12;

    f1.d = 4.0;
    f2.d = -5.0;
    f3.d = 0.1;
    f4.d = 2.0;
    f5.i = DOUBLE_QUIET_NAN_BITS;
    f6.d = 7.0;
    f7.d = 7.0;
    f8.i = DOUBLE_QUIET_NAN_BITS;
    f9.d = 0.0;
    f10.d = -0.0;
    f11.d = -0.0;
    f12.d = 0.0;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f2.i;
    expected.m128i_u64[1] = f3.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minpd failed");
    }

    d1.m128i_u64[0] = f5.i;
    d1.m128i_u64[1] = f6.i;
    d2.m128i_u64[0] = f7.i;
    d2.m128i_u64[1] = f8.i;
    expected.m128i_u64[0] = f6.i;
    expected.m128i_u64[1] = f8.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minpd failed");
    }
    d1.m128i_u64[0] = f9.i;
    d1.m128i_u64[1] = f10.i;
    d2.m128i_u64[0] = f11.i;
    d2.m128i_u64[1] = f12.i;
    expected.m128i_u64[0] = f10.i;
    expected.m128i_u64[1] = f12.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x5d, f1.i, f3.i, f2.i, f4.i, f2.i, f3.i);
    testSse128(0, 0x66, 0x5d, f5.i, f6.i, f7.i, f8.i, f6.i, f8.i);
    testSse128(0, 0x66, 0x5d, f9.i, f10.i, f11.i, f12.i, f10.i, f12.i);
}

void testSse2Minsd35d() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble fNan;
    TestDouble fNegZero;
    TestDouble fZero;

    f1.d = 4.0;
    f2.d = -5.0;
    f3.d = 5.0;
    f4.d = 2.0;
    fNan.i = DOUBLE_QUIET_NAN_BITS;
    fZero.d = 0.0;
    fNegZero.d = -0.0;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f2.i;
    expected.m128i_u64[1] = f3.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }

    d1.m128i_u64[0] = fNan.i;
    d1.m128i_u64[1] = f1.i;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = fNan.i;
    expected.m128i_u64[0] = f2.i;
    expected.m128i_u64[1] = f1.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }
    d1.m128i_u64[0] = f2.i;
    d1.m128i_u64[1] = f1.i;
    d2.m128i_u64[0] = fNan.i;
    d2.m128i_u64[1] = f3.i;
    expected.m128i_u64[0] = fNan.i;
    expected.m128i_u64[1] = f1.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }
    d1.m128i_u64[0] = f2.i;
    d1.m128i_u64[1] = f1.i;
    d2.m128i_u64[0] = fNan.i;
    d2.m128i_u64[1] = f2.i;
    expected.m128i_u64[0] = fNan.i;
    expected.m128i_u64[1] = f1.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }
    d1.m128i_u64[0] = fZero.i;
    d1.m128i_u64[1] = f1.i;
    d2.m128i_u64[0] = fNegZero.i;
    d2.m128i_u64[1] = f2.i;
    expected.m128i_u64[0] = fNegZero.i;
    expected.m128i_u64[1] = f1.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }
    d1.m128i_u64[0] = fNegZero.i;
    d1.m128i_u64[1] = f1.i;
    d2.m128i_u64[0] = fZero.i;
    d2.m128i_u64[1] = f2.i;
    expected.m128i_u64[0] = fZero.i;
    expected.m128i_u64[1] = f1.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x5d, f1.i, f3.i, f2.i, f4.i, f2.i, f3.i);
    testSse128(0, 0xf2, 0x5d, fNan.i, f1.i, f2.i, fNan.i, f2.i, f1.i);
    testSse128(0, 0xf2, 0x5d, f2.i, f1.i, fNan.i, f3.i, fNan.i, f1.i);
    testSse128(0, 0xf2, 0x5d, fZero.i, f1.i, fNegZero.i, f3.i, fNegZero.i, f1.i);
    testSse128(0, 0xf2, 0x5d, fNegZero.i, f1.i, fZero.i, f3.i, fZero.i, f1.i);
}

void testSse2Divpd15e() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;
    TestDouble f6;

    f1.d = 4.0;
    f2.d = 8.0;
    f3.d = -0.1;
    f4.d = 2.0;
    f5.d = 0.5;
    f6.d = -0.05;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f6.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        divpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Divpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x5e, f1.i, f3.i, f2.i, f4.i, f5.i, f6.i);
}

void testSse2Divsd35e() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;
    TestDouble f5;

    f1.d = 4.0;
    f2.d = 8.0;
    f3.d = -0.1;
    f4.d = 2.0;
    f5.d = 0.5;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f5.i;
    expected.m128i_u64[1] = f3.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        divsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Divpd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x5e, f1.i, f3.i, f2.i, f4.i, f5.i, f3.i);
}

void testSse2Maxpd15f() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;

    f1.d = 4.0;
    f2.d = -5.0;
    f3.d = 0.1;
    f4.d = 2.0;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f1.i;
    expected.m128i_u64[1] = f4.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        maxpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("maxpd failed");
    }
#endif 
    testSse128(0, 0x66, 0x5f, f1.i, f3.i, f2.i, f4.i, f1.i, f4.i);
}

void testSse2Maxsd35f() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;

    f1.d = -6.0;
    f2.d = -5.0;
    f3.d = 0.1;
    f4.d = 2.0;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = f1.i;
    d1.m128i_u64[1] = f3.i;
    __m128i d2;
    d2.m128i_u64[0] = f2.i;
    d2.m128i_u64[1] = f4.i;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = f2.i;
    expected.m128i_u64[1] = f3.i;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        maxsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("maxsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x5f, f1.i, f3.i, f2.i, f4.i, f2.i, f3.i);
}

void testSse2Punpcklbw160() {
    #if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1122334455667788;
    d1.m128i_u64[1] = 0xffffffffffffffff;
    __m128i d2;
    d2.m128i_u64[0] = 0x99aabbccddeeff00;
    d2.m128i_u64[1] = 0xffffffffffffffff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xdd55ee66ff770088;
    expected.m128i_u64[1] = 0x9911aa22bb33cc44;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpcklbw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpcklbw failed");
    }
#endif 
    testSse128(0, 0x66, 0x60, 0x1122334455667788, 0xffffffffffffffff, 0x99aabbccddeeff00, 0xffffffffffffffff, 0xdd55ee66ff770088, 0x9911aa22bb33cc44);
}

void testSse2xPunpcklwd161() {
    #if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1122334455667788;
    d1.m128i_u64[1] = 0xffffffffffffffff;
    __m128i d2;
    d2.m128i_u64[0] = 0x99aabbccddeeff00;
    d2.m128i_u64[1] = 0xffffffffffffffff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xddee5566ff007788;
    expected.m128i_u64[1] = 0x99aa1122bbcc3344;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpcklwd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpcklbw failed");
    }
#endif 
    testSse128(0, 0x66, 0x61, 0x1122334455667788, 0xffffffffffffffff, 0x99aabbccddeeff00, 0xffffffffffffffff, 0xddee5566ff007788, 0x99aa1122bbcc3344);
}

void testSse2Punpckldq162() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1122334455667788;
    d1.m128i_u64[1] = 0xffffffffffffffff;
    __m128i d2;
    d2.m128i_u64[0] = 0x99aabbccddeeff00;
    d2.m128i_u64[1] = 0xffffffffffffffff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xddeeff0055667788;
    expected.m128i_u64[1] = 0x99aabbcc11223344;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpckldq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpcklbw failed");
    }
#endif 
    testSse128(0, 0x66, 0x62, 0x1122334455667788, 0xffffffffffffffff, 0x99aabbccddeeff00, 0xffffffffffffffff, 0xddeeff0055667788, 0x99aabbcc11223344);
}

void testSse2Packsswb163() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_i16[0] = 0;
    d1.m128i_i16[1] = 0x0123;
    d1.m128i_i16[2] = 0x0080;
    d1.m128i_i16[3] = 0x007f;
    d1.m128i_i16[4] = -50;
    d1.m128i_i16[5] = -128;
    d1.m128i_i16[6] = -127;
    d1.m128i_i16[7] = -2000;

    __m128i d2;
    d2.m128i_i16[0] = 5;
    d2.m128i_i16[1] = -1;
    d2.m128i_i16[2] = 0x7fff;
    d2.m128i_u16[3] = 0xabcd;
    d2.m128i_i16[4] = -150;
    d2.m128i_i16[5] = -750;
    d2.m128i_i16[6] = -5;
    d2.m128i_i16[7] = 1;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x808180ce7f7f7f00;
    expected.m128i_u64[1] = 0x01fb8080807fff05;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        packsswb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("packsswb failed");
    }
#endif 
    testSse128(0, 0x66, 0x63, 0x007f008001230000, 0xf830ff81ff80ffce, 0xabcd7fffffff0005, 0x0001fffbfd12ff6a, 0x808180ce7f7f7f00, 0x01fb8080807fff05);
}

void testSse2Pcmpgtb164() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1122334455667788;
    d1.m128i_u64[1] = 0x99aabbccddeeff00;
    __m128i d2;
    d2.m128i_u64[0] = 0x122232ff807f0011;
    d2.m128i_u64[1] = 0x98abbb0080110000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0000ffffff00ff00;
    expected.m128i_u64[1] = 0xff000000ff000000;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pcmpgtb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pcmpgtb failed");
    }
#endif 
    testSse128(0, 0x66, 0x64, 0x1122334455667788, 0x99aabbccddeeff00, 0x122232ff807f0011, 0x98abbb0080110000, 0x0000ffffff00ff00, 0xff000000ff000000);
}

void testSse2Pcmpgtw165() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1122334455667788;
    d1.m128i_u64[1] = 0x99aabbccddeeff00;
    __m128i d2;
    d2.m128i_u64[0] = 0x112133445567ffff;
    d2.m128i_u64[1] = 0x99abbbccdded0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xffff00000000ffff;
    expected.m128i_u64[1] = 0x00000000ffff0000;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pcmpgtw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pcmpgtw failed");
    }
#endif 
    testSse128(0, 0x66, 0x65, 0x1122334455667788, 0x99aabbccddeeff00, 0x112133445567ffff, 0x99abbbccdded0000, 0xffff00000000ffff, 0x00000000ffff0000);
}

void testSse2Pcmpgtd166() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1122334455667788;
    d1.m128i_u64[1] = 0x99aabbccddeeff00;
    __m128i d2;
    d2.m128i_u64[0] = 0x1122334355667789;
    d2.m128i_u64[1] = 0x99aabbccddeefeff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xffffffff00000000;
    expected.m128i_u64[1] = 0x00000000ffffffff;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pcmpgtd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pcmpgtd failed");
    }
#endif 
    testSse128(0, 0x66, 0x66, 0x1122334455667788, 0x99aabbccddeeff00, 0x1122334355667789, 0x99aabbccddeefeff, 0xffffffff00000000, 0x00000000ffffffff);
}

void testSse2Packuswb167() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_i16[0] = 0;
    d1.m128i_i16[1] = 0x0123;
    d1.m128i_i16[2] = 0x0080;
    d1.m128i_i16[3] = 0x007f;
    d1.m128i_i16[4] = -50;
    d1.m128i_i16[5] = -128;
    d1.m128i_i16[6] = -127;
    d1.m128i_i16[7] = -2000;

    __m128i d2;
    d2.m128i_i16[0] = 5;
    d2.m128i_i16[1] = -1;
    d2.m128i_i16[2] = 0x7fff;
    d2.m128i_u16[3] = 0xabcd;
    d2.m128i_i16[4] = -150;
    d2.m128i_i16[5] = -750;
    d2.m128i_i16[6] = -5;
    d2.m128i_i16[7] = 1;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x000000007f80ff00;
    expected.m128i_u64[1] = 0x0100000000ff0005;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        packuswb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("packuswb failed");
    }
#endif 
    testSse128(0, 0x66, 0x67, 0x007f008001230000, 0xf830ff81ff80ffce, 0xabcd7fffffff0005, 0x0001fffbfd12ff6a, 0x000000007f80ff00, 0x0100000000ff0005);
}

void testSse2Punpckhbw168() {
    #if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[1] = 0x1122334455667788;
    d1.m128i_u64[0] = 0xffffffffffffffff;
    __m128i d2;
    d2.m128i_u64[1] = 0x99aabbccddeeff00;
    d2.m128i_u64[0] = 0xffffffffffffffff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xdd55ee66ff770088;
    expected.m128i_u64[1] = 0x9911aa22bb33cc44;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpckhbw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpckhbw failed");
    }
#endif 
    testSse128(0, 0x66, 0x68, 0xffffffffffffffff, 0x1122334455667788, 0xffffffffffffffff, 0x99aabbccddeeff00, 0xdd55ee66ff770088, 0x9911aa22bb33cc44);
}

void testSse2Punpckhwd169() {
    #if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[1] = 0x1122334455667788;
    d1.m128i_u64[0] = 0xffffffffffffffff;
    __m128i d2;
    d2.m128i_u64[1] = 0x99aabbccddeeff00;
    d2.m128i_u64[0] = 0xffffffffffffffff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xddee5566ff007788;
    expected.m128i_u64[1] = 0x99aa1122bbcc3344;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpckhwd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpckhwd failed");
    }
#endif 
    testSse128(0, 0x66, 0x69, 0xffffffffffffffff, 0x1122334455667788, 0xffffffffffffffff, 0x99aabbccddeeff00, 0xddee5566ff007788, 0x99aa1122bbcc3344);
}

void testSse2Punpckhdq16a() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[1] = 0x1122334455667788;
    d1.m128i_u64[0] = 0xffffffffffffffff;
    __m128i d2;
    d2.m128i_u64[1] = 0x99aabbccddeeff00;
    d2.m128i_u64[0] = 0xffffffffffffffff;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xddeeff0055667788;
    expected.m128i_u64[1] = 0x99aabbcc11223344;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpckhdq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpckhdq failed");
    }
#endif 
    testSse128(0, 0x66, 0x6a, 0xffffffffffffffff, 0x1122334455667788, 0xffffffffffffffff, 0x99aabbccddeeff00, 0xddeeff0055667788, 0x99aabbcc11223344);
}

void testSse2Packssdw16b() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_i32[0] = 0;
    d1.m128i_i32[1] = 0x12345678;
    d1.m128i_i32[2] = 0x0080;
    d1.m128i_i32[3] = 0x007f;

    __m128i d2;
    d2.m128i_i32[0] = 5;
    d2.m128i_i32[1] = -1;
    d2.m128i_i32[2] = 0x7fff;
    d2.m128i_i32[3] = 0xabcd;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x007f00807fff0000;
    expected.m128i_u64[1] = 0x7fff7fffffff0005;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        packssdw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("packssdw failed");
    }
#endif 
    testSse128(0, 0x66, 0x6b, 0x1234567800000000, 0x0000007f00000080, 0xffffffff00000005, 0x0000abcd00007fff, 0x007f00807fff0000, 0x7fff7fffffff0005);
}

void testSse2Punpcklqdq16c() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x1111111122222222;
    expected.m128i_u64[1] = 0x5555555566666666;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpcklqdq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpcklqdq failed");
    }
#endif 
    testSse128(0, 0x66, 0x6c, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x1111111122222222, 0x5555555566666666);
}

void testSse2Punpckhqdq16d() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x3333333344444444;
    expected.m128i_u64[1] = 0x7777777788888888;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        punpckhqdq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("punpckhqdq failed");
    }
#endif 
    testSse128(0, 0x66, 0x6d, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x3333333344444444, 0x7777777788888888);
}

void testSse2Movd16e() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0000000012345678;
    expected.m128i_u64[1] = 0;

    __asm {
        movups xmm0, d1
        mov ecx, 0x12345678
        movd xmm0, ecx
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movd failed");
    }
#endif 
    testSseReg32(0, 0x66, 0x6e, 0x1111111122222222, 0x3333333344444444, 0x12345678, 0x0000000012345678, 0);
}

void testSse2Movdqa16f() {
    testSse128(0, 0x66, 0x6f, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x5555555566666666, 0x7777777788888888);
}

void testSse2Movdqu36f() {
    testSse128(0, 0xf3, 0x6f, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x5555555566666666, 0x7777777788888888);
}

void testSse2Movd17e() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    U32 result;
    U32 expected = 0x22222222;

    __asm {
        movups xmm0, d1
        mov ecx, 0x12345678
        movd ecx, xmm0
        mov result, ecx
        emms
    }
    if (result!=expected) {
        failed("movd failed");
    }
#endif 
    testSseE32r(0, 0x66, 0x7e, 0x12345678, 0x1111111122222222, 0x3333333344444444, 0x22222222);
}

void testSse2Movq37e() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x5555555566666666;
    expected.m128i_u64[1] = 0;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        movq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movq failed");
    }
    U64 result64 = 0x5555555566666666;

    __asm {
        movups xmm0, d1
        movq xmm0, result64
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movq failed");
    }
#endif 
    testSse128E64(0, 0xf3, 0x7e, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x5555555566666666, 0);
}

void testSse2Pshufd170() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x7777777788888888;
    expected.m128i_u64[1] = 0x6666666655555555;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pshufd xmm1, xmm0, 0x1E
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pshufd failed");
    }
#endif 
    testSse128imm(0, 0x66, 0x70, 0x1E, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x7777777788888888, 0x6666666655555555);
}

void testSse2Pshuflw370() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i d2;
    d2.m128i_u64[0] = 0x5555666677778888;
    d2.m128i_u64[1] = 0xfedcba0987654321;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x8888777755556666;
    expected.m128i_u64[1] = 0xfedcba0987654321;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pshuflw xmm1, xmm0, 0x1E
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pshuflw failed");
    }
#endif 
    testSse128imm(0, 0xf2, 0x70, 0x1E, 0x1111222233334444, 0x1234567890abcdef, 0x5555666677778888, 0xfedcba0987654321, 0x8888777755556666, 0xfedcba0987654321);
}

void testSse2Pshufhw370() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i d2;
    d2.m128i_u64[0] = 0x5555666677778888;
    d2.m128i_u64[1] = 0xfedcba0987654321;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x5555666677778888;
    expected.m128i_u64[1] = 0x43218765fedcba09;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pshufhw xmm1, xmm0, 0x1E
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pshufhw failed");
    }
#endif 
    testSse128imm(0, 0xf3, 0x70, 0x1E, 0x1111222233334444, 0x1234567890abcdef, 0x5555666677778888, 0xfedcba0987654321, 0x5555666677778888, 0x43218765fedcba09);
}

void testSse2Psrlw171() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0111022203330444;
    expected.m128i_u64[1] = 0x01230567090a0cde;

    __asm {
        movups xmm1, d1
        psrlw xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrlw failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x71, 2, 4, 0x1111222233334444, 0x1234567890abcdef, 0x0111022203330444, 0x01230567090a0cde);
}

void testSse2Psraw171() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0111022203330444;
    expected.m128i_u64[1] = 0x01230567f90afcde;

    __asm {
        movups xmm1, d1
        psraw xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psraw failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x71, 4, 4, 0x1111222233334444, 0x1234567890abcdef, 0x0111022203330444, 0x01230567f90afcde);
}

void testSse2Psllw171() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x1110222033304440;
    expected.m128i_u64[1] = 0x234067800ab0def0;

    __asm {
        movups xmm1, d1
        psllw xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psllw failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x71, 6, 4, 0x1111222233334444, 0x1234567890abcdef, 0x1110222033304440, 0x234067800ab0def0);
}

void testSse2Psrld172() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0111122203333444;
    expected.m128i_u64[1] = 0x01234567090abcde;

    __asm {
        movups xmm1, d1
        psrld xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrld failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x72, 2, 4, 0x1111222233334444, 0x1234567890abcdef, 0x0111122203333444, 0x01234567090abcde);
}

void testSse2Psrad172() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0111122203333444;
    expected.m128i_u64[1] = 0x01234567f90abcde;

    __asm {
        movups xmm1, d1
        psrad xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrad failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x72, 4, 4, 0x1111222233334444, 0x1234567890abcdef, 0x0111122203333444, 0x01234567f90abcde);
}

void testSse2Pslld172() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x1234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x1112222033344440;
    expected.m128i_u64[1] = 0x234567800abcdef0;

    __asm {
        movups xmm1, d1
        pslld xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pslld failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x72, 6, 4, 0x1111222233334444, 0x1234567890abcdef, 0x1112222033344440, 0x234567800abcdef0);
}

void testSse2Psrlq173() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x8234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0111122223333444;
    expected.m128i_u64[1] = 0x08234567890abcde;

    __asm {
        movups xmm1, d1
        psrlq xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrlq failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x73, 2, 4, 0x1111222233334444, 0x8234567890abcdef, 0x0111122223333444, 0x08234567890abcde);
}

void testSse2Psrldq173() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x8234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x90abcdef11112222;
    expected.m128i_u64[1] = 0x0000000082345678;

    __asm {
        movups xmm1, d1
        psrldq xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrldq failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x73, 3, 4, 0x1111222233334444, 0x8234567890abcdef, 0x90abcdef11112222, 0x0000000082345678);
}

void testSse2Psllq173() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x8234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x1112222333344440;
    expected.m128i_u64[1] = 0x234567890abcdef0;

    __asm {
        movups xmm1, d1
        psllq xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrldq failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x73, 6, 4, 0x1111222233334444, 0x8234567890abcdef, 0x1112222333344440, 0x234567890abcdef0);
}

void testSse2Pslldq173() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x8234567890abcdef;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x3333444400000000;
    expected.m128i_u64[1] = 0x90abcdef11112222;

    __asm {
        movups xmm1, d1
        pslldq xmm1, 4
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pslldq failed");
    }
#endif 
    testSse128SubImm(0, 0x66, 0x73, 7, 4, 0x1111222233334444, 0x8234567890abcdef, 0x3333444400000000, 0x90abcdef11112222);
}

void testSse2Movdqa17f() {
    testSse128r(0, 0x66, 0x7f, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x5555555566666666, 0x7777777788888888);
}

void testSse2Movdqu37f() {
    testSse128r(0, 0xf3, 0x7f, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x5555555566666666, 0x7777777788888888);
}

void testSse2Pcmpeqb174() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x1100001100222200;
    d2.m128i_u64[1] = 0x0033000000444444;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xff0000ff00ffff00;
    expected.m128i_u64[1] = 0x00ff000000ffffff;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pcmpeqb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pcmpeqb failed");
    }
#endif 
    testSse128(0, 0x66, 0x74, 0x1111111122222222, 0x3333333344444444, 0x1100001100222200, 0x0033000000444444, 0xff0000ff00ffff00, 0x00ff000000ffffff);
}

void testSse2Pcmpeqw175() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x1111001100222222;
    d2.m128i_u64[1] = 0x0033000000444444;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xffff00000000ffff;
    expected.m128i_u64[1] = 0x000000000000ffff;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pcmpeqw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pcmpeqw failed");
    }
#endif 
    testSse128(0, 0x66, 0x75, 0x1111111122222222, 0x3333333344444444, 0x1111001100222222, 0x0033000000444444, 0xffff00000000ffff, 0x000000000000ffff);
}

void testSse2Pcmpeqd176() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;

    __m128i d2;
    d2.m128i_u64[0] = 0x1111111102222222;
    d2.m128i_u64[1] = 0x3333033344444444;

    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xffffffff00000000;
    expected.m128i_u64[1] = 0x00000000ffffffff;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pcmpeqd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pcmpeqd failed");
    }
#endif 
    testSse128(0, 0x66, 0x76, 0x1111111122222222, 0x3333333344444444, 0x1111111102222222, 0x3333033344444444, 0xffffffff00000000, 0x00000000ffffffff);
}

void testSse2Cmppd1c2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    __m128d d1;
    d1.m128d_f64[0] = 4.0;
    d1.m128d_f64[1] = 5.0;
    __m128d d2;
    d2.m128d_f64[0] = -8.0;
    d2.m128d_f64[1] = 5.0;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0;
    expected.m128i_u64[1] = -1;

     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpeqpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpeqpd failed");
    }

    expected.m128i_u64[0] = 0;
    expected.m128i_u64[1] = 0;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpltpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpltpd failed");
    }

    expected.m128i_u64[0] = 0;
    expected.m128i_u64[1] = -1;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmplepd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmplepd failed");
    }

    expected.m128i_u64[0] = -1;
    expected.m128i_u64[1] = 0;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpneqpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpneqpd failed");
    }

    expected.m128i_u64[0] = -1;
    expected.m128i_u64[1] = -1;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnltpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpnltpd failed");
    }

    expected.m128i_u64[0] = -1;
    expected.m128i_u64[1] = 0;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnlepd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpnlepd failed");
    }

    expected.m128i_u64[0] = -1;
    expected.m128i_u64[1] = 0;
    TestDouble d;
    d.i = 0x7ff8000000000000;
    d1.m128d_f64[0] = d.d;

     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpunordpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpunordpd failed");
    }

    expected.m128i_u64[0] = 0;
    expected.m128i_u64[1] = -1;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpordpd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpordpd failed");
    }
#endif
    // cmpeqpd
    testSse128imm(0, 0x66, 0xc2, 0, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0xffffffffffffffff);
    // cmpltpd
    testSse128imm(0, 0x66, 0xc2, 1, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0);
    // cmplepd
    testSse128imm(0, 0x66, 0xc2, 2, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0xffffffffffffffff);
    // cmpunordpd
    testSse128imm(0, 0x66, 0xc2, 3, 0x7ff8000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0);
    // cmpneqpd
    testSse128imm(0, 0x66, 0xc2, 4, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0);
    // cmpnltpd
    testSse128imm(0, 0x66, 0xc2, 5, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0xffffffffffffffff);
    // cmpnlepd
    testSse128imm(0, 0x66, 0xc2, 6, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0);
    // cmpordpd
    testSse128imm(0, 0x66, 0xc2, 7, 0x7ff8000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0xffffffffffffffff);
}

// :TODO: should test more cases
void testSse2Cmpsd3c2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    __m128d d1;
    d1.m128d_f64[0] = 4.0;
    d1.m128d_f64[1] = 5.0;
    __m128d d2;
    d2.m128d_f64[0] = -8.0;
    d2.m128d_f64[1] = 5.0;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0;
    expected.m128i_u64[1] = 0x4014000000000000; // 5.0

     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpeqsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpeqsd failed");
    }

    expected.m128i_u64[0] = 0;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpltsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpltsd failed");
    }

    expected.m128i_u64[0] = 0;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmplesd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmplesd failed");
    }

    expected.m128i_u64[0] = -1;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpneqsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpneqsd failed");
    }

    expected.m128i_u64[0] = -1;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnltsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpnltsd failed");
    }

    expected.m128i_u64[0] = -1;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnlesd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpnlesd failed");
    }

    expected.m128i_u64[0] = -1;
    TestDouble d;
    d.i = 0x7ff8000000000000;
    d1.m128d_f64[0] = d.d;

     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpunordsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpunordsd failed");
    }

    expected.m128i_u64[0] = 0;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpordsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cmpordsd failed");
    }
#endif
    // cmpeqsd
    testSse128imm(0, 0xf2, 0xc2, 0, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0x4014000000000000);
    // cmpltsd
    testSse128imm(0, 0xf2, 0xc2, 1, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0x4014000000000000);
    // cmplesd
    testSse128imm(0, 0xf2, 0xc2, 2, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0x4014000000000000);
    // cmpunordsd
    testSse128imm(0, 0xf2, 0xc2, 3, 0x7ff8000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0x4014000000000000);
    // cmpneqsd
    testSse128imm(0, 0xf2, 0xc2, 4, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0x4014000000000000);
    // cmpnltsd
    testSse128imm(0, 0xf2, 0xc2, 5, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0x4014000000000000);
    // cmpnlesd
    testSse128imm(0, 0xf2, 0xc2, 6, 0x4010000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0xffffffffffffffff, 0x4014000000000000);
    // cmpordsd
    testSse128imm(0, 0xf2, 0xc2, 7, 0x7ff8000000000000, 0x4014000000000000, 0xc020000000000000, 0x4014000000000000, 0, 0x4014000000000000);
}

void testSse2Movnti3c3() {   
    for (U8 m=0;m<8;m++) {
        newInstruction(0);
        cpu->reg[m].u32 = 0x12348765;
        memory->writed(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET, 0xcdcdcdcd);
        pushCode8(0x0f);
        pushCode8(0xc3);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(SSE_MEM_VALUE_TMP_OFFSET);
        runTestCPU();
        if (memory->readd(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET)!=0x12348765) {
            failed("movnti failed");
        }
    }    
}

void testPinsrw1c4() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    __m128i result;
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x5555666677778888;
    U32 reg = 0x11229900;
    __m128i expected;
    expected.m128i_u64[0] = 0x1111222233334444;
    expected.m128i_u64[1] = 0x5555666699008888;

    __asm {
        __asm movups xmm0, d1
        __asm mov ecx, reg
        __asm pinsrw xmm0, ecx, 5
        __asm movups result, xmm0
        __asm emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("failed");
    }
#endif
    testSse16Eimm8(0, 0x66, 0xc4, 0x1111222233334444, 0x5555666677778888, 0x55559900, 0x1111222233334444, 0x5555666699008888, 5);
}

void testSse2Shufpd1c6() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x1111111122222222;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x1111111122222222;
    expected.m128i_u64[1] = 0x7777777788888888;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        shufpd xmm1, xmm0, 0x1E
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Shufpd failed");
    }
#endif 
    testSse128imm(0, 0x66, 0xc6, 0x1E, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x1111111122222222, 0x7777777788888888);
}

void testSse2Psrlw1d1() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 2;
    d2.m128i_u64[1] = 0x7777777788888888; // ignored
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x00100f000f230567;
    expected.m128i_u64[1] = 0x0ccc0ccc11111111;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psrlw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrlw failed");
    }
#endif 
    testSse128(0, 0x66, 0xd1, 0x00403c013c8d159e, 0x3333333344444444, 2, 0x7777777788888888, 0x00100f000f230567, 0x0ccc0ccc11111111);
}

void testSse2Psrld1d2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 2;
    d2.m128i_u64[1] = 0x7777777788888888; // ignored
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x00100f000f234567;
    expected.m128i_u64[1] = 0x0ccccccc11111111;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psrld xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrld failed");
    }
#endif 
    testSse128(0, 0x66, 0xd2, 0x00403c013c8d159e, 0x3333333344444444, 2, 0x7777777788888888, 0x00100f000f234567, 0x0ccccccc11111111);
}

void testSse2Psrlq1d3() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 2;
    d2.m128i_u64[1] = 0x7777777788888888; // ignored
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x00100f004f234567;
    expected.m128i_u64[1] = 0x0cccccccd1111111;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psrlq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrlq failed");
    }
#endif 
    testSse128(0, 0x66, 0xd3, 0x00403c013c8d159e, 0x3333333344444444, 2, 0x7777777788888888, 0x00100f004f234567, 0x0cccccccd1111111);
}

void testSse2Paddq1d4() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef; // good number to test for overflows
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x4789abce57123455;
    expected.m128i_u64[1] = 0xaaaaaaaacccccccc;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrlq failed");
    }
#endif 
    testSse128(0, 0x66, 0xd4, 0xf2345678f0abcdef, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x4789abce57123455, 0xaaaaaaaacccccccc);
}

void testSse2Pmullw1d5() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef; // good number to test for overflows
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x7777777788888888;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xaf448dd80622473a;
    expected.m128i_u64[1] = 0x81b581b564206420;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmullw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmullw failed");
    }
#endif 
    testSse128(0, 0x66, 0xd5, 0xf2345678f0abcdef, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0xaf448dd80622473a, 0x81b581b564206420);
}

void testSse2Movq1d6() {
    testSse128E64r(0, 0x66, 0xd6, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x7777777788888888, 0x5555555566666666, 0, 0x5555555566666666, 0x3333333344444444);
}

void testSse2Movdq2q3d6() {
    testSseMmx64r(0, 0xf2, 0xd6, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x3333333344444444, 0xFFFFFFFFFFFFFFFF);
}

void testSse2Movq2dq3d6() {
    testSseMmx64(0, 0xf3, 0xd6, 0x1111111122222222, 0x3333333344444444, 0x5555555566666666, 0x5555555566666666, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
}

void testSse2Pmovmskb1d7() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi32(0x11803344, 0xF0559900, 0x77C01177, 0xA0801122);
    U32 result;
    U32 reg = 0xdeadbeef;
    U32 expected = 0x0000c4a4;

    __asm {
        movups xmm1, d1
        mov ecx, reg
        pmovmskb ecx, xmm0
        mov result, ecx
        emms
    }
    if (result!=expected) {
        failed("Pmovmskb failed");
    }
#endif
    testSseReg32r(0, 0x66, 0xd7, 0xdeadbeef, 0xF055990011803344, 0xA080112277C01177, 0x0000c4a4, 0xffffffff);
}

void testSse2Psubusb1d8() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x9d0001238a456789;
    expected.m128i_u64[1] = 0x2211000000000000;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubusb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubusb failed");
    }
#endif 
    testSse128(0, 0x66, 0xd8, 0xf2345678f0abcdef, 0x3333333344444444, 0x5555555566666666, 0x1122334455667788, 0x9d0001238a456789, 0x2211000000000000);
}

void testSse2Psubusw1d9() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x9cdf01238a456789;
    expected.m128i_u64[1] = 0x2211000000000000;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubusw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubusw failed");
    }
#endif 
    testSse128(0, 0x66, 0xd9, 0xf2345678f0abcdef, 0x3333333344444444, 0x5555555566666666, 0x1122334455667788, 0x9cdf01238a456789, 0x2211000000000000);
}

void testSse2Pminub1da() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55555555666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x55345555666666ef;
    expected.m128i_u64[1] = 0x1122334455654321;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pminub xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pminub failed");
    }
#endif 
    testSse128(0, 0x66, 0xda, 0xf2345678f0abcdef, 0x1234567887654321, 0x55555555666666ff, 0x1122334455667788, 0x55345555666666ef, 0x1122334455654321);
}

void testSse2Pand1db() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x50140150602244ef;
    expected.m128i_u64[1] = 0x1020124005644300;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pand xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pand failed");
    }
#endif 
    testSse128(0, 0x66, 0xdb, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0x50140150602244ef, 0x1020124005644300);
}

void testSse2Paddusb1dc() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xff8902cdffffffff;
    expected.m128i_u64[1] = 0x235689bcdccbbaa9;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddusb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddusb failed");
    }
#endif 
    testSse128(0, 0x66, 0xdc, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0xff8902cdffffffff, 0x235689bcdccbbaa9);
}

void testSse2Paddusw1dd() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xffff02cdffffffff;
    expected.m128i_u64[1] = 0x235689bcdccbbaa9;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddusw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddusw failed");
    }
#endif 
    testSse128(0, 0x66, 0xdd, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0xffff02cdffffffff, 0x235689bcdccbbaa9);
}

void testSse2Pmaxub1de() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xf2550178f0abcdff;
    expected.m128i_u64[1] = 0x1234567887667788;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmaxub xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmaxub failed");
    }
#endif 
    testSse128(0, 0x66, 0xde, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0xf2550178f0abcdff, 0x1234567887667788);
}

void testSse2Pandn1df() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0541000506442210;
    expected.m128i_u64[1] = 0x0102210450023488;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pandn xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pandn failed");
    }
#endif 
    testSse128(0, 0x66, 0xdf, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0x0541000506442210, 0x0102210450023488);
}

void testSse2Pavgb1e0() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xa4450167ab899af7;
    expected.m128i_u64[1] = 0x122b455e6e665d55;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pavgb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pavgb failed");
    }
#endif 
    testSse128(0, 0x66, 0xe0, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0xa4450167ab899af7, 0x122b455e6e665d55);
}

void testSse2Psraw1e1() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 2;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xfc8d005efc2af37b;
    expected.m128i_u64[1] = 0x048d159ee1d910c8;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psraw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psraw failed");
    }
#endif 
    testSse128(0, 0x66, 0xe1, 0xf2340178f0abcdef, 0x1234567887654321, 2, 0x1122334455667788, 0xfc8d005efc2af37b, 0x048d159ee1d910c8);
}

void testSse2Psrad1e2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 17;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xfffff91afffff855;
    expected.m128i_u64[1] = 0x0000091affffc3b2;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psrad xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psrad failed");
    }
#endif 
    testSse128(0, 0x66, 0xe2, 0xf2340178f0abcdef, 0x1234567887654321, 17, 0x1122334455667788, 0xfffff91afffff855, 0x0000091affffc3b2);
}

void testSse2Pavgw1e3() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xa3c50167ab899a77;
    expected.m128i_u64[1] = 0x11ab44de6e665d55;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pavgw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pavgw failed");
    }
#endif 
    testSse128(0, 0x66, 0xe3, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0xa3c50167ab899a77, 0x11ab44de6e665d55);
}

void testSse2Pmulhuw1e4() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x50bb0001604452da;
    expected.m128i_u64[1] = 0x013711502d2a1f58;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmulhuw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmulhuw failed");
    }
#endif 
    testSse128(0, 0x66, 0xe4, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0x50bb0001604452da, 0x013711502d2a1f58);
}

void testSse2Pmulhw1e5() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2340178f0abcdef;
    d1.m128i_u64[1] = 0x1234567887654321;
    __m128i d2;
    d2.m128i_u64[0] = 0x55550155666666ff;;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xfb660001f9deebdb;
    expected.m128i_u64[1] = 0x01371150d7c41f58;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmulhw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmulhw failed");
    }
#endif 
    testSse128(0, 0x66, 0xe5, 0xf2340178f0abcdef, 0x1234567887654321, 0x55550155666666ff, 0x1122334455667788, 0xfb660001f9deebdb, 0x01371150d7c41f58);
}

void testSse2Cvttpd2dq1e6() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;

    f1.d = 12345678900.0;
    f2.d = -5000.6;

    f3.d = -12345678900.0;
    f4.d = 5000.6;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x5555666677778888;
    __m128i d2;
    d2.m128i_u64[0] = f1.i;
    d2.m128i_u64[1] = f2.i;
    __m128i expected;
    expected.m128i_u64[0] = 0xffffec7880000000;
    expected.m128i_u64[1] = 0;
    __m128i result;

    __asm {
        movups xmm0, d1
        movups xmm1, d2
        cvttpd2dq xmm0, xmm1
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvttpd2dq failed");
    }

    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x5555666677778888;
    d2.m128i_u64[0] = f3.i;
    d2.m128i_u64[1] = f4.i;
    expected.m128i_u64[0] = 0x0000138880000000;
    expected.m128i_u64[1] = 0;

    __asm {
        movups xmm0, d1
        movups xmm1, d2
        cvttpd2dq xmm0, xmm1
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvttpd2dq failed");
    }
#endif 
    testSse128(0, 0x66, 0xe6, 0x1111222233334444, 0x5555666677778888, f1.i, f2.i, 0xffffec7880000000, 0);
    testSse128(0, 0x66, 0xe6, 0x1111222233334444, 0x5555666677778888, f3.i, f4.i, 0x0000138880000000, 0);
}

void testSse2Cvtpd2dq3e6() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 12345678900.0;
    f2.d = -5000;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x5555666677778888;
    __m128i d2;
    d2.m128i_u64[0] = f1.i;
    d2.m128i_u64[1] = f2.i;
    __m128i expected;
    expected.m128i_u64[0] = 0xffffec7880000000;
    expected.m128i_u64[1] = 0;
    __m128i result;

    __asm {
        movups xmm0, d1
        movups xmm1, d2
        cvtpd2dq xmm0, xmm1
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtpd2dq failed");
    }
#endif 
    testSse128(0, 0xf2, 0xe6, 0x1111222233334444, 0x5555666677778888, f1.i, f2.i, 0xffffec7880000000, 0);
}

void testSse2Cvtdq2pd3e6() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 123456789.0;
    f2.d = -5000;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u64[0] = 0x1111222233334444;
    d1.m128i_u64[1] = 0x5555666677778888;
    __m128i d2;
    d2.m128i_u64[0] = 0xffffec78075bcd15;
    d2.m128i_u64[1] = 0x1234567890abcdef;
    __m128i expected;
    expected.m128i_u64[0] = f1.i;
    expected.m128i_u64[1] = f2.i;
    __m128i result;

    __asm {
        movups xmm0, d1
        movups xmm1, d2
        cvtdq2pd xmm0, xmm1
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("cvtdq2pd failed");
    }
#endif 
    testSse128(0, 0xf3, 0xe6, 0x1111222233334444, 0x5555666677778888, 0xffffec78075bcd15, 0x1234567890abcdef, f1.i, f2.i);
}

void testSse2Movntdq1e7() {  
    testSse128r(0, 0x66, 0xe7, 0x1234567890abcdef, 0x1234567890abcdef, 0x1111222233334444, 0x5555666677778888, 0xffffffffffffffff, 0xffffffffffffffff, 0x1111222233334444, 0x5555666677778888);
}

void testSse2Psubsb1e8() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x9ddf01238a808089;
    expected.m128i_u64[1] = 0x221100efefdecd7f;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubsb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubsb failed");
    }
#endif 
    testSse128(0, 0x66, 0xe8, 0xf2345678f0abcdef, 0x3333333344444444, 0x5555555566666666, 0x1122334455667788, 0x9ddf01238a808089, 0x221100efefdecd7f);
}

void testSse2Psubsw1e9() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x9cdf01238a458000;
    expected.m128i_u64[1] = 0x2211ffefeedeccbc;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubsw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubsw failed");
    }
#endif 
    testSse128(0, 0x66, 0xe9, 0xf2345678f0abcdef, 0x3333333344444444, 0x5555555566666666, 0x1122334455667788, 0x9cdf01238a458000, 0x2211ffefeedeccbc);
}

void testSse2Pminsw1ea() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333003344004444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xf2345555f0abcdef;
    expected.m128i_u64[1] = 0x1122003344004444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pminsw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pminsw failed");
    }
#endif 
    testSse128(0, 0x66, 0xea, 0xf2345678f0abcdef, 0x3333003344004444, 0x5555555566666666, 0x1122334455667788, 0xf2345555f0abcdef, 0x1122003344004444);
}

void testSse2Por1eb() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333003344004444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xf775577df6efefef;
    expected.m128i_u64[1] = 0x33333377556677cc;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        por xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("por failed");
    }
#endif 
    testSse128(0, 0x66, 0xeb, 0xf2345678f0abcdef, 0x3333003344004444, 0x5555555566666666, 0x1122334455667788, 0xf775577df6efefef, 0x33333377556677cc);
}

void testSse2Paddsb1ec() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333003344004444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x477f7f7f56113355;
    expected.m128i_u64[1] = 0x445533777f667fcc;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddsb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddsb failed");
    }
#endif 
    testSse128(0, 0x66, 0xec, 0xf2345678f0abcdef, 0x3333003344004444, 0x5555555566666666, 0x1122334455667788, 0x477f7f7f56113355, 0x445533777f667fcc);
}

void testSse2Paddsw1ed() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333003344004444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x47897fff57113455;
    expected.m128i_u64[1] = 0x445533777fff7fff;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddsw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddsw failed");
    }
#endif 
    testSse128(0, 0x66, 0xed, 0xf2345678f0abcdef, 0x3333003344004444, 0x5555555566666666, 0x1122334455667788, 0x47897fff57113455, 0x445533777fff7fff);
}

void testSse2Pmaxsw1ee() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333003344004444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x5555567866666666;
    expected.m128i_u64[1] = 0x3333334455667788;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmaxsw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmaxsw failed");
    }
#endif 
    testSse128(0, 0x66, 0xee, 0xf2345678f0abcdef, 0x3333003344004444, 0x5555555566666666, 0x1122334455667788, 0x5555567866666666, 0x3333334455667788);
}

void testSse2Pxor1ef() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0xf2345678f0abcdef;
    d1.m128i_u64[1] = 0x3333003344004444;
    __m128i d2;
    d2.m128i_u64[0] = 0x5555555566666666;
    d2.m128i_u64[1] = 0x1122334455667788;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xa761032d96cdab89;
    expected.m128i_u64[1] = 0x22113377116633cc;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pxor xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pxor failed");
    }
#endif 
    testSse128(0, 0x66, 0xef, 0xf2345678f0abcdef, 0x3333003344004444, 0x5555555566666666, 0x1122334455667788, 0xa761032d96cdab89, 0x22113377116633cc);
}

void testSse2Psllw1f1() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 2;
    d2.m128i_u64[1] = 0x7777777788888888; // ignored
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0100f004f2345678;
    expected.m128i_u64[1] = 0xcccccccc11101110;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psllw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psllw failed");
    }
#endif 
    testSse128(0, 0x66, 0xf1, 0x00403c013c8d159e, 0x3333333344444444, 2, 0x7777777788888888, 0x0100f004f2345678, 0xcccccccc11101110);
}

void testSse2Pslld1f2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 2;
    d2.m128i_u64[1] = 0x7777777788888888; // ignored
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0100f004f2345678;
    expected.m128i_u64[1] = 0xcccccccc11111110;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pslld xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pslld failed");
    }
#endif 
    testSse128(0, 0x66, 0xf2, 0x00403c013c8d159e, 0x3333333344444444, 2, 0x7777777788888888, 0x0100f004f2345678, 0xcccccccc11111110);
}

void testSse2Psllq1f3() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 17;
    d2.m128i_u64[1] = 0x7777777788888888; // ignored
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x7802791a2b3c0000;
    expected.m128i_u64[1] = 0x6666888888880000;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psllq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psllq failed");
    }
#endif 
    testSse128(0, 0x66, 0xf3, 0x00403c013c8d159e, 0x3333333344444444, 17, 0x7777777788888888, 0x7802791a2b3c0000, 0x6666888888880000);
}

void testSse2Pmuludq1f4() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 17;
    d2.m128i_u64[1] = 0x7777777788888888;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x00000004055e6f7e;
    expected.m128i_u64[1] = 0x2468acf0eca86420;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmuludq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmuludq failed");
    }
#endif 
    testSse128(0, 0x66, 0xf4, 0x00403c013c8d159e, 0x3333333344444444, 17, 0x7777777788888888, 0x00000004055e6f7e, 0x2468acf0eca86420);
}

void testSse2Pmaddwd1f5() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 17;
    d2.m128i_u64[1] = 0x7777777788888888;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0000000000016f7e;
    expected.m128i_u64[1] = 0x2fc9036ac048c840;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        pmaddwd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("pmaddwd failed");
    }
#endif 
    testSse128(0, 0x66, 0xf5, 0x00403c013c8d159e, 0x3333333344444444, 17, 0x7777777788888888, 0x0000000000016f7e, 0x2fc9036ac048c840);
}

void testSse2Psadbw1f6() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x0000000000000334;
    expected.m128i_u64[1] = 0x000000000000017e;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psadbw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psadbw failed");
    }
#endif 
    testSse128(0, 0x66, 0xf6, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0x0000000000000334, 0x000000000000017e);
}

void testSse2Maskmovdqu1f7() {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            if (m==from) {
                continue;
            }            
            initSseTest();            
            loadSSE(m, 0, 0x1122334455667788, 0x99aabbccddeeff00);
            loadSSE(from, 1, 0x8000800080008000, 0x0080808000008080);
            EDI = SSE_MEM_VALUE_TMP_OFFSET+64;
            memory->writeq(cpu->seg[DS].address+EDI, 0x9999999999999999);
            memory->writeq(cpu->seg[DS].address+EDI+8, 0x9999999999999999);
            pushCode8(0x66);
            pushCode8(0x0f);            
            pushCode8(0xf7);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();
            U64 result1 = memory->readq(cpu->seg[DS].address+EDI);
            U64 result2 = memory->readq(cpu->seg[DS].address+EDI+8);
            if (result1!=0x1199339955997799 || result2!=0x99aabbcc9999ff00) {
                failed("maskmovq failed");
            }
        }
    } 
}

void testSse2Psubb1f8() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xd17739977c454d5e;
    expected.m128i_u64[1] = 0xbb31ba1919084444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubb failed");
    }
#endif 
    testSse128(0, 0x66, 0xf8, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0xd17739977c454d5e, 0xbb31ba1919084444);
}

void testSse2Psubw1f9() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xd07738977c454d5e;
    expected.m128i_u64[1] = 0xbb31ba1919084444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubw failed");
    }
#endif 
    testSse128(0, 0x66, 0xf9, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0xd07738977c454d5e, 0xbb31ba1919084444);
}

void testSse2Psubd1fa() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xd07738977c444d5e;
    expected.m128i_u64[1] = 0xbb30ba1919084444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubd failed");
    }
#endif 
    testSse128(0, 0x66, 0xfa, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0xd07738977c444d5e, 0xbb30ba1919084444);
}

void testSse2Psubq1fb() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0xd07738967c444d5e;
    expected.m128i_u64[1] = 0xbb30ba1919084444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        psubq xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("psubq failed");
    }
#endif 
    testSse128(0, 0x66, 0xfb, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0xd07738967c444d5e, 0xbb30ba1919084444);
}

void testSse2Paddb1fc() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x2f093f6bfcd5ddde;
    expected.m128i_u64[1] = 0xab35ac4d6f804444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddb xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddb failed");
    }
#endif 
    testSse128(0, 0x66, 0xfc, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0x2f093f6bfcd5ddde, 0xab35ac4d6f804444);
}

void testSse2Paddw1fd() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x3333333344444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x30093f6bfcd5ddde;
    expected.m128i_u64[1] = 0xab35ac4d6f804444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddw xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddw failed");
    }
#endif 
    testSse128(0, 0x66, 0xfd, 0x00403c013c8d159e, 0x3333333344444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0x30093f6bfcd5ddde, 0xab35ac4d6f804444);
}

void testSse2Paddd1fe() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1;
    d1.m128i_u64[0] = 0x00403c013c8d159e;
    d1.m128i_u64[1] = 0x33333333f4444444;
    __m128i d2;
    d2.m128i_u64[0] = 0x2fc9036ac048c840;
    d2.m128i_u64[1] = 0x7802791a2b3c0000;
    __m128i result;
    __m128i expected;
    expected.m128i_u64[0] = 0x30093f6bfcd5ddde;
    expected.m128i_u64[1] = 0xab35ac4d1f804444;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        paddd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("paddd failed");
    }
#endif 
    testSse128(0, 0x66, 0xfe, 0x00403c013c8d159e, 0x33333333f4444444, 0x2fc9036ac048c840, 0x7802791a2b3c0000, 0x30093f6bfcd5ddde, 0xab35ac4d1f804444);
}

#endif
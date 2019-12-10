#include "boxedwine.h"

#ifdef __TEST

#include "testCPU.h"
#include "testSSE.h"

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
}

void testSse2Cvttsd2si32c() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.d = -5000.6f; // should truncate

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
#endif
    testSseReg32r(0, 0xF2, 0x2c, 0,   f1.i, ((U64)2) << 32 | 1, 0x80000000);
    testSseReg32r(0, 0xF2, 0x2c, 0,  f2.i, ((U64)2) << 32 | 1, 0xffffec78);
}

void testSse2Cvtpd2pi12d() {
    TestDouble f1;
    TestDouble f2;

    f1.d = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.d = -5000.0f; // :TODO: test rounding

    U64 expected = 0xffffec7880000000;
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
#endif 
    testSseMmx64r(0, 0x66, 0x2d, 0, f1.i, f2.i, expected);
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

    f1.f = -50.0;
    f2.f = 175.0;
    f3.f = 0.0;
    f4.f = 1000000.0;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u32[0] = f1.i;
    d1.m128i_u32[1] = f2.i;
    d1.m128i_u32[2] = f3.i;
    d1.m128i_u32[3] = f4.i;
    __m128i expected;
    expected.m128i_i32[0] = -50;
    expected.m128i_i32[1] = 175;
    expected.m128i_i32[2] = 0;
    expected.m128i_i32[3] = 1000000;
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
    testSse128(0, 0x66, 0x5b, 0, 0, f1.i | (U64)f2.i << 32, f3.i | (U64)f4.i << 32, 0x000000afffffffce, 0x000f424000000000);
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
    f2.f = 175.7f;
    f3.f = 0.0f;
    f4.f = 1000000.4f;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1;
    d1.m128i_u32[0] = f1.i;
    d1.m128i_u32[1] = f2.i;
    d1.m128i_u32[2] = f3.i;
    d1.m128i_u32[3] = f4.i;
    __m128i expected;
    expected.m128i_i32[0] = -50;
    expected.m128i_i32[1] = 175;
    expected.m128i_i32[2] = 0;
    expected.m128i_i32[3] = 1000000;
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
    testSse128(0, 0xf3, 0x5b, 0, 0, f1.i | (U64)f2.i << 32, f3.i | (U64)f4.i << 32, 0x000000afffffffce, 0x000f424000000000);
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
#endif 
    testSse128(0, 0x66, 0x5d, f1.i, f3.i, f2.i, f4.i, f2.i, f3.i);
}

void testSse2Minsd35d() {
    TestDouble f1;
    TestDouble f2;
    TestDouble f3;
    TestDouble f4;

    f1.d = 4.0;
    f2.d = -5.0;
    f3.d = 5.0;
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
        minsd xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("minsd failed");
    }
#endif 
    testSse128(0, 0xf2, 0x5d, f1.i, f3.i, f2.i, f4.i, f2.i, f3.i);
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

void testSee2xPunpcklwd161() {
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

void testSee2Packsswb163() {
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
    d2.m128i_i16[3] = 0xabcd;
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

#endif
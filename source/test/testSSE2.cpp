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

#endif
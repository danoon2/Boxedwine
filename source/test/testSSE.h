#ifndef __TEST_SSE_H__
#define __TEST_SSE_H__

void testSse128(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl=0, U64 memResulth=0);
void testSse128r(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl=0, U64 memResulth=0);
void testSse128E64(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl=0, U64 memResulth=0);
void testSseMmx64(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2, U64 xmmResultl, U64 xmmResulth);
void testSseReg32(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U32 value2, U64 xmmResultl, U64 xmmResulth);
void testSseMmx64r(U8 preOp1, U8 preOp2, U8 op, U64 value1, U64 value2l, U64 value2h, U64 mmxResult);
void testSseReg32r(U8 preOp1, U8 preOp2, U8 op, U32 value1, U64 value2l, U64 value2h, U32 result, U32 memResult=0);
void testSse128f(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U32 flagsResult);
void testSseE32r(U8 preOp1, U8 preOp2, U8 op, U32 value1, U64 value2l, U64 value2h, U32 result, U32 memResult=0);
void testSse128imm(U8 preOp1, U8 preOp2, U8 op, U8 imm, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth);
void testSse128SubImm(U8 preOp1, U8 preOp2, U8 op, U8 g, U8 imm, U64 value1l, U64 value1h, U64 xmmResultl, U64 xmmResulth);

#ifdef _MSC_VER // if Visual C/C++
__inline __m64 _mm_set_pi64x (const __int64 i) {
    union {
        __int64 i;
        __m64 v;
    } u;

    u.i = i;
    return u.v;
}
#endif

#define SSE_MEM_VALUE128_DEFAULT1 0x1234567890abcdefl
#define SSE_MEM_VALUE128_DEFAULT2 0x24680bdf13579acel
#define SSE_MEM_VALUE_TMP_OFFSET 64

#define SSE_MEM_VALUE128_LOW 0xaabbccddeeff2468l
#define SSE_MEM_VALUE128_HIGH 0x1122334455667788l

void testSseMovUps310();
void testSseMovSs310();
void testSseMovUps311();
void testSseMovSs311();
void testSseMovHlps312();
void testSseMovLps312();
void testSseMovLps313();
void testSseUnpcklps314();
void testSseUnpckhps315();
void testSseMovlhps316();
void testSseMovhps316();
void testSseMovhps317();
void testSseMovaps328();
void testSseMovaps329();
void testSseCvtpi2ps32a();
void testSseCvtsi2ss32a();
void testSseMovntps32b();
void testSseCvttps2pi32c();
void testSseCvttss2si32c();
void testSseCvtps2pi32d();
void testSseCvtss2si32d();
void testSseUcomiss32e();
void testSseComiss32f();
void testSseMovmskps350();
void testSseSqrtps351();
void testSseSqrtss351();
void testSseRsqrtps352();
void testSseRsqrtss352();
void testSseRcpps353();
void testSseRcpss353();
void testSseAndps354();
void testSseAndnps355();
void testSseOrps356();
void testSseXorps357();
void testSseAddps358();
void testSseAddss358();
void testSseMulps359();
void testSseMulss359();
void testSseSubps35c();
void testSseSubss35c();
void testSseMinps35d();
void testSseMinss35d();
void testSseDivps35e();
void testSseDivss35e();
void testSseMaxps35f();
void testSseMaxss35f();
void testSsePshufw370();
void testCmpps0x3c2();
void testCmpss0x3c2();
void testPinsrw3c4();
void testPextrw3c5();
void testShufps3c6();
void testPmovmskb3d7();
void testPmovmskb1d7();
void testPminub3da();
void testPmaxub3de();
void testPavgb3e0();
void testPavgw3e3();
void testPmulhuw3e4();
void testMovntq3e7();
void testPminsw3ea();
void testPmaxsw3ee();
void testPsadbw3f6();
void testMaskmovq3f7();

#endif
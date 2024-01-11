#include "boxedwine.h"

#ifdef __TEST

#ifdef BOXEDWINE_MSVC
#include <nmmintrin.h>
#endif

#include "testCPU.h"
#include "testMMX.h"

extern KMemory* memory;

void testMmxEmms() {

}

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
#define X86_TEST_MMX(d1, d2, result, inst)     \
 { U64 tmp = 0; U64 data1=d1; U64 data2=d2; \
 __asm {    \
    __asm movq mm0, data1  \
    __asm movq mm1, data2  \
    __asm inst mm0, mm1    \
    __asm movq tmp, mm0 \
    __asm emms             \
}                           \
if (result!=tmp) {failed("failed");}}

#define X86_TEST_SSE(d1, d2, result, inst)     \
 { __m128 tmp = 0; __m128 data1=d1; __m128 data2=d2; \
 __asm {    \
    __asm movq xmm0, data1  \
    __asm movq xmm1, data2  \
    __asm inst xmm0, xmm1    \
    __asm movq tmp, xmm0 \
    __asm emms             \
}                           \
if (result!=tmp) {failed("failed");}}

#define X86_TEST_MMX_SUB(d1, d2, result, inst)     \
 { U64 tmp = 0; U64 data1=d1; U64 data2=d2; \
 __asm {    \
    __asm movq mm0, data1  \
    __asm inst mm0, data2    \
    __asm movq tmp, mm0 \
    __asm emms             \
}                           \
if (result!=tmp) {failed("failed");}}

#else
#define X86_TEST_MMX(data1, data2, r, inst)
#define X86_TEST_MMX_SUB(data1, data2, r, inst)
#endif

void initMmxTest() {    
    newInstruction(0);
    memory->writeq(cpu->seg[DS].address, MMX_MEM_VALUE64_DEFAULT);
    memory->writeq(cpu->seg[DS].address+MMX_MEM_VALUE64_OFFSET, MMX_MEM_VALUE64);
    memory->writed(cpu->seg[DS].address+MMX_MEM_VALUE32_OFFSET, MMX_MEM_VALUE32);
    for (int i=0;i<8;i++) {
        // movq  mm0, QWORD PTR ds:0x0
        pushCode8(0x0f);
        pushCode8(0x6F);
        pushCode8(0x04 | (i<<3));
        pushCode8(0x25);
        pushCode32(0);
    }
    cpu->big = 1;
}

void loadMMX(U8 reg, U32 index, U64 value) {
    memory->writeq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET+index*16, value);
    pushCode8(0x0f);
    pushCode8(0x6F);
    pushCode8(0x04 | (reg<<3));
    pushCode8(0x25);
    pushCode32(MMX_MEM_VALUE_TMP_OFFSET+index*16);
}

void testMmxMovdToMmx() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U32 data = 0x12345678;
    U64 result = 0;

    __asm {
        movd mm0, data
        movq result, mm0
        emms
    }
    if (result!=0x12345678l) {
        failed("movd failed");
    }
#endif    
    for (U8 m=0;m<8;m++) {
        for (U8 r=0;r<8;r++) {
            initMmxTest();            
            pushCode8(0x0f);
            pushCode8(0x6E);
            pushCode8(0xC0 | (m<<3) | r);
            cpu->reg[r].u32 = MMX_MEM_VALUE32;
            runTestCPU();
            for (U8 m1=0;m1<8;m1++) {
                if (m1==m) {
                    if (cpu->reg_mmx[m].q!=(U64)MMX_MEM_VALUE32) {
                        failed("movd failed");
                    }
                } else {
                    if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                        failed("movd failed");
                    }
                }
            }
        }
        initMmxTest();         
        pushCode8(0x0f);
        pushCode8(0x6E);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE32_OFFSET);
        runTestCPU();
        for (U8 m1=0;m1<8;m1++) {
            if (m1==m) {
                if (cpu->reg_mmx[m].q!=MMX_MEM_VALUE32) {
                    failed("movd failed");
                }
            } else {
                if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                    failed("movd failed");
                }
            }
        }
    }    
}

void testMmxMovdToE() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 data = 0x1234567890abcdef;
    U32 result = 0;

    __asm {
        push eax;
        movd mm0, data
        movd eax, mm0
        mov result, eax;
        pop eax;
        emms
    }
    if (result!=0x90abcdef) {
        failed("movd failed");
    }
#endif    
    for (U8 m=0;m<8;m++) {
        for (U8 r=0;r<8;r++) {
            initMmxTest();     
            loadMMX(m, 0, MMX_MEM_VALUE32);
            pushCode8(0x0f);
            pushCode8(0x7E);
            pushCode8(0xC0 | (m<<3) | r);
            cpu->reg[r].u32 = 0;
            runTestCPU();
            if (cpu->reg[r].u32!=MMX_MEM_VALUE32) {
                failed("movd failed");
            }
        }
        initMmxTest();      
        loadMMX(m, 1, MMX_MEM_VALUE64);
        memory->writed(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET, 0x11111111);
        pushCode8(0x0f);
        pushCode8(0x7E);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE_TMP_OFFSET);
        runTestCPU();
        if (memory->readd(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET)!=(U32)(MMX_MEM_VALUE64)) {
            failed("movd failed");
        }
    }    
}

void testMmx64(U8 op, U64 value1, U64 value2, U64 result) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            if (m==from) {
                continue;
            }
            initMmxTest();            
            loadMMX(m, 0, value1);
            loadMMX(from, 1, value2);
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();
            for (U8 m1=0;m1<8;m1++) {
                if (m1==m || m1==from) {
                    if (cpu->reg_mmx[m].q!=result) {
                        failed("mmx failed");
                    }
                } else {
                    if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                        failed("mmx failed");
                    }
                }
            }
        }
        initMmxTest();         
        loadMMX(m, 0, value1);
        memory->writeq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET+16, value2);
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE_TMP_OFFSET+16);
        runTestCPU();
        for (U8 m1=0;m1<8;m1++) {
            if (m1==m) {
                if (cpu->reg_mmx[m].q!=result) {
                    failed("mmx failed");
                }
            } else {
                if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                    failed("mmx failed");
                }
            }
        }
    }    
}

void testMmx64Reg(U8 op, U64 value1, U32 value2, U32 result) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            initMmxTest(); 
            for (int i=0;i<8;i++) {
                cpu->reg[i].u32 = DEFAULT;
            }
            loadMMX(from, 0, value1);
            cpu->reg[m].u32 = value2;
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();
            if (cpu->reg[m].u32!=result) {
                failed("mmx failed");
            }
        }
    }    
}

void testMmx64Eimm8(U8 preOp1, U8 op, U64 value1, U32 value2, U64 result, U8 imm8) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            initMmxTest();            
            loadMMX(m, 0, value1);
            for (int i=0;i<8;i++) {
                cpu->reg[from].u32 = DEFAULT;
            }
            cpu->reg[from].u32 = value2;
            if (preOp1) {
                pushCode8(preOp1);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            pushCode8(imm8);
            runTestCPU();
            for (U8 m1=0;m1<8;m1++) {
                if (m1==m || m1==from) {
                    if (cpu->reg_mmx[m].q!=result) {
                        failed("mmx failed");
                    }
                } else {
                    if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                        failed("mmx failed");
                    }
                }
            }
        }
        initMmxTest();         
        loadMMX(m, 0, value1);
        memory->writed(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET+16, value2);
        if (preOp1) {
            pushCode8(preOp1);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE_TMP_OFFSET+16);
        pushCode8(imm8);
        runTestCPU();
        for (U8 m1=0;m1<8;m1++) {
            if (m1==m) {
                if (cpu->reg_mmx[m].q!=result) {
                    failed("mmx failed");
                }
            } else {
                if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                    failed("mmx failed");
                }
            }
        }
    }    
}

void testRegMmx64imm8(U8 preOp1, U8 op, U32 value1, U64 value2, U32 result, U8 imm8) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            initMmxTest();            
            loadMMX(from, 0, value2);
            for (int i=0;i<8;i++) {
                cpu->reg[from].u32 = DEFAULT;
            }
            cpu->reg[m].u32 = value1;
            if (preOp1) {
                pushCode8(preOp1);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            pushCode8(imm8);
            runTestCPU();
            if (cpu->reg[m].u32!=result) {
                failed("mmx failed");
            }
        }
    }    
}

void testMmx64imm8(U8 preOp1, U8 op, U64 value1, U64 value2, U64 result, U8 imm8) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            if (m==from) {
                continue;
            }
            initMmxTest();            
            loadMMX(m, 0, value1);
            loadMMX(from, 1, value2);
            if (preOp1) {
                pushCode8(preOp1);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            pushCode8(imm8);
            runTestCPU();
            for (U8 m1=0;m1<8;m1++) {
                if (m1==m || m1==from) {
                    if (cpu->reg_mmx[m].q!=result) {
                        failed("mmx failed");
                    }
                } else {
                    if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                        failed("mmx failed");
                    }
                }
            }
        }
        initMmxTest();         
        loadMMX(m, 0, value1);
        memory->writeq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET+16, value2);
        if (preOp1) {
            pushCode8(preOp1);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE_TMP_OFFSET+16);
        pushCode8(imm8);
        runTestCPU();
        for (U8 m1=0;m1<8;m1++) {
            if (m1==m) {
                if (cpu->reg_mmx[m].q!=result) {
                    failed("mmx failed");
                }
            } else {
                if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                    failed("mmx failed");
                }
            }
        }
    }    
}

void testMmx64imm8Sub(U8 op, U8 g, U64 value1, U8 value2, U64 result) {
    for (U8 m=0;m<8;m++) {
        initMmxTest();            
        loadMMX(m, 0, value1);
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0xC0 | m | (g<<3));            
        pushCode8(value2);
        runTestCPU();
        for (U8 m1=0;m1<8;m1++) {
            if (m1==m) {
                if (cpu->reg_mmx[m].q!=result) {
                    failed("mmx failed");
                }
            } else {
                if (cpu->reg_mmx[m1].q!=MMX_MEM_VALUE64_DEFAULT) {
                    failed("mmx failed");
                }
            }
        }
    }    
}

void testMmxMovqToMmx() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 data = 0x1234567890abcdefl;
    U64 result = 0;

    __asm {
        movq mm0, data
        movq result, mm0
        emms
    }
    if (result!=0x1234567890abcdefl) {
        failed("movq failed");
    }
#endif    
    testMmx64(0x6f, MMX_MEM_VALUE64_DEFAULT, MMX_MEM_VALUE64, MMX_MEM_VALUE64);
}

void testMmxMovqToE() {   
    for (U8 m=0;m<8;m++) {
        for (U8 r=0;r<8;r++) {
            if (m==r) {
                continue;
            }
            initMmxTest();     
            loadMMX(m, 0, MMX_MEM_VALUE64);
            pushCode8(0x0f);
            pushCode8(0x7F);
            pushCode8(0xC0 | (m<<3) | r);
            cpu->reg[r].u32 = 0;
            runTestCPU();
            if (cpu->reg_mmx[r].q!=MMX_MEM_VALUE64) {
                failed("movd failed");
            }
        }
        initMmxTest();      
        loadMMX(m, 1, MMX_MEM_VALUE64);
        memory->writeq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET, 0x1111111111111111l);
        pushCode8(0x0f);
        pushCode8(0x7F);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE_TMP_OFFSET);
        runTestCPU();
        if (memory->readq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET)!=MMX_MEM_VALUE64) {
            failed("movd failed");
        }
    }    
}

void testMmxPaddb() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aaddccl, 0x88aaccee10327676, paddb);
    testMmx64(0xfc, 0x33445566778899aal, 0x5566778899aaddccl, 0x88aaccee10327676);
}

void testMmxPaddw() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aaddccl, 0x88aaccee11327776, paddw);
    testMmx64(0xfd, 0x33445566778899aal, 0x5566778899aaddccl, 0x88aaccee11327776);
}

void testMmxPaddd() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aaddccl, 0x88aaccee11337776, paddd);
    testMmx64(0xfe, 0x33445566778899aal, 0x5566778899aaddccl, 0x88aaccee11337776);
}

void testMmxPaddsb() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aa11ccl, 0x7f7f7fee1080aa80, paddsb);
    testMmx64(0xec, 0x33445566778899aal, 0x5566778899aa11ccl, 0x7f7f7fee1080aa80);
}

void testMmxPaddsw() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aa11ccl, 0x7fff7fff1132ab76, paddsw);
    testMmx64(0xed, 0x33445566778899aal, 0x5566778899aa11ccl, 0x7fff7fff1132ab76);
}

void testMmxPaddusb() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aa11ccl, 0x88aacceeffffaaff, paddusb);
    testMmx64(0xdc, 0x33445566778899aal, 0x5566778899aa11ccl, 0x88aacceeffffaaff);
}

void testMmxPaddusw() {
    X86_TEST_MMX(0x33445566778899aal, 0x5566778899aa11ccl, 0x88aacceeffffab76, paddusw);
    testMmx64(0xdd, 0x33445566778899aal, 0x5566778899aa11ccl, 0x88aacceeffffab76);
}

void testMmxPsubb() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x22bc3300deddccbbl, psubb);
    testMmx64(0xf8, 0x33445566778899aal, 0x1188226699abcdefl, 0x22bc3300deddccbbl);
}

void testMmxPsubw() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x21bc3300ddddcbbbl, psubw);
    testMmx64(0xf9, 0x33445566778899aal, 0x1188226699abcdefl, 0x21bc3300ddddcbbbl);
}

void testMmxPsubd() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x21bc3300dddccbbbl, psubd);
    testMmx64(0xfa, 0x33445566778899aal, 0x1188226699abcdefl, 0x21bc3300dddccbbbl);
}

void testMmxPsubsb() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x227f33007fddccbbl, psubsb);
    testMmx64(0xe8, 0x33445566778899aal, 0x1188226699abcdefl, 0x227f33007fddccbbl);
}

void testMmxPsubsw() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x21bc33007fffcbbbl, psubsw);
    testMmx64(0xe9, 0x33445566778899aal, 0x1188226699abcdefl, 0x21bc33007fffcbbbl);
}

void testMmxPsubusb() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x2200330000000000l, psubusb);
    testMmx64(0xd8, 0x33445566778899aal, 0x1188226699abcdefl, 0x2200330000000000l);
}

void testMmxPsubusw() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    // picked numbers that would give different results than paddsw
    U64 data1 = 0x33445566778899aal;
    U64 data2 = 0x1188226699abcdefl;
    U64 result = 0;

    __asm {
        movq mm0, data1
        movq mm1, data2
        psubusw mm0, mm1
        movq result, mm0
        emms
    }
    if (result!=0x21bc330000000000l) {
        failed("paddsb failed");
    }
#endif 
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x21bc330000000000l, psubusw);
    testMmx64(0xd9, 0x33445566778899aal, 0x1188226699abcdefl, 0x21bc330000000000l);
}

void testMmxPmulhw() {
    X86_TEST_MMX(0x03e8fc1800007fff, 0x03e803e855557fff, 0x000ffff000003fff, pmulhw);
    testMmx64(0xe5, 0x03e8fc1800007fff, 0x03e803e855557fff, 0x000ffff000003fff);
}

void testMmxPmullw() {
    X86_TEST_MMX(0x03e8fc1800007fff, 0x03e803e855557fff, 0x4240bdc000000001, pmullw);
    testMmx64(0xd5, 0x03e8fc1800007fff, 0x03e803e855557fff, 0x4240bdc000000001);
}

void testMmxPmaddwd() {
    X86_TEST_MMX(0x03e9fc1800007fff, 0x03e803e855557fff, 0x000003e83fff0001, pmaddwd);
    testMmx64(0xf5, 0x03e9fc1800007fff, 0x03e803e855557fff, 0x000003e83fff0001);
}

void testMmxPcmpeqb() {
    X86_TEST_MMX(0x030000feff011234, 0x03010001fffe1234, 0xff00ff00ff00ffff, pcmpeqb);
    testMmx64(0x74, 0x030000feff011234, 0x03010001fffe1234, 0xff00ff00ff00ffff);
}

void testMmxPcmpeqw() {
    X86_TEST_MMX(0x030000feff011234, 0x03010001fffe1234, 0x000000000000ffff, pcmpeqw);
    testMmx64(0x75, 0x030000feff011234, 0x03010001fffe1234, 0x000000000000ffff);
}

void testMmxPcmpeqd() {
    X86_TEST_MMX(0x030000feff011234, 0x03010001ff011234, 0x00000000ffffffff, pcmpeqd);
    testMmx64(0x76, 0x030000feff011234, 0x03010001ff011234, 0x00000000ffffffff);
}

void testMmxPcmpgtb() {
    X86_TEST_MMX(0x030100feff011234, 0x03000001fffe1234, 0x00ff000000ff0000, pcmpgtb);
    testMmx64(0x64, 0x030100feff011234, 0x03000001fffe1234, 0x00ff000000ff0000);
}

void testMmxPcmpgtw() {
    X86_TEST_MMX(0x0301fffefffe1334, 0x03000001fffe1234, 0xffff00000000ffff, pcmpgtw);
    testMmx64(0x65, 0x0301fffefffe1334, 0x03000001fffe1234, 0xffff00000000ffff);
}

void testMmxPcmpgtd() {
    X86_TEST_MMX(0x03010001ff011234, 0x03000ffefffe1234, 0xffffffff00000000, pcmpgtd);
    testMmx64(0x66, 0x03010001ff011234, 0x03000ffefffe1234, 0xffffffff00000000);
}

void testMmxPackssdw() {
    X86_TEST_MMX(0x00000030ffee1234, 0x00030000fffff234, 0x7ffff23400308000, packssdw);
    testMmx64(0x6b, 0x00000030ffee1234, 0x00030000fffff234, 0x7ffff23400308000);
}

void testMmxPacksswb() {
    X86_TEST_MMX(0x000300300300fffe, 0x00eb00000fff0034, 0x7f007f3403307ffe, packsswb);
    testMmx64(0x63, 0x000300300300fffe, 0x00eb00000fff0034, 0x7f007f3403307ffe);
}

void testMmxPackuswb() {
    X86_TEST_MMX(0x000300300300fffe, 0x00eb00000fff0034, 0xeb00ff340330ff00, packuswb);
    testMmx64(0x67, 0x000300300300fffe, 0x00eb00000fff0034, 0xeb00ff340330ff00);
}

void testMmxPunpckhbw() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x99110022aa33bb44, punpckhbw);
    testMmx64(0x68, 0x1122334455667788, 0x9900aabbccddeeff, 0x99110022aa33bb44);
}

void testMmxPunpckhdq() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x9900aabb11223344, punpckhdq);
    testMmx64(0x6a, 0x1122334455667788, 0x9900aabbccddeeff, 0x9900aabb11223344);
}

void testMmxPunpckhwd() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x99001122aabb3344, punpckhwd);
    testMmx64(0x69, 0x1122334455667788, 0x9900aabbccddeeff, 0x99001122aabb3344);
}

void testMmxPunpcklbw() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0xcc55dd66ee77ff88, punpcklbw);
    testMmx64(0x60, 0x1122334455667788, 0x9900aabbccddeeff, 0xcc55dd66ee77ff88);
}

void testMmxPunpckldq() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0xccddeeff55667788, punpckldq);
    testMmx64(0x62, 0x1122334455667788, 0x9900aabbccddeeff, 0xccddeeff55667788);
}

void testMmxPunpcklwd() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0xccdd5566eeff7788, punpcklwd);
    testMmx64(0x61, 0x1122334455667788, 0x9900aabbccddeeff, 0xccdd5566eeff7788);
}

void testMmxPxor() {
    #if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    U64 data1 = 0x1122334455667788;
    U64 data2 = 0x9900aabbccddeeff;
    U64 result = 0;

    __asm {
        movq mm0, data1
        movq mm1, data2
        pxor mm0, mm1
        movq result, mm0
        emms
    }
    if (result!=0x882299ff99bb9977) {
        failed("paddsb failed");
    }
#endif 
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x882299ff99bb9977, pxor);
    testMmx64(0xef, 0x1122334455667788, 0x9900aabbccddeeff, 0x882299ff99bb9977);
}

void testMmxPor() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x9922bbffddffffff, por);
    testMmx64(0xeb, 0x1122334455667788, 0x9900aabbccddeeff, 0x9922bbffddffffff);
}

void testMmxPand() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x1100220044446688, pand);
    testMmx64(0xdb, 0x1122334455667788, 0x9900aabbccddeeff, 0x1100220044446688);
}

void testMmxPandn() {
    X86_TEST_MMX(0x1122334455667788, 0x9900aabbccddeeff, 0x880088bb88998877, pandn);
    testMmx64(0xdf, 0x1122334455667788, 0x9900aabbccddeeff, 0x880088bb88998877);
}

void testMmxPsllwImm8() {
    X86_TEST_MMX_SUB(0x0102f00712345678, 1, 0x0204e00e2468acf0, psllw);
    testMmx64imm8Sub(0x71, 6, 0x0102f00712345678, 1, 0x0204e00e2468acf0);

    X86_TEST_MMX_SUB(0x0102f00712345678, 8, 0x0200070034007800, psllw);
    testMmx64imm8Sub(0x71, 6, 0x0102f00712345678, 8, 0x0200070034007800);
}

void testMmxPsllw() {
    X86_TEST_MMX(0x0102f00712345678, 1, 0x0204e00e2468acf0, psllw);
    testMmx64(0xf1, 0x0102f00712345678, 1, 0x0204e00e2468acf0);

    X86_TEST_MMX(0x0102f00712345678, 8, 0x0200070034007800, psllw);
    testMmx64(0xf1, 0x0102f00712345678, 8, 0x0200070034007800);
}

void testMmxPslldImm8() {
    X86_TEST_MMX_SUB(0x0102f007F2345678, 1, 0x0205e00ee468acf0, pslld);
    testMmx64imm8Sub(0x72, 6, 0x0102f007F2345678, 1, 0x0205e00ee468acf0);

    X86_TEST_MMX_SUB(0x0102f00712345678, 16, 0xf007000056780000, pslld);
    testMmx64imm8Sub(0x72, 6, 0x0102f00712345678, 16, 0xf007000056780000);
}

void testMmxPslld() {
    X86_TEST_MMX(0x0102f007F2345678, 1, 0x0205e00ee468acf0, pslld);
    testMmx64(0xf2, 0x0102f007F2345678, 1, 0x0205e00ee468acf0);

    X86_TEST_MMX(0x0102f00712345678, 16, 0xf007000056780000, pslld);
    testMmx64(0xf2, 0x0102f00712345678, 16, 0xf007000056780000);
}

void testMmxPsllqImm8() {
    X86_TEST_MMX_SUB(0x0102f00712345678, 1, 0x0205e00e2468acf0, psllq);
    testMmx64imm8Sub(0x73, 6, 0x0102f00712345678, 1, 0x0205e00e2468acf0);

    X86_TEST_MMX_SUB(0x0102f007F2345678, 32, 0xf234567800000000, psllq);
    testMmx64imm8Sub(0x73, 6, 0x0102f007F2345678, 32, 0xf234567800000000);
}

void testMmxPsllq() {
    X86_TEST_MMX(0x0102f007F2345678, 1, 0x0205e00fe468acf0, psllq);
    testMmx64(0xf3, 0x0102f007F2345678, 1, 0x0205e00fe468acf0);

    X86_TEST_MMX(0x0102f007F2345678, 32, 0xf234567800000000, psllq);
    testMmx64(0xf3, 0x0102f007F2345678, 32, 0xf234567800000000);
}

void testMmxPsrlwImm8() {
    X86_TEST_MMX_SUB(0x0102f007F2345678, 1, 0x00817803791a2b3c, psrlw);
    testMmx64imm8Sub(0x71, 2, 0x0102f007F2345678, 1, 0x00817803791a2b3c);

    X86_TEST_MMX_SUB(0x0102f007F2345678, 8, 0x000100f000f20056, psrlw);
    testMmx64imm8Sub(0x71, 2, 0x0102f007F2345678, 8, 0x000100f000f20056);
}

void testMmxPsrlw() {
    X86_TEST_MMX(0x0102f007F2345678, 1, 0x00817803791a2b3c, psrlw);
    testMmx64(0xd1, 0x0102f007F2345678, 1, 0x00817803791a2b3c);

    X86_TEST_MMX(0x0102f007F2345678, 8, 0x000100f000f20056, psrlw);
    testMmx64(0xd1, 0x0102f007F2345678, 8, 0x000100f000f20056);
}

void testMmxPsrldImm8() {
    X86_TEST_MMX_SUB(0x0102f007F2345678, 1, 0x00817803791a2b3c, psrld);
    testMmx64imm8Sub(0x72, 2, 0x0102f007F2345678, 1, 0x00817803791a2b3c);

    X86_TEST_MMX_SUB(0x0102f007F2345678, 16, 0x000001020000f234, psrld);
    testMmx64imm8Sub(0x72, 2, 0x0102f007F2345678, 16, 0x000001020000f234);
}

void testMmxPsrld() {
    X86_TEST_MMX(0x0102f007F2345678, 1, 0x00817803791a2b3c, psrld);
    testMmx64(0xd2, 0x0102f007F2345678, 1, 0x00817803791a2b3c);

    X86_TEST_MMX(0x0102f007F2345678, 16, 0x000001020000f234, psrld);
    testMmx64(0xd2, 0x0102f007F2345678, 16, 0x000001020000f234);
}

void testMmxPsrlqImm8() {
    X86_TEST_MMX_SUB(0xf102f007F2345678, 1, 0x78817803f91a2b3c, psrlq);
    testMmx64imm8Sub(0x73, 2, 0xf102f007F2345678, 1, 0x78817803f91a2b3c);

    X86_TEST_MMX_SUB(0xf102f007F2345678, 32, 0x00000000f102f007, psrlq);
    testMmx64imm8Sub(0x73, 2, 0xf102f007F2345678, 32, 0x00000000f102f007);
}

void testMmxPsrlq() {
    X86_TEST_MMX(0xf102f007F2345678, 1, 0x78817803f91a2b3c, psrlq);
    testMmx64(0xd3, 0xf102f007F2345678, 1, 0x78817803f91a2b3c);

    X86_TEST_MMX(0xf102f007F2345678, 32, 0x00000000f102f007, psrlq);
    testMmx64(0xd3, 0xf102f007F2345678, 32, 0x00000000f102f007);
}

void testMmxPsrawImm8() {
    X86_TEST_MMX_SUB(0xf102f007F2345678, 1, 0xf881f803f91a2b3c, psraw);
    testMmx64imm8Sub(0x71, 4, 0xf102f007F2345678, 1, 0xf881f803f91a2b3c);

    X86_TEST_MMX_SUB(0xf102f007F2345678, 8, 0xfff1fff0fff20056, psraw);
    testMmx64imm8Sub(0x71, 4, 0xf102f007F2345678, 8, 0xfff1fff0fff20056);
}

void testMmxPsraw() {
    X86_TEST_MMX(0xf102f007F2345678, 1, 0xf881f803f91a2b3c, psraw);
    testMmx64(0xe1, 0xf102f007F2345678, 1, 0xf881f803f91a2b3c);

    X86_TEST_MMX(0xf102f007F2345678, 8, 0xfff1fff0fff20056, psraw);
    testMmx64(0xe1, 0xf102f007F2345678, 8, 0xfff1fff0fff20056);
}

void testMmxPsradImm8() {
    X86_TEST_MMX_SUB(0xf102f00772345678, 1, 0xf8817803391a2b3c, psrad);
    testMmx64imm8Sub(0x72, 4, 0xf102f00772345678, 1, 0xf8817803391a2b3c);

    X86_TEST_MMX_SUB(0xf102f00772345678, 16, 0xfffff10200007234, psrad);
    testMmx64imm8Sub(0x72, 4, 0xf102f00772345678, 16, 0xfffff10200007234);
}

void testMmxPsrad() {
    X86_TEST_MMX(0xf102f00772345678, 1, 0xf8817803391a2b3c, psrad);
    testMmx64(0xe2, 0xf102f00772345678, 1, 0xf8817803391a2b3c);

    X86_TEST_MMX(0xf102f00772345678, 16, 0xfffff10200007234, psrad);
    testMmx64(0xe2, 0xf102f00772345678, 16, 0xfffff10200007234);
}

void testMmxPaddq3d4() {
    X86_TEST_MMX(0xf2345678f0abcdef, 0x5555555566666666, 0x4789abce57123455, paddq);
    testMmx64(0xd4, 0xf2345678f0abcdef, 0x5555555566666666, 0x4789abce57123455);
}

void testSse2Pmuludq3f4() {
    X86_TEST_MMX(0x0102f007F2345678, 0x5555555512345678, 0x113932851df4d840, pmuludq);
    testMmx64(0xf4, 0x0102f007F2345678, 0x5555555512345678, 0x113932851df4d840);
}

void testSse2Psubq3fb() {
    X86_TEST_MMX(0x33445566778899aal, 0x1188226699abcdefl, 0x21bc32ffdddccbbb, psubq);
    testMmx64(0xfb, 0x33445566778899aal, 0x1188226699abcdefl, 0x21bc32ffdddccbbb);
}

#endif
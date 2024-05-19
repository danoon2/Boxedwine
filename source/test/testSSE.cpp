#include "boxedwine.h"

#ifdef __TEST

#ifdef BOXEDWINE_MSVC
#include <nmmintrin.h>
#endif

#include "testCPU.h"
#include "testMMX.h"
#include "testSSE.h"

extern KMemory* memory;

void initSseTest() {    
    newInstruction(0);
    memory->writeq(cpu->seg[DS].address, SSE_MEM_VALUE128_DEFAULT1);
    memory->writeq(cpu->seg[DS].address+8, SSE_MEM_VALUE128_DEFAULT2);
    for (int i=0;i<8;i++) {
        // movq  xmm0, QWORD PTR ds:0x0
        pushCode8(0x0f);
        pushCode8(0x10);
        pushCode8(0x04 | (i<<3));
        pushCode8(0x25);
        pushCode32(0);
    }
    cpu->big = 1;
}

void loadSSE(U8 reg, U32 index, U64 value1l, U64 value1h) {
    memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+index*16, value1l);
    memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+index*16+8, value1h);

    pushCode8(0x0f);
    pushCode8(0x10);
    pushCode8(0x04 | (reg<<3));
    pushCode8(0x25);
    pushCode32(SSE_MEM_VALUE_TMP_OFFSET+index*16);
}

void testSse128(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl, U64 memResulth) {
    if (!memResultl && !memResulth) {
        memResultl = xmmResultl;
        memResulth = xmmResulth;
    }
    for (U8 m=0;m<8;m++) {
        if (xmmResultl!=0xFFFFFFFFFFFFFFFFl && xmmResulth!=0xFFFFFFFFFFFFFFFFl) {
            for (U8 from=0;from<8;from++) {
                if (m==from) {
                    continue;
                }
                initSseTest();            
                loadSSE(m, 0, value1l, value1h);
                loadSSE(from, 1, value2l, value2h);
                if (preOp1) {
                    pushCode8(preOp1);
                }
                if (preOp2) {
                    pushCode8(preOp2);
                }
                pushCode8(0x0f);
                pushCode8(op);
                pushCode8(0xC0 | (m << 3) | from);            
                runTestCPU();
                for (U8 m1=0;m1<8;m1++) {
                    if (m1==m || m1==from) {
                        TestDouble f1;
                        TestDouble t1;

                        t1.i = cpu->xmm[m].pi.u64[0];
                        f1.i = xmmResultl;
                        if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                            failed("sse failed");
                        }
                    } else {
                        if (cpu->xmm[m1].pi.u64[0]!=SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1]!=SSE_MEM_VALUE128_DEFAULT2) {
                            failed("sse failed");
                        }
                    }
                }
            }
        }
        if (memResultl!=0xFFFFFFFFFFFFFFFFl && memResulth!=0xFFFFFFFFFFFFFFFFl) {
            initSseTest();         
            loadSSE(m, 0, value1l, value1h);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2l);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24, value2h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();
            for (U8 m1=0;m1<8;m1++) {
                if (m1==m) {
                    if (cpu->xmm[m].pi.u64[0]!=memResultl || cpu->xmm[m].pi.u64[1]!=memResulth) {
                        failed("sse failed");
                    }
                } else {
                    if (cpu->xmm[m1].pi.u64[0]!=SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1]!=SSE_MEM_VALUE128_DEFAULT2) {
                        failed("sse failed");
                    }
                }
            }
        }
    }    
}

void testSse16Eimm8(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U32 value2, U64 xmmResultl, U64 xmmResulth, U8 imm8) {
    for (U8 m = 0; m < 8; m++) {
        for (U8 from = 0; from < 8; from++) {
            initSseTest();
            loadSSE(m, 0, value1l, value1h);
            for (int i = 0; i < 8; i++) {
                cpu->reg[from].u32 = DEFAULT;
            }
            cpu->reg[from].u32 = value2;
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);
            pushCode8(imm8);
            runTestCPU();
            for (U8 m1 = 0; m1 < 8; m1++) {
                if (m1 == m || m1 == from) {
                    if (cpu->xmm[m].pi.u64[0] != xmmResultl || cpu->xmm[m].pi.u64[1] != xmmResulth) {
                        failed("sse failed");
                    }
                } else {
                    if (cpu->xmm[m1].pi.u64[0] != SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1] != SSE_MEM_VALUE128_DEFAULT2) {
                        failed("sse failed");
                    }
                }
            }
        }
        initSseTest();
        loadSSE(m, 0, value1l, value1h);
        memory->writed(cpu->seg[DS].address + SSE_MEM_VALUE_TMP_OFFSET + 16, value2);
        if (preOp1) {
            pushCode8(preOp1);
        }
        if (preOp2) {
            pushCode8(preOp2);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m << 3));
        pushCode8(0x25);
        pushCode32(SSE_MEM_VALUE_TMP_OFFSET + 16);
        pushCode8(imm8);
        runTestCPU();
        for (U8 m1 = 0; m1 < 8; m1++) {
            if (m1 == m) {
                if (cpu->xmm[m].pi.u64[0] != xmmResultl || cpu->xmm[m].pi.u64[1] != xmmResulth) {
                    failed("sse failed");
                }
            } else {
                if (cpu->xmm[m1].pi.u64[0] != SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1] != SSE_MEM_VALUE128_DEFAULT2) {
                    failed("sse failed");
                }
            }
        }
    }
}

void testSse128imm(U8 preOp1, U8 preOp2, U8 op, U8 imm, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            if (m==from) {
                continue;
            }
            initSseTest();            
            loadSSE(m, 0, value1l, value1h);
            loadSSE(from, 1, value2l, value2h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            pushCode8(imm);
            runTestCPU();
            for (U8 m1=0;m1<8;m1++) {
                if (m1==m || m1==from) {
                    Test_Float f1;
                    Test_Float t1;

                    t1.i = cpu->xmm[m].pi.u32[0];
                    f1.i = (U32)xmmResultl;
                    if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                        failed("sse failed");
                    }
                } else {
                    if (cpu->xmm[m1].pi.u64[0]!=SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1]!=SSE_MEM_VALUE128_DEFAULT2) {
                        failed("sse failed");
                    }
                }
            }
        }
        initSseTest();         
        loadSSE(m, 0, value1l, value1h);
        memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2l);
        memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24, value2h);
        if (preOp1) {
            pushCode8(preOp1);
        }
        if (preOp2) {
            pushCode8(preOp2);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
        pushCode8(imm);
        runTestCPU();
        for (U8 m1=0;m1<8;m1++) {
            if (m1==m) {
                if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                    failed("sse failed");
                }
            } else {
                if (cpu->xmm[m1].pi.u64[0]!=SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1]!=SSE_MEM_VALUE128_DEFAULT2) {
                    failed("sse failed");
                }
            }
        }
    }    
}

void testSse128SubImm(U8 preOp1, U8 preOp2, U8 op, U8 g, U8 imm, U64 value1l, U64 value1h, U64 xmmResultl, U64 xmmResulth) {
    for (U8 m=0;m<8;m++) {
        initSseTest();            
        loadSSE(m, 0, value1l, value1h);
        if (preOp1) {
            pushCode8(preOp1);
        }
        if (preOp2) {
            pushCode8(preOp2);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0xC0 | (g << 3) | m);            
        pushCode8(imm);
        runTestCPU();
        if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
            failed("sse failed");
        }

    }    
}

void testSse128f(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U32 flagsResult) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            if (m==from) {
                continue;
            }
            initSseTest();            
            loadSSE(m, 0, value1l, value1h);
            loadSSE(from, 1, value2l, value2h);
            cpu->flags = 0;
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();
            U32 flags = cpu->flags & FLAG_MASK;
            if (flags != flagsResult) {
                failed("sse failed");
            }
        }

        initSseTest();         
        loadSSE(m, 0, value1l, value1h);
        memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2l);
        memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24, value2h);
        cpu->flags = 0;
        if (preOp1) {
            pushCode8(preOp1);
        }
        if (preOp2) {
            pushCode8(preOp2);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
        runTestCPU();
        U32 flags = cpu->flags & FLAG_MASK;
        if (flags != flagsResult) {
            failed("sse failed");
        }
    }  
}

void testSse128r(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl, U64 memResulth) {
    if (!memResultl && !memResulth) {
        memResultl = xmmResultl;
        memResulth = xmmResulth;
    }
    for (U8 m=0;m<8;m++) {
        if (xmmResultl!=0xFFFFFFFFFFFFFFFFl && xmmResulth!=0xFFFFFFFFFFFFFFFFl) {
            for (U8 from=0;from<8;from++) {
                if (m==from) {
                    continue;
                }
                initSseTest();            
                loadSSE(m, 0, value1l, value1h);
                loadSSE(from, 1, value2l, value2h);
                if (preOp1) {
                    pushCode8(preOp1);
                }
                if (preOp2) {
                    pushCode8(preOp2);
                }
                pushCode8(0x0f);
                pushCode8(op);
                pushCode8(0xC0 | m | (from << 3));            
                runTestCPU();
                for (U8 m1=0;m1<8;m1++) {
                    if (m1==m || m1==from) {
                        if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                            failed("sse failed");
                        }
                    } else {
                        if (cpu->xmm[m1].pi.u64[0]!=SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1]!=SSE_MEM_VALUE128_DEFAULT2) {
                            failed("sse failed");
                        }
                    }
                }
            }
        }
        if (memResultl!=0xFFFFFFFFFFFFFFFFl && memResulth!=0xFFFFFFFFFFFFFFFFl) {
            initSseTest();         
            loadSSE(m, 0, value2l, value2h);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value1l);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24, value1h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();

            U64 result1 = memory->readq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16);
            U64 result2 = memory->readq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24);
            if (result1!=memResultl || result2!=memResulth) {
                failed("sse failed");
            }
        }
    }    
}

void testSse128E64(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl, U64 memResulth) {
    if (!memResultl && !memResulth) {
        memResultl = xmmResultl;
        memResulth = xmmResulth;
    }
    for (U8 m=0;m<8;m++) {
        if (xmmResultl!=0xFFFFFFFFFFFFFFFFl && xmmResulth!=0xFFFFFFFFFFFFFFFFl) {
            for (U8 from=0;from<8;from++) {
                if (m==from) {
                    continue;
                }
                initSseTest();            
                loadSSE(m, 0, value1l, value1h);
                loadSSE(from, 1, value2l, value2h);
                if (preOp1) {
                    pushCode8(preOp1);
                }
                if (preOp2) {
                    pushCode8(preOp2);
                }
                pushCode8(0x0f);
                pushCode8(op);
                pushCode8(0xC0 | from | (m << 3));            
                runTestCPU();
                for (U8 m1=0;m1<8;m1++) {
                    if (m1==m || m1==from) {
                        if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                            failed("sse failed");
                        }
                    } else {
                        if (cpu->xmm[m1].pi.u64[0]!=SSE_MEM_VALUE128_DEFAULT1 || cpu->xmm[m1].pi.u64[1]!=SSE_MEM_VALUE128_DEFAULT2) {
                            failed("sse failed");
                        }
                    }
                }
            }
        }
        if (memResultl!=0xFFFFFFFFFFFFFFFFl && memResulth!=0xFFFFFFFFFFFFFFFFl) {
            initSseTest();         
            loadSSE(m, 0, value2l, value2h);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2l);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();

            if (cpu->xmm[m].pi.u64[0]!=memResultl || cpu->xmm[m].pi.u64[1]!=memResulth) {
                failed("sse failed");
            }
        }
    }    
}

void testSse128E64r(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2l, U64 value2h, U64 xmmResultl, U64 xmmResulth, U64 memResultl, U64 memResulth) {
    if (!memResultl && !memResulth) {
        memResultl = xmmResultl;
        memResulth = xmmResulth;
    }
    for (U8 m=0;m<8;m++) {
        if (xmmResultl!=0xFFFFFFFFFFFFFFFFl && xmmResulth!=0xFFFFFFFFFFFFFFFFl) {
            for (U8 from=0;from<8;from++) {
                if (m==from) {
                    continue;
                }
                initSseTest();            
                loadSSE(m, 0, value1l, value1h);
                loadSSE(from, 1, value2l, value2h);
                if (preOp1) {
                    pushCode8(preOp1);
                }
                if (preOp2) {
                    pushCode8(preOp2);
                }
                pushCode8(0x0f);
                pushCode8(op);
                pushCode8(0xC0 | m | (from << 3));            
                runTestCPU();
                if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                    failed("sse failed");
                }
            }
        }
        if (memResultl!=0xFFFFFFFFFFFFFFFFl && memResulth!=0xFFFFFFFFFFFFFFFFl) {
            initSseTest();         
            loadSSE(m, 0, value2l, value2h);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value1l);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24, value1h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();

            if (memory->readq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16)!=memResultl || memory->readq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24)!=memResulth) {
                failed("sse failed");
            }
        }
    }    
}

void testSseMmx64(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U64 value2, U64 xmmResultl, U64 xmmResulth, U64 memResultl, U64 memResulth) {
    if (!memResultl && !memResulth) {
        memResultl = xmmResultl;
        memResulth = xmmResulth;
    }
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            initSseTest();  
            initMmxTest();
            loadSSE(m, 0, value1l, value1h);
            loadMMX(from, 2, value2);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();

            if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                failed("sse failed");
            }
        }

        if (memResultl!=0xFFFFFFFFFFFFFFFF || memResulth!=0xFFFFFFFFFFFFFFFF) {
            initSseTest();         
            loadSSE(m, 0, value1l, value1h);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();
            if (cpu->xmm[m].pi.u64[0]!=memResultl || cpu->xmm[m].pi.u64[1]!=memResulth) {
                failed("sse failed");
            }
        }
    }    
}

void testSseMmx64r(U8 preOp1, U8 preOp2, U8 op, U64 value1, U64 value2l, U64 value2h, U64 mmxResult, U64 mmxMemResult) {
    if (!mmxMemResult) {
        mmxMemResult = mmxResult;
    }
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            initSseTest();  
            initMmxTest();            
            loadMMX(m, 2, value1);
            loadSSE(from, 0, value2l, value2h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();

            if (cpu->reg_mmx[m].q!=mmxResult) {
                failed("sse failed");
            }
        }
        if (mmxMemResult!=0xFFFFFFFFFFFFFFFF) {
            initSseTest();         
            loadMMX(m, 0, value1);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2l);
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+24, value2h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();
            if (cpu->reg_mmx[m].q!=mmxMemResult) {
                failed("sse failed");
            }
        }
    }    
}

void testRegSseImm8(U8 preOp1, U8 op, U64 value1l, U64 value1h, U32 value2, U32 result, U8 imm8) {
    for (U8 m = 0; m < 8; m++) {
        for (U8 from = 0; from < 8; from++) {
            initSseTest();
            loadSSE(from, 0, value1l, value1h);
            for (int i = 0; i < 8; i++) {
                cpu->reg[from].u32 = DEFAULT;
            }
            cpu->reg[m].u32 = value2;
            if (preOp1) {
                pushCode8(preOp1);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);
            pushCode8(imm8);
            runTestCPU();
            if (cpu->reg[m].u32 != result) {
                failed("sse failed");
            }
        }
    }
}

void testSseReg32(U8 preOp1, U8 preOp2, U8 op, U64 value1l, U64 value1h, U32 value2, U64 xmmResultl, U64 xmmResulth) {
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            initSseTest();  
            loadSSE(m, 0, value1l, value1h);
            cpu->reg[from].u32 = value2;
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();

            if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
                failed("sse failed");
            }
        }

        initSseTest();         
        loadSSE(m, 0, value1l, value1h);
        memory->writed(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2);
        if (preOp1) {
            pushCode8(preOp1);
        }
        if (preOp2) {
            pushCode8(preOp2);
        }
        pushCode8(0x0f);
        pushCode8(op);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
        runTestCPU();
        if (cpu->xmm[m].pi.u64[0]!=xmmResultl || cpu->xmm[m].pi.u64[1]!=xmmResulth) {
            failed("sse failed");
        }
    }    
}

void testSseReg32r(U8 preOp1, U8 preOp2, U8 op, U32 value1, U64 value2l, U64 value2h, U32 result, U32 memResult) {
    if (!memResult) {
        memResult = result;
    }
    for (U8 m=0;m<8;m++) {
        if (result!=0xFFFFFFFF) {
            for (U8 from=0;from<8;from++) {
                initSseTest();  
                loadSSE(from, 0, value2l, value2h);
                cpu->reg[m].u32 = value1;
                if (preOp1) {
                    pushCode8(preOp1);
                }
                if (preOp2) {
                    pushCode8(preOp2);
                }
                pushCode8(0x0f);
                pushCode8(op);
                pushCode8(0xC0 | (m << 3) | from);            
                runTestCPU();

                if (cpu->reg[m].u32!=result) {
                    failed("sse failed");
                }
            }
        }
        if (memResult!=0xFFFFFFFF) {
            initSseTest();         
            cpu->reg[m].u32 = value1;
            memory->writeq(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value2l);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (m<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();
            if (cpu->reg[m].u32!=result) {
                failed("sse failed");
            }
        }
    }    
}

void testSseE32r(U8 preOp1, U8 preOp2, U8 op, U32 value1, U64 value2l, U64 value2h, U32 result, U32 memResult) {
    if (!memResult) {
        memResult = result;
    }
    for (U8 from=0;from<8;from++) {
        if (result!=0xFFFFFFFF) {
            for (U8 m=0;m<8;m++) {
                initSseTest();  
                loadSSE(from, 0, value2l, value2h);
                cpu->reg[m].u32 = value1;
                if (preOp1) {
                    pushCode8(preOp1);
                }
                if (preOp2) {
                    pushCode8(preOp2);
                }
                pushCode8(0x0f);
                pushCode8(op);
                pushCode8(0xC0 | (from << 3) | m);            
                runTestCPU();

                if (cpu->reg[m].u32!=result) {
                    failed("sse failed");
                }
            }
        }
        if (memResult!=0xFFFFFFFF) {
            initSseTest();         
            memory->writed(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16, value1);
            loadSSE(from, 0, value2l, value2h);
            if (preOp1) {
                pushCode8(preOp1);
            }
            if (preOp2) {
                pushCode8(preOp2);
            }
            pushCode8(0x0f);
            pushCode8(op);
            pushCode8(0x04 | (from<<3));
            pushCode8(0x25);
            pushCode32(SSE_MEM_VALUE_TMP_OFFSET+16);
            runTestCPU();
            if (memory->readd(cpu->seg[DS].address+SSE_MEM_VALUE_TMP_OFFSET+16)!=result) {
                failed("sse failed");
            }
        }
    }    
}

void testSseMovUps310() {  
    testSse128(0, 0, 0x10, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSseMovSs310() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x12345678eeff2468l), _mm_set_pi64x(0x24680bdf13579acel));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        movss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movq failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x00000000eeff2468l), _mm_set_pi64x(0x0000000000000000l));
    __asm {
        movups xmm1, d1
        movss xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x10, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x12345678eeff2468l, 0x24680bdf13579acel, 0x000000eeff2468l, 0x0000000000000000l);
}

void testSseMovUps311() {  
    testSse128r(0, 0, 0x11, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSseMovSs311() {
    // 4 byte write when going to memory
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x12345678eeff2468l), _mm_set_pi64x(0x24680bdf13579acel));

    __asm {
        movups xmm1, d
        movss result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movss failed");
    }
#endif 
    testSse128r(0, 0xf3, 0x11, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x12345678eeff2468l, 0x24680bdf13579acel, 0x12345678eeff2468l, 0x24680bdf13579acel);
}

void testSseMovHlps312() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x24680bdf13579acel), _mm_set_pi64x(0x1122334455667788l));

    __asm {
        movups xmm1, d1
        movups xmm0, d2
        movhlps xmm0, xmm1
        movups result, xmm0
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movhlps failed");
    }
#endif 
    testSse128(0, 0, 0x12, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0x24680bdf13579acel, 0x1122334455667788l, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl);
}

void testSseMovLps312() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;

    __asm {
        movups xmm1, d2
        movlps xmm1, d1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlps failed");
    }
#endif 
    testSse128(0, 0, 0x12, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0x1234567890abcdefl, 0x1122334455667788l);
}

void testSseMovLps313() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i result = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x1122334455667788l));

    __asm {
        movups xmm1, d1
        movlps result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlps failed");
    }
#endif 
    testSse128r(0, 0, 0x13, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0x1234567890abcdefl, 0x1122334455667788l);
}

void testSseUnpcklps314() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x90abcdefeeff2468), _mm_set_pi64x(0x12345678aabbccdd));
    __m128i result;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        unpcklps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlps failed");
    }
#endif 
    testSse128(0, 0, 0x14, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0x90abcdefeeff2468, 0x12345678aabbccdd);
}

void testSseUnpckhps315() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x13579ace55667788), _mm_set_pi64x(0x24680bdf11223344));
    __m128i result;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        unpckhps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlps failed");
    }
#endif 
    testSse128(0, 0, 0x15, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0x13579ace55667788, 0x24680bdf11223344);
}

void testSseMovlhps316() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1234567890abcdefl));
    __m128i result;

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        movlhps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("movlps failed");
    }
#endif 
    testSse128(0, 0, 0x16, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1234567890abcdefl, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl);
}

void testSseMovhps316() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1234567890abcdefl));
    __m128i result;

    __asm {
        movups xmm1, d1
        movhps xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Movhps failed");
    }
#endif 
    testSse128(0, 0, 0x16, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0xaabbccddeeff2468l, 0x1234567890abcdefl);
}

void testSseMovhps317() {
    #if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x24680bdf13579ace), _mm_set_pi64x(0x1122334455667788));

    __asm {
        movups xmm1, d2
        movhps d1, xmm1
        emms
    }
    if (memcmp(&d1, &expected, 16)) {
        failed("Movhps failed");
    }
#endif 
    testSse128r(0, 0, 0x17, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, 0x24680bdf13579ace, 0x1122334455667788);
}

void testSseMovaps328() {
    testSse128(0, 0, 0x28, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSseMovaps329() {
    testSse128r(0, 0, 0x29, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSseCvtpi2ps32a() {
    U64 d2 = ((U64)((U32)(-5000)) << 32) | 5000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));    
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xc59c4000459c4000), _mm_set_pi64x(0x1122334455667788));
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtpi2ps xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cvtpi2ps failed");
    }
#endif 
    testSseMmx64(0, 0, 0x2a, 0xaabbccddeeff2468l, 0x1122334455667788l, d2, 0xc59c4000459c4000, 0x1122334455667788);
}

void testSseCvtsi2ss32a() {
    U32 d2 = (U32)(-5000);
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));    
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddc59c4000), _mm_set_pi64x(0x1122334455667788));
    __m128i result;

    __asm {
        movups xmm1, d1
        cvtsi2ss xmm1, d2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cvtsi2ss failed");
    }
#endif 
    testSseReg32(0, 0xF3, 0x2a, 0xaabbccddeeff2468l, 0x1122334455667788l, d2, 0xaabbccddc59c4000, 0x1122334455667788);
}

void testSseMovntps32b() {
    testSse128r(0, 0, 0x2b, SSE_MEM_VALUE128_DEFAULT1, SSE_MEM_VALUE128_DEFAULT2, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH, 0xFFFFFFFFFFFFFFFFl, 0xFFFFFFFFFFFFFFFFl, SSE_MEM_VALUE128_LOW, SSE_MEM_VALUE128_HIGH);
}

void testSseCvttps2pi32c() {
    Test_Float f1;
    Test_Float f2;

    f1.f = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.f = -5000.6f; // should tuncate

    U64 expected = 0xffffec7880000000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(f1.i, f2.i, 1, 2);        
    U64 result;

    __asm {
        movups xmm1, d1
        cvttps2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result!=expected) {
        failed("Cvttps2pi failed");
    }
#endif 
    testSseMmx64r(0, 0, 0x2c, 0, (((U64)f2.i) << 32) | (U64)f1.i, ((U64)2) << 32 | 1, expected);

    f1.f = -12345678900.0f; // should result in indefinite integer, 80000000
    f2.f = 5000.6f; // should tuncate

    expected = 0x0000138880000000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    d1 = _mm_setr_epi32(f1.i, f2.i, 1, 2);
    result;

    __asm {
        movups xmm1, d1
        cvttps2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result != expected) {
        failed("Cvttps2pi failed");
    }
#endif 
    testSseMmx64r(0, 0, 0x2c, 0, (((U64)f2.i) << 32) | (U64)f1.i, ((U64)2) << 32 | 1, expected);
}

void testSseCvttss2si32c() {
    Test_Float f1;
    Test_Float f2;
    Test_Float f3;

    f1.f = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.f = -5000.6f; // should truncate
    f3.f = -12345678900.0f; // should result in indefinite integer, 80000000

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    U32 expected = 0x80000000;
    U32 result;

    __asm {
        movups xmm1, d1
        cvttss2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Cvttss2si failed");
    }

    d1 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = 0xffffec78;

    __asm {
        movups xmm1, d1
        cvttss2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Cvttss2si failed");
    }

    d1 = _mm_setr_epi32(f3.i, 1, 2, 3);
    expected = 0x80000000;

    __asm {
        movups xmm1, d1
        cvttss2si eax, xmm1
        mov result, eax
        emms
    }
    if (result != expected) {
        failed("Cvttss2si failed");
    }
#endif
    testSseReg32r(0, 0xF3, 0x2c, 0, (((U64)3) << 32) | f1.i, ((U64)2) << 32 | 1, 0x80000000);
    testSseReg32r(0, 0xF3, 0x2c, 0, (((U64)3) << 32) | f2.i, ((U64)2) << 32 | 1, 0xffffec78);
    testSseReg32r(0, 0xF3, 0x2c, 0, (((U64)3) << 32) | f3.i, ((U64)2) << 32 | 1, 0x80000000);
}

void testSseCvtps2pi32d() {
    Test_Float f1;
    Test_Float f2;

    f1.f = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.f = -5000.0f; // :TODO: test rounding

    U64 expected = 0xffffec7880000000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(f1.i, f2.i, 1, 2);        
    U64 result;

    __asm {
        movups xmm1, d1
        cvtps2pi mm1, xmm1
        movq result, mm1
        emms
    }
    if (result!=expected) {
        failed("Cvtps2pi failed");
    }
#endif 
    testSseMmx64r(0, 0, 0x2d, 0, ((U64)f2.i << 32) | f1.i, ((U64)2) << 32 | 1, expected);
}

void testSseCvtss2si32d() {
    Test_Float f1;
    Test_Float f2;

    f1.f = 12345678900.0f; // should result in indefinite integer, 80000000
    f2.f = -5000.0f; // :TODO: test rounding

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    U32 expected = 0x80000000;
    U32 result;

    __asm {
        movups xmm1, d1
        cvtss2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Cvtss2si failed");
    }

    d1 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = 0xffffec78;

    __asm {
        movups xmm1, d1
        cvttss2si eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Cvtss2si failed");
    }
#endif
    testSseReg32r(0, 0xF3, 0x2d, 0,  (((U64)3) << 32) | f1.i, ((U64)2) << 32 | 1, 0x80000000);
    testSseReg32r(0, 0xF3, 0x2d, 0,  (((U64)3) << 32) | f2.i, ((U64)2) << 32 | 1, 0xffffec78);
}

// :TODO: test exceptions?
void testSseUcomiss32e() {
    Test_Float f1;
    Test_Float f2;

    f1.f = 1;
    f2.f = 2;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    __m128i d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    U32 result;
    U32 expected = CF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Ucomiss failed");
    }

    f1.f = -1;
    f2.f = -1;

    d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = ZF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Ucomiss failed");
    }

    f1.f = 1;
    f2.f = -1;

    d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = 0;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Ucomiss failed");
    }

    f1.i = 0x7fc00000;
    f2.f = -1;

    d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = ZF | PF | CF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        ucomiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Ucomiss failed");
    }
#endif
    f1.f = 1;
    f2.f = 2;
    testSse128f(0, 0, 0x2e, f1.i, 0, f2.i, 0, CF);
    f1.f = -1;
    f2.f = -1;
    testSse128f(0, 0, 0x2e, f1.i, 0, f2.i, 0, ZF);
    f1.f = 1;
    f2.f = -1;
    testSse128f(0, 0, 0x2e, f1.i, 0, f2.i, 0, 0);
    f1.i = 0x7fc00000;
    f2.f = -1;
    testSse128f(0, 0, 0x2e, f1.i, 0, f2.i, 0, ZF | PF | CF);
}

void testSseComiss32f() {
    Test_Float f1;
    Test_Float f2;

    f1.f = 1;
    f2.f = 2;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    __m128i d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    __m128i d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    U32 result;
    U32 expected = CF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        comiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Comiss failed");
    }

    f1.f = -1;
    f2.f = -1;

    d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = ZF;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        comiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Comiss failed");
    }

    f1.f = 1;
    f2.f = -1;

    d1 = _mm_setr_epi32(f1.i, 1, 2, 3);    
    d2 = _mm_setr_epi32(f2.i, 1, 2, 3);    
    expected = 0;

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        comiss xmm1, xmm2
        lahf
        movzx eax, ah
        mov result, eax
        emms
    }
    result &= FLAG_MASK;
    if (result!=expected) {
        failed("Comiss failed");
    }
#endif
    f1.f = 1;
    f2.f = 2;
    testSse128f(0, 0, 0x2f, f1.i, 0, f2.i, 0, CF);
    f1.f = -1;
    f2.f = -1;
    testSse128f(0, 0, 0x2f, f1.i, 0, f2.i, 0, ZF);
    f1.f = 1;
    f2.f = -1;
    testSse128f(0, 0, 0x2f, f1.i, 0, f2.i, 0, 0);
}

void testSseMovmskps350() {
    Test_Float f1;
    Test_Float f2;
    Test_Float f3;
    Test_Float f4;

    f1.f = 0x7fc00000; // QNan
    f2.f = -5000.0f;
    f3.f = 5000.0f;
    f4.f = 0.0f;
    
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i d1 = _mm_setr_epi32(f1.i, f2.i, f3.i, f4.i);
    U32 expected = 2;
    U32 result;

    __asm {
        movups xmm1, d1
        movmskps eax, xmm1
        mov result, eax
        emms
    }
    if (result!=expected) {
        failed("Movmskps failed");
    }
#endif
    testSseReg32r(0, 0, 0x50, 0, 0xc59c40004eff8000l, 0x00000000459c4000l, 2, 0xFFFFFFFF);
}

void testSseSqrtps351() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected = floatTo128(0.0f, 2.0f, 16.0f, sqrtf(2.0));
    __m128i result;
    __m128i d1;
    __m128i d2 = floatTo128(0.0f, 4.0f, 256.0f, 2.0);

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        sqrtps xmm1, xmm2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Sqrtps failed");
    }
#endif
    testSse128(0, 0, 0x51, 0, 0, 0x4080000000000000l, 0x4000000043800000, 0x4000000000000000l, 0x3fb504f341800000l);
}

void testSseSqrtss351() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected = floatTo128(16.0f, 2.0f, 0.0f, 4.0f);
    __m128i result;
    __m128i d1 = floatTo128(256.0f, 2.0f, 0.0f, 4.0);

    __asm {
        movups xmm1, d1
        sqrtss xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Sqrtss failed");
    }
#endif
    testSse128(0, 0xf3, 0x51, 0x4000000043800000l, 0x4080000000000000l, 0x4000000043800000l, 0x4080000000000000l, 0x4000000041800000l, 0x4080000000000000l);
}

void testSseRsqrtps352() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x3efff0007f800000l), _mm_set_pi64x(0x3f34f8003d7ff000l));
    __m128i result;
    __m128i d1;
    __m128i d2 = floatTo128(0.0f, 4.0f, 256.0f, 2.0);

    __asm {
        movups xmm1, d1
        movups xmm2, d2
        rsqrtps xmm1, xmm2
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Rsqrtps failed");
    }
#endif
    // :TODO: not exact match
#ifdef BOXEDWINE_X64
    testSse128(0, 0, 0x52, 0x4080000000000000, 0x4000000043800000, 0x4080000000000000l, 0x4000000043800000, 0x3efff0007f800000l, 0x3f34f8003d7ff000);
#else
    // :TODO: for some reason this doesn't work on Linux
    // testSse128(0, 0, 0x52, 0x4080000000000000, 0x4000000043800000, 0x4080000000000000l, 0x4000000043800000, 0x3f0000007f800000, 0x3f3504f33d800000);
#endif
}

void testSseRsqrtss352() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x411000003efff000), _mm_set_pi64x(0x4000000043800000));
    __m128i result;
    __m128i d1 = floatTo128(4.0f, 9.0f, 256.0f, 2.0);

    __asm {
        movups xmm1, d1
        rsqrtss xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Rsqrtss failed");
    }
#endif
    // :TODO: not exact match
#ifdef BOXEDWINE_X64 
    testSse128(0, 0xf3, 0x52, 0x4110000040800000, 0x4000000043800000, 0x4110000040800000, 0x4000000043800000, 0x411000003efff000, 0x4000000043800000);
#elif defined(BOXEDWINE_BINARY_TRANSLATOR)
    testSse128(0, 0xf3, 0x52, 0x4110000040800000, 0x4000000043800000, 0x4110000040800000, 0x4000000043800000, 0x411000003eff8000, 0x4000000043800000);
#else
    testSse128(0, 0xf3, 0x52, 0x4110000040800000, 0x4000000043800000, 0x4110000040800000, 0x4000000043800000, 0x411000003effc988, 0x4000000043800000);
#endif
}

void testSseRcpps353() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x3de380003e7ff000), _mm_set_pi64x(0x412000003b7ff000));
    __m128i result;
    __m128i d1 = floatTo128(4.0f, 9.0f, 256.0f, 0.1f);

    __asm {
        movups xmm1, d1
        rcpps xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Rcpps failed");
    }
#endif
    // :TODO: not exact match
#ifdef BOXEDWINE_X64
    testSse128(0, 0, 0x53, 0x4110000040800000, 0x3dcccccd43800000, 0x4110000040800000, 0x3dcccccd43800000, 0x3de380003e7ff000, 0x412000003b7ff000);
#elif defined(BOXEDWINE_BINARY_TRANSLATOR)
    testSse128(0, 0, 0x53, 0x4110000040800000, 0x3dcccccd43800000, 0x4110000040800000, 0x3dcccccd43800000, 0x3de300003e7f8000, 0x412000003b7f8000);
#else
    testSse128(0, 0, 0x53, 0x4110000040800000, 0x3dcccccd43800000, 0x4110000040800000, 0x3dcccccd43800000, 0x3de38df43e7f58cc, 0x411fc11d3b7f58cc);
#endif
}

void testSseRcpss353() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)        
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x411000003e7ff000), _mm_set_pi64x(0x3dcccccd43800000));
    __m128i result;
    __m128i d1 = floatTo128(4.0f, 9.0f, 256.0f, 0.1f);

    __asm {
        movups xmm1, d1
        rcpss xmm1, xmm1
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Rcpps failed");
    }
#endif
    // :TODO: not exact match
#ifdef BOXEDWINE_X64
    testSse128(0, 0xf3, 0x53, 0x4110000040800000, 0x3dcccccd43800000, 0x4110000040800000, 0x3dcccccd43800000, 0x411000003e7ff000, 0x3dcccccd43800000);
#elif defined(BOXEDWINE_BINARY_TRANSLATOR)
    testSse128(0, 0xf3, 0x53, 0x4110000040800000, 0x3dcccccd43800000, 0x4110000040800000, 0x3dcccccd43800000, 0x411000003e7f8000, 0x3dcccccd43800000);
#else
    testSse128(0, 0xf3, 0x53, 0x4110000040800000, 0x3dcccccd43800000, 0x4110000040800000, 0x3dcccccd43800000, 0x411000003e800000, 0x3dcccccd43800000);
#endif
}

void testSseAndps354() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x0230445880ab0468), _mm_set_pi64x(0x0020034411461288));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        andps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Andps failed");
    }
#endif 
    testSse128(0, 0, 0x54, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0x0230445880ab0468, 0x0020034411461288);
}

void testSseAndnps355() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xa88b88856e542000), _mm_set_pi64x(0x1102300044206500));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        andnps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Andnps failed");
    }
#endif 
    testSse128(0, 0, 0x55, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xa88b88856e542000, 0x1102300044206500);
}

void testSseOrps356() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xbabfdefdfeffedef), _mm_set_pi64x(0x356a3bdf5777ffce));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        orps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Orps failed");
    }
#endif 
    testSse128(0, 0, 0x56, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xbabfdefdfeffedef, 0x356a3bdf5777ffce);
}

void testSseXorps357() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi64(_mm_set_pi64x(0x1234567890abcdefl), _mm_set_pi64x(0x24680bdf13579acel));
    __m128i d2 = _mm_setr_epi64(_mm_set_pi64x(0xaabbccddeeff2468l), _mm_set_pi64x(0x1122334455667788l));
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0xb88f9aa57e54e987), _mm_set_pi64x(0x354a389b4631ed46));

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        xorps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Xorps failed");
    }
#endif 
    testSse128(0, 0, 0x57, 0x1234567890abcdefl, 0x24680bdf13579acel, 0xaabbccddeeff2468l, 0x1122334455667788l, 0xb88f9aa57e54e987, 0x354a389b4631ed46);
}

void testSseAddps358() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-4.0f, 10.0f, -5.5, 1.0f);
    __m128i result;
    __m128i expected = floatTo128(0.0f, 19.0f, -10.5, 0.9f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        addps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Addps failed");
    }
#endif 
    testSse128(0, 0, 0x58, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0800000, 0x3f800000c0b00000, 0x4198000000000000, 0x3f666666c1280000);
}

void testSseAddss358() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 1.0f);
    __m128i result;
    __m128i expected = floatTo128(-1.0f, 9.0f, -5.0, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        addss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Addss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x58, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x3f800000c0b00000, 0x41100000bf800000, 0xbdcccccdc0a00000);
}

void testSseMulps359() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(-20.0f, 90.0f, 27.5f, -0.2f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        mulps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Mulps failed");
    }
#endif 
    testSse128(0, 0, 0x59, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0x42b40000c1a00000, 0xbe4ccccd41dc0000);
}

void testSseMulss359() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(-20.0f, 9.0f, -5.0, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        mulss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Mulss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x59, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0x41100000c1a00000, 0xbdcccccdc0a00000);
}

void testSseSubps35c() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(9.0f, -1.0f, 0.5f, -2.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        subps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Subps failed");
    }
#endif 
    testSse128(0, 0, 0x5c, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0xbf80000041100000, 0xc00666663f000000);
}

void testSseSubss35c() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(9.0f, 9.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        subss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Subss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x5c, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0x4110000041100000, 0xbdcccccdc0a00000);
}

void testSseMinps35d() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(-5.0f, 9.0f, -5.5f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minps failed");
    }
    d1 = floatTo128(10.0f, -7.0f, -0.0f, 0.0f);
    d1.m128i_u32[0] = FLOAT_QUIET_NAN_BITS;
    d2 = floatTo128(17.0f, 10.0f, 0.0f, -0.0f);
    d2.m128i_u32[1] = FLOAT_QUIET_NAN_BITS;
    expected = floatTo128(17.0f, 9.0f, 0.0f, -0.0f);
    expected.m128i_u32[1] = FLOAT_QUIET_NAN_BITS;
    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minps failed");
    }
#endif 
    testSse128(0, 0, 0x5d, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0x41100000c0a00000, 0xbdcccccdc0b00000);
    testSse128(0, 0, 0x5d, 0xc0e000007fc00000, 0x0000000080000000, 0x7fc0000041880000, 0x8000000000000000, 0x7fc0000041880000, 0x8000000000000000);
}

void testSseMinss35d() {
    Test_Float fNan;
    Test_Float fZero;
    Test_Float fNegZero;

    fNan.i = FLOAT_QUIET_NAN_BITS;
    fZero.f = 0.0;
    fNegZero.f = -0.0;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(-5.0f, 9.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minss failed");
    }
    d1 = floatTo128(fNan.f, 9.0f, -5.0, -0.1f);
    d2 = floatTo128(-5.0f, 10.0f, -5.5, 2.0f);
    expected = floatTo128(-5.0f, 9.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minss failed");
    }
    d1 = floatTo128(4.0f, 9.0f, -5.0, -0.1f);
    d2 = floatTo128(fNan.f, 10.0f, -5.5, 2.0f);
    expected = floatTo128(fNan.f, 9.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minss failed");
    }
    d1 = floatTo128(fZero.f, 9.0f, -5.0, -0.1f);
    d2 = floatTo128(fNegZero.f, 10.0f, -5.5, 2.0f);
    expected = floatTo128(fNegZero.f, 9.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minss failed");
    }
    d1 = floatTo128(fNegZero.f, 9.0f, -5.0, -0.1f);
    d2 = floatTo128(fZero.f, 10.0f, -5.5, 2.0f);
    expected = floatTo128(fZero.f, 9.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        minss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Minss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x5d, 0x4110000040800000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0x41100000c0a00000, 0xbdcccccdc0a00000);

    testSse128(0, 0xf3, 0x5d, 0x411000007fc00000, 0xbdcccccdc0a00000, 0x41200000c0a00000, 0x40000000c0b00000, 0x41100000c0a00000, 0xbdcccccdc0a00000);
    testSse128(0, 0xf3, 0x5d, 0x4110000040800000, 0xbdcccccdc0a00000, 0x412000007fc00000, 0x40000000c0b00000, 0x411000007fc00000, 0xbdcccccdc0a00000);
    testSse128(0, 0xf3, 0x5d, 0x4110000000000000, 0xbdcccccdc0a00000, 0x4120000080000000, 0x40000000c0b00000, 0x4110000080000000, 0xbdcccccdc0a00000);
    testSse128(0, 0xf3, 0x5d, 0x4110000080000000, 0xbdcccccdc0a00000, 0x4120000000000000, 0x40000000c0b00000, 0x4110000000000000, 0xbdcccccdc0a00000);
}

void testSseDivps35e() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 5.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-8.0f, 20.0f, -5.0, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(-0.5f, 0.25f, 1.0f, -0.05f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        divps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Divps failed");
    }
#endif 
    testSse128(0, 0, 0x5e, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x3e800000bf000000, 0xbd4ccccd3f800000);
}

void testSseDivss35e() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 5.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-8.0f, 20.0f, -5.0, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(-0.5f, 5.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        divss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Divss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x5e, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a00000bf000000, 0xbdcccccdc0a00000);
}

void testSseMaxps35f() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 5.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-8.0f, 20.0f, -5.0, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(4.0f, 20.0f, -5.0f, 2.0f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        maxps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Maxps failed");
    }
#endif 
    testSse128(0, 0, 0x5f, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x41a0000040800000, 0x40000000c0a00000);
}

void testSseMaxss35f() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = floatTo128(4.0f, 5.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-8.0f, 20.0f, -5.0, 2.0f);
    __m128i result;
    __m128i expected = floatTo128(4.0f, 5.0f, -5.0f, -0.1f);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        maxss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Maxss failed");
    }
#endif 
    testSse128(0, 0xf3, 0x5f, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a0000040800000, 0xbdcccccdc0a00000);
}

void testSsePshufw370() {
    U64 d1 = 0x1111222233334444l;
    U64 d2 = 0x5555666677778888l;
    U64 expected = 0x8888777755556666;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pshufw mm0, mm1, 0x1E
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64imm8(0, 0x70, d1, d2, expected, 0x1E);
}

void testCmpps0x3c2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    __m128i d1 = floatTo128(4.0f, 5.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-8.0f, 20.0f, -5.0, 2.0f);
    __m128i result;
    __m128i expected = _mm_setr_epi32(0, 0, -1, 0);

     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpeqps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(0, -1, 0, -1);
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpltps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(0, -1, -1, -1);
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpleps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(-1, -1, 0, -1);
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpneqps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(-1, 0, -1, 0);
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnltps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(-1, 0, 0, 0);
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnleps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(0, -1, 0, 0);
    d1.m128i_i32[1] = FLOAT_QUIET_NAN_BITS;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpunordps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi32(-1, 0, -1, -1);
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpordps xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }
#endif
    // cmpeqps
    testSse128imm(0, 0, 0xc2, 0, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x0000000000000000, 0x00000000ffffffff);
    // cmpltps
    testSse128imm(0, 0, 0xc2, 1, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0xffffffff00000000, 0xffffffff00000000);
    // cmpleps
    testSse128imm(0, 0, 0xc2, 2, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0xffffffff00000000, 0xffffffffffffffff);
    // cmpunordps
    testSse128imm(0, 0, 0xc2, 3, 0x7fd0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0xffffffff00000000, 0x0000000000000000);
    // cmpneqps
    testSse128imm(0, 0, 0xc2, 4, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0xffffffffffffffff, 0xffffffff00000000);
    // cmpnltps
    testSse128imm(0, 0, 0xc2, 5, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x00000000ffffffff, 0x00000000ffffffff);
    // cmpnleps
    testSse128imm(0, 0, 0xc2, 6, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x00000000ffffffff, 0x0000000000000000);
    // cmpordps
    testSse128imm(0, 0, 0xc2, 7, 0x7fd0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x00000000ffffffff, 0xffffffffffffffff);
}

void testCmpss0x3c2() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    __m128i d1 = floatTo128(4.0f, 5.0f, -5.0, -0.1f);
    __m128i d2 = floatTo128(-8.0f, 20.0f, -5.0, 2.0f);
    __m128i result;
    __m128i expected = _mm_setr_epi64(_mm_set_pi64x(0x40a0000000000000), _mm_set_pi64x(0xbdcccccdc0a00000));

     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpeqss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x40a0000000000000), _mm_set_pi64x(0xbdcccccdc0a00000));
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpltss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x40a0000000000000), _mm_set_pi64x(0xbdcccccdc0a00000));
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpless xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x40a00000ffffffff), _mm_set_pi64x(0xbdcccccdc0a00000));
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpneqss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x40a00000ffffffff), _mm_set_pi64x(0xbdcccccdc0a00000));
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnltss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x40a00000ffffffff), _mm_set_pi64x(0xbdcccccdc0a00000));
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpnless xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x7fc0000000000000), _mm_set_pi64x(0xbdcccccdc0a00000));
    d1.m128i_i32[1] = FLOAT_QUIET_NAN_BITS;
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpunordss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }

    expected = _mm_setr_epi64(_mm_set_pi64x(0x7fc00000ffffffff), _mm_set_pi64x(0xbdcccccdc0a00000));
     __asm {
        movups xmm0, d2
        movups xmm1, d1
        cmpordss xmm1, xmm0
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Cmpps failed");
    }
#endif
    // cmpeqss
    testSse128imm(0, 0xf3, 0xc2, 0, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a0000000000000, 0xbdcccccdc0a00000);
    // cmpltss
    testSse128imm(0, 0xf3, 0xc2, 1, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a0000000000000, 0xbdcccccdc0a00000);
    // cmpless
    testSse128imm(0, 0xf3, 0xc2, 2, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a0000000000000, 0xbdcccccdc0a00000);
    // cmpunordss
    testSse128imm(0, 0xf3, 0xc2, 3, 0x7fd0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x7fd0000000000000, 0xbdcccccdc0a00000);
    // cmpneqss
    testSse128imm(0, 0xf3, 0xc2, 4, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a00000ffffffff, 0xbdcccccdc0a00000);
    // cmpnltss
    testSse128imm(0, 0xf3, 0xc2, 5, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a00000ffffffff, 0xbdcccccdc0a00000);
    // cmpnless
    testSse128imm(0, 0xf3, 0xc2, 6, 0x40a0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x40a00000ffffffff, 0xbdcccccdc0a00000);
    // cmpordss
    testSse128imm(0, 0xf3, 0xc2, 7, 0x7fd0000040800000, 0xbdcccccdc0a00000, 0x41a00000c1000000, 0x40000000c0a00000, 0x7fd00000ffffffff, 0xbdcccccdc0a00000);
}

void testPinsrw3c4() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    U64 result;
    U64 d1 = 0x1111222233334444l;
    U32 reg = 0x5555;
    U64 expected = 0x1111555533334444l;

     __asm {
        __asm movq mm0, d1
        __asm mov ecx, reg
        __asm pinsrw mm0, ecx, 2
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64Eimm8(0, 0xc4, 0x1111222233334444l, 0x5555, 0x1111555533334444l, 2);
}

void testPextrw3c5() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    U32 result;
    U64 d1 = 0x1111222233334444l;
    U32 reg = 0x12345678;
    U32 expected = 0x2222;

     __asm {
        __asm movq mm0, d1
        __asm mov ecx, reg
        __asm pextrw ecx, mm0, 2
        __asm mov result, ecx
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testRegMmx64imm8(0, 0xc5, 0x12345678, 0x1111222233334444l, 0x2222, 2);
}

void testPextrw1c5() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)  
    __m128i d1 = _mm_setr_epi32(0x11112222, 0x33334444, 0x55556666, 0x77778888);
    U32 result;
    U32 reg = 0x12345678;
    U32 expected = 0x4444;

    __asm {
        movups xmm0, d1
        mov ecx, reg
        pextrw ecx, xmm0, 2
        mov result, ecx
        emms
    }
    if (result != expected) {
        failed("failed");
    }
#endif
    testRegSseImm8(0x66, 0xc5, 0x1111222233334444l, 0x5555666677778888, 0x12345678, 0x2222, 2);
}

void testShufps3c6() {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    __m128i d1 = _mm_setr_epi32(0x11111111, 0x22222222, 0x33333333, 0x44444444);
    __m128i d2 = _mm_setr_epi32(0x55555555, 0x66666666, 0x77777777, 0x88888888);
    __m128i result;
    __m128i expected = _mm_setr_epi32(0x33333333, 0x44444444, 0x66666666, 0x55555555);

    __asm {
        movups xmm0, d2
        movups xmm1, d1
        shufps xmm1, xmm0, 0x1E
        movups result, xmm1
        emms
    }
    if (memcmp(&result, &expected, 16)) {
        failed("Shufps failed");
    }
#endif 
    testSse128imm(0, 0, 0xc6, 0x1E, 0x2222222211111111, 0x4444444433333333, 0x6666666655555555, 0x8888888877777777, 0x4444444433333333, 0x5555555566666666);
    testSse128imm(0, 0, 0xc6, 1 | 0 | 1 << 4 | 3 << 6 , 0x2222222211111111, 0x4444444433333333, 0x6666666655555555, 0x8888888866666666, 0x1111111122222222, 0x8888888866666666);
}

void testPmovmskb3d7() {
    U64 d1 = 0x1122804455667788l;
    U32 reg = 0x12345678;
    U32 expected = 0x21;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U32 result;    

     __asm {
        __asm movq mm0, d1
        __asm mov ecx, reg
        __asm pmovmskb ecx, mm0
        __asm mov result, ecx
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64Reg(0xd7, d1, reg, expected);
}

void testPminub3da() {
    U64 d1 = 0x1122804455667788l;
    U64 d2 = 0x2211704377ff3211l;
    U64 expected = 0x1111704355663211l;    
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pminub mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xda, d1, d2, expected);
}

void testPmaxub3de() {
    U64 d1 = 0x1122804455667788l;
    U64 d2 = 0x2211704377ff3211l;
    U64 expected = 0x2222804477ff7788l;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pmaxub mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xde, d1, d2, expected);
}

// :TODO: rounding
void testPavgb3e0() {
    U64 d1 = 0x1122804455667788l;
    U64 d2 = 0x2312704277fe3112l;
    U64 expected = 0x1a1a784366b2544d;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pavgb mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xe0, d1, d2, expected);
}

// :TODO: rounding
void testPavgw3e3() {
    U64 d1 = 0x1122804455667788l;
    U64 d2 = 0x2212704277fe3112l;
    U64 expected = 0x199a784366b2544d;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pavgw mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xe3, d1, d2, expected);
}

void testPmulhuw3e4() {
    U64 d1 = 0x1122804455660001l;
    U64 d2 = 0x2212ffff77fe3112l;
    U64 expected = 0x0247804328070000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pmulhuw mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xe4, d1, d2, expected);
}

void testMovntq3e7() {   
    for (U8 m=0;m<8;m++) {
        initMmxTest();      
        loadMMX(m, 1, MMX_MEM_VALUE64);
        memory->writeq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET, 0x1111111111111111l);
        pushCode8(0x0f);
        pushCode8(0xE7);
        pushCode8(0x04 | (m<<3));
        pushCode8(0x25);
        pushCode32(MMX_MEM_VALUE_TMP_OFFSET);
        runTestCPU();
        if (memory->readq(cpu->seg[DS].address+MMX_MEM_VALUE_TMP_OFFSET)!=MMX_MEM_VALUE64) {
            failed("movntq failed");
        }
    }    
}

void testPminsw3ea() {
    U64 d1 = 0x11112345ffff8000;
    U64 d2 = 0x11121234fffe7fff;
    U64 expected = 0x11111234fffe8000;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pminsw mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xea, d1, d2, expected);
}

void testPmaxsw3ee() {
    U64 d1 = 0x11112345ffff8000;
    U64 d2 = 0x11121234fffe7fff;
    U64 expected = 0x11122345ffff7fff;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm pmaxsw mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xee, d1, d2, expected);
}

void testPsadbw3f6() {
    U64 d1 = 0x12112345ffff8000;
    U64 d2 = 0x11121234fffe7fff;
    U64 expected = 0x0000000000000125;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    U64 result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm psadbw mm0, mm1
        __asm movq result, mm0
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    testMmx64(0xf6, d1, d2, expected);
}

void testMaskmovq3f7() {
    U64 d1 = 0x1122334455667788;
    U64 d2 = 0x807fff00808001f0;
    U64 expected = 0x1199339955669988;
    U64 result = 0x9999999999999999;
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)    
    U64* pResult = &result;

     __asm {
        __asm movq mm0, d1
        __asm movq mm1, d2
        __asm mov edi, pResult
        __asm maskmovq mm0, mm1
        __asm emms
    }
    if (result!=expected) {
        failed("failed");
    }
#endif
    for (U8 m=0;m<8;m++) {
        for (U8 from=0;from<8;from++) {
            if (m==from) {
                continue;
            }            
            initMmxTest();            
            loadMMX(m, 0, d1);
            loadMMX(from, 1, d2);
            EDI = MMX_MEM_VALUE_TMP_OFFSET+64;
            memory->writeq(cpu->seg[DS].address+EDI, 0x9999999999999999);
            pushCode8(0x0f);
            pushCode8(0xf7);
            pushCode8(0xC0 | (m << 3) | from);            
            runTestCPU();
            result = memory->readq(cpu->seg[DS].address+EDI);
            if (result!=expected) {
                failed("maskmovq failed");
            }
        }
    } 
}

#endif

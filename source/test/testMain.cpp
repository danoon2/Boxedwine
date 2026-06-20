/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include <stdio.h>
#include <stdlib.h>

#include "cpu/testAdc.h"
#include "cpu/testAdd.h"
#include "cpu/testAnd.h"
#include "cpu/testAsmJit.h"
#include "cpu/testBCD.h"
#include "cpu/testBit.h"
#include "cpu/testCallRet.h"
#include "cpu/testCMov.h"
#include "cpu/testCmp.h"
#include "cpu/testCPU.h"
#include "cpu/testFPU.h"
#include "cpu/testIncDec.h"
#include "cpu/testJmp.h"
#include "cpu/testLoadSeg.h"
#include "cpu/testLoop.h"
#include "cpu/testMisc.h"
#include "cpu/testMMX.h"
#include "cpu/testMov.h"
#include "cpu/testMulDiv.h"
#include "cpu/testNegNot.h"
#include "cpu/testOr.h"
#include "cpu/testPushPop.h"
#include "cpu/testPushPopSeg.h"
#include "cpu/testSbb.h"
#include "cpu/testSegPrefix.h"
#include "cpu/testSet.h"
#include "cpu/testShift.h"
#include "cpu/testSSE.h"
#include "cpu/testString.h"
#include "cpu/testSub.h"
#include "cpu/testTest.h"
#include "cpu/testX86Binary.h"
#include "cpu/testX86Util.h"
#include "cpu/testXchg.h"
#include "cpu/testXor.h"
#include "devs/testDspAudio.h"
#include "mmu/testSelfModifying.h"

void testWaitPid();
void testHardLinksShareIdentityDataAndXattrs();
void testReadDirectoryReturnsIsDir();
void testUtimensatPreservesAccessTimeInStat();
void testFutimensPreservesAccessTimeInFstat();
void testFutimensTime64SignExtendedSecondsPreservesAccessTimeInFstat();
void testDirectoryReparseSidecarReplacedAfterRemoveAndRecreate();
void testDotDotAfterDotResolvesToParentDirectory();
void testUtf8NamesSurviveNativeFilesystemReload();
void testTrailingDotNamesCanBeUnlinked();
void testDirectorySeekCanStoreOpaquePosition();
void testStartupArgsDefaultUtf8LocaleEnvironment();

namespace {

int totalFails = 0;

const TestEntry TEST_ENTRIES[] = {
    {testDspAudioWriteMath, "Test DSP Audio Write Math"},
    {testMemoryAccess32, "Test 32-bit Memory Access"},
    {testMemoryAccess16, "Test 16-bit Memory Access"},
    {testAddR8R8_0x000, "Test Add R8,R8 000"},
    {testAddE8R8_0x000, "Test Add E8,R8 000"},
    {testAddR16R16_0x001, "Test Add R16,R16 001"},
    {testAddE16R16_0x001, "Test Add E16,R16 001"},
    {testAddR32R32_0x001, "Test Add R32,R32 001"},
    {testAddE32R32_0x001, "Test Add E32,R32 001"},
    {testAddR8R8_0x002, "Test Add R8,R8 002"},
    {testAddR8E8_0x002, "Test Add R8,E8 002"},
    {testAddR16R16_0x003, "Test Add R16,R16 003"},
    {testAddR16E16_0x003, "Test Add R16,E16 003"},
    {testAddR32R32_0x003, "Test Add R32,R32 003"},
    {testAddR32E32_0x003, "Test Add R32,E32 003"},
    {testAddAlIb_0x004, "Test Add AL,Ib 004"},
    {testAddAxIw_0x005, "Test Add AX,Iw 005"},
    {testAddEaxId_0x005, "Test Add EAX,Id 005"},
    {testPushEs_0x006, "Test Push ES 006"},
    {testPushEs_0x206, "Test Push ES 206"},
    {testPopEs_0x007, "Test Pop ES 007"},
    {testPopEs_0x207, "Test Pop ES 207"},
    {testOrR8R8_0x008, "Test Or R8,R8 008"},
    {testOrE8R8_0x008, "Test Or E8,R8 008"},
    {testOrR16R16_0x009, "Test Or R16,R16 009"},
    {testOrE16R16_0x009, "Test Or E16,R16 009"},
    {testOrR32R32_0x009, "Test Or R32,R32 009"},
    {testOrE32R32_0x009, "Test Or E32,R32 009"},
    {testOrR8R8_0x00a, "Test Or R8,R8 00a"},
    {testOrR8E8_0x00a, "Test Or R8,E8 00a"},
    {testOrR16R16_0x00b, "Test Or R16,R16 00b"},
    {testOrR16E16_0x00b, "Test Or R16,E16 00b"},
    {testOrR32R32_0x00b, "Test Or R32,R32 00b"},
    {testOrR32E32_0x00b, "Test Or R32,E32 00b"},
    {testOrAlIb_0x00c, "Test Or AL,Ib 00c"},
    {testOrAxIw_0x00d, "Test Or AX,Iw 00d"},
    {testOrEaxId_0x00d, "Test Or EAX,Id 00d"},
    {testPushCs_0x00e, "Test Push CS 00e"},
    {testPushCs_0x20e, "Test Push CS 20e"},
    {testAdcR8R8_0x010, "Test Adc R8,R8 010"},
    {testAdcE8R8_0x010, "Test Adc E8,R8 010"},
    {testAdcR16R16_0x011, "Test Adc R16,R16 011"},
    {testAdcE16R16_0x011, "Test Adc E16,R16 011"},
    {testAdcR32R32_0x011, "Test Adc R32,R32 011"},
    {testAdcE32R32_0x011, "Test Adc E32,R32 011"},
    {testAdcR8R8_0x012, "Test Adc R8,R8 012"},
    {testAdcR8E8_0x012, "Test Adc R8,E8 012"},
    {testAdcR16R16_0x013, "Test Adc R16,R16 013"},
    {testAdcR16E16_0x013, "Test Adc R16,E16 013"},
    {testAdcR32R32_0x013, "Test Adc R32,R32 013"},
    {testAdcR32E32_0x013, "Test Adc R32,E32 013"},
    {testAdcAlIb_0x014, "Test Adc AL,Ib 014"},
    {testAdcAxIw_0x015, "Test Adc AX,Iw 015"},
    {testAdcEaxId_0x015, "Test Adc EAX,Id 015"},
    {testPushSs_0x016, "Test Push SS 016"},
    {testPushSs_0x216, "Test Push SS 216"},
    {testPopSs_0x017, "Test Pop SS 017"},
    {testPopSs_0x217, "Test Pop SS 217"},
    {testSbbR8R8_0x018, "Test Sbb R8,R8 018"},
    {testSbbE8R8_0x018, "Test Sbb E8,R8 018"},
    {testSbbR16R16_0x019, "Test Sbb R16,R16 019"},
    {testSbbE16R16_0x019, "Test Sbb E16,R16 019"},
    {testSbbR32R32_0x019, "Test Sbb R32,R32 019"},
    {testSbbE32R32_0x019, "Test Sbb E32,R32 019"},
    {testSbbR8R8_0x01a, "Test Sbb R8,R8 01a"},
    {testSbbR8E8_0x01a, "Test Sbb R8,E8 01a"},
    {testSbbR16R16_0x01b, "Test Sbb R16,R16 01b"},
    {testSbbR16E16_0x01b, "Test Sbb R16,E16 01b"},
    {testSbbR32R32_0x01b, "Test Sbb R32,R32 01b"},
    {testSbbR32E32_0x01b, "Test Sbb R32,E32 01b"},
    {testSbbAlIb_0x01c, "Test Sbb AL,Ib 01c"},
    {testSbbAxIw_0x01d, "Test Sbb AX,Iw 01d"},
    {testSbbEaxId_0x01d, "Test Sbb EAX,Id 01d"},
    {testPushDs_0x01e, "Test Push DS 01e"},
    {testPushDs_0x21e, "Test Push DS 21e"},
    {testPopDs_0x01f, "Test Pop DS 01f"},
    {testPopDs_0x21f, "Test Pop DS 21f"},
    {testAndR8R8_0x020, "Test And R8,R8 020"},
    {testAndE8R8_0x020, "Test And E8,R8 020"},
    {testAndR16R16_0x021, "Test And R16,R16 021"},
    {testAndE16R16_0x021, "Test And E16,R16 021"},
    {testAndR32R32_0x021, "Test And R32,R32 021"},
    {testAndE32R32_0x021, "Test And E32,R32 021"},
    {testAndR8R8_0x022, "Test And R8,R8 022"},
    {testAndR8E8_0x022, "Test And R8,E8 022"},
    {testAndR16R16_0x023, "Test And R16,R16 023"},
    {testAndR16E16_0x023, "Test And R16,E16 023"},
    {testAndR32R32_0x023, "Test And R32,R32 023"},
    {testAndR32E32_0x023, "Test And R32,E32 023"},
    {testAndAlIb_0x024, "Test And AL,Ib 024"},
    {testAndAxIw_0x025, "Test And AX,Iw 025"},
    {testAndEaxId_0x025, "Test And EAX,Id 025"},
    {testSegEs_0x026, "Test Seg ES 026"},
    {testSegEs_0x226, "Test Seg ES 226"},
    {testDaa_0x027, "Test DAA 027"},
    {testSubR8R8_0x028, "Test Sub R8,R8 028"},
    {testSubE8R8_0x028, "Test Sub E8,R8 028"},
    {testSubR16R16_0x029, "Test Sub R16,R16 029"},
    {testSubE16R16_0x029, "Test Sub E16,R16 029"},
    {testSubR32R32_0x029, "Test Sub R32,R32 029"},
    {testSubE32R32_0x029, "Test Sub E32,R32 029"},
    {testSubR8R8_0x02a, "Test Sub R8,R8 02a"},
    {testSubR8E8_0x02a, "Test Sub R8,E8 02a"},
    {testSubR16R16_0x02b, "Test Sub R16,R16 02b"},
    {testSubR16E16_0x02b, "Test Sub R16,E16 02b"},
    {testSubR32R32_0x02b, "Test Sub R32,R32 02b"},
    {testSubR32E32_0x02b, "Test Sub R32,E32 02b"},
    {testSubAlIb_0x02c, "Test Sub AL,Ib 02c"},
    {testSubAxIw_0x02d, "Test Sub AX,Iw 02d"},
    {testSubEaxId_0x02d, "Test Sub EAX,Id 02d"},
    {testSegCs_0x02e, "Test Seg CS 02e"},
    {testSegCs_0x22e, "Test Seg CS 22e"},
    {testDas_0x02f, "Test DAS 02f"},
    {testXorR8R8_0x030, "Test Xor R8,R8 030"},
    {testXorE8R8_0x030, "Test Xor E8,R8 030"},
    {testXorR16R16_0x031, "Test Xor R16,R16 031"},
    {testXorE16R16_0x031, "Test Xor E16,R16 031"},
    {testXorR32R32_0x031, "Test Xor R32,R32 031"},
    {testXorE32R32_0x031, "Test Xor E32,R32 031"},
    {testXorR8R8_0x032, "Test Xor R8,R8 032"},
    {testXorR8E8_0x032, "Test Xor R8,E8 032"},
    {testXorR16R16_0x033, "Test Xor R16,R16 033"},
    {testXorR16E16_0x033, "Test Xor R16,E16 033"},
    {testXorR32R32_0x033, "Test Xor R32,R32 033"},
    {testXorR32E32_0x033, "Test Xor R32,E32 033"},
    {testXorAlIb_0x034, "Test Xor AL,Ib 034"},
    {testXorAxIw_0x035, "Test Xor AX,Iw 035"},
    {testXorEaxId_0x035, "Test Xor EAX,Id 035"},
    {testSegSs_0x036, "Test Seg SS 036"},
    {testSegSs_0x236, "Test Seg SS 236"},
    {testAaa_0x037, "Test AAA 037"},
    {testCmpR8R8_0x038, "Test Cmp R8,R8 038"},
    {testCmpE8R8_0x038, "Test Cmp E8,R8 038"},
    {testCmpR16R16_0x039, "Test Cmp R16,R16 039"},
    {testCmpE16R16_0x039, "Test Cmp E16,R16 039"},
    {testCmpR32R32_0x039, "Test Cmp R32,R32 039"},
    {testCmpE32R32_0x039, "Test Cmp E32,R32 039"},
    {testCmpR8R8_0x03a, "Test Cmp R8,R8 03a"},
    {testCmpR8E8_0x03a, "Test Cmp R8,E8 03a"},
    {testCmpR16R16_0x03b, "Test Cmp R16,R16 03b"},
    {testCmpR16E16_0x03b, "Test Cmp R16,E16 03b"},
    {testCmpR32R32_0x03b, "Test Cmp R32,R32 03b"},
    {testCmpR32E32_0x03b, "Test Cmp R32,E32 03b"},
    {testCmpAlIb_0x03c, "Test Cmp AL,Ib 03c"},
    {testCmpAxIw_0x03d, "Test Cmp AX,Iw 03d"},
    {testCmpEaxId_0x03d, "Test Cmp EAX,Id 03d"},
    {testSegDs_0x03e, "Test Seg DS 03e"},
    {testSegDs_0x23e, "Test Seg DS 23e"},
    {testAas_0x03f, "Test AAS 03f"},
    {testIncR16_0x040, "Test Inc R16 040"},
    {testIncR32_0x240, "Test Inc R32 240"},
    {testDecR16_0x048, "Test Dec R16 048"},
    {testDecR32_0x248, "Test Dec R32 248"},
    {testPushR16_0x050, "Test Push R16 050"},
    {testPushR32_0x250, "Test Push R32 250"},
    {testPopR16_0x058, "Test Pop R16 058"},
    {testPopR32_0x258, "Test Pop R32 258"},
    {testPushA16_0x060, "Test PushA 060"},
    {testPushA32_0x260, "Test PushA 260"},
    {testPopA16_0x061, "Test PopA 061"},
    {testPopA32_0x261, "Test PopA 261"},
    {testBoundR16M16_0x062, "Test Bound R16,M16 062"},
    {testBoundR32M32_0x262, "Test Bound R32,M32 262"},
    {testSegFs_0x064, "Test Seg FS 064"},
    {testSegFs_0x264, "Test Seg FS 264"},
    {testSegGs_0x065, "Test Seg GS 065"},
    {testSegGs_0x265, "Test Seg GS 265"},
    {testOpSizePrefix_0x066, "Test Operand size prefix 066"},
    {testOpSizePrefix_0x266, "Test Operand size prefix 266"},
    {testAddressPrefix_0x067, "Test Address prefix 067"},
    {testAddressPrefix_0x267, "Test Address prefix 267"},
    {testPushIw_0x068, "Test Push Iw 068"},
    {testPushId_0x268, "Test Push Id 268"},
    {testIMulR16E16Iw_0x069, "Test IMul R16,E16,Iw 069"},
    {testIMulR32E32Id_0x269, "Test IMul R32,E32,Id 269"},
    {testPushIb16_0x06a, "Test Push Ib 06a"},
    {testPushIb32_0x26a, "Test Push Ib 26a"},
    {testIMulR16E16Ib_0x06b, "Test IMul R16,E16,Ib 06b"},
    {testIMulR32E32Ib_0x26b, "Test IMul R32,E32,Ib 26b"},
    {testJO_0x070, "Test JO 070"},
    {testJO_0x270, "Test JO 270"},
    {testJNO_0x071, "Test JNO 071"},
    {testJNO_0x271, "Test JNO 271"},
    {testJB_0x072, "Test JB 072"},
    {testJB_0x272, "Test JB 272"},
    {testJNB_0x073, "Test JNB 073"},
    {testJNB_0x273, "Test JNB 273"},
    {testJZ_0x074, "Test JZ 074"},
    {testJZ_0x274, "Test JZ 274"},
    {testJNZ_0x075, "Test JNZ 075"},
    {testJNZ_0x275, "Test JNZ 275"},
    {testJBE_0x076, "Test JBE 076"},
    {testJBE_0x276, "Test JBE 276"},
    {testJNBE_0x077, "Test JNBE 077"},
    {testJNBE_0x277, "Test JNBE 277"},
    {testJS_0x078, "Test JS 078"},
    {testJS_0x278, "Test JS 278"},
    {testJNS_0x079, "Test JNS 079"},
    {testJNS_0x279, "Test JNS 279"},
    {testJP_0x07a, "Test JP 07a"},
    {testJP_0x27a, "Test JP 27a"},
    {testJNP_0x07b, "Test JNP 07b"},
    {testJNP_0x27b, "Test JNP 27b"},
    {testJL_0x07c, "Test JL 07c"},
    {testJL_0x27c, "Test JL 27c"},
    {testJNL_0x07d, "Test JNL 07d"},
    {testJNL_0x27d, "Test JNL 27d"},
    {testJLE_0x07e, "Test JLE 07e"},
    {testJLE_0x27e, "Test JLE 27e"},
    {testJNLE_0x07f, "Test JNLE 07f"},
    {testJNLE_0x27f, "Test JNLE 27f"},
    {testAddE8Ib_0x080, "Test Add E8,Ib 080"},
    {testOrE8Ib_0x080, "Test Or E8,Ib 080"},
    {testAdcE8Ib_0x080, "Test Adc E8,Ib 080"},
    {testSbbE8Ib_0x080, "Test Sbb E8,Ib 080"},
    {testAndE8Ib_0x080, "Test And E8,Ib 080"},
    {testSubE8Ib_0x080, "Test Sub E8,Ib 080"},
    {testXorE8Ib_0x080, "Test Xor E8,Ib 080"},
    {testCmpE8Ib_0x080, "Test Cmp E8,Ib 080"},
    {testAddE8Ib_0x280, "Test Add E8,Ib 280"},
    {testOrE8Ib_0x280, "Test Or E8,Ib 280"},
    {testAdcE8Ib_0x280, "Test Adc E8,Ib 280"},
    {testSbbE8Ib_0x280, "Test Sbb E8,Ib 280"},
    {testAndE8Ib_0x280, "Test And E8,Ib 280"},
    {testSubE8Ib_0x280, "Test Sub E8,Ib 280"},
    {testXorE8Ib_0x280, "Test Xor E8,Ib 280"},
    {testCmpE8Ib_0x280, "Test Cmp E8,Ib 280"},
    {testAddE16Iw_0x081, "Test Add E16,Iw 081"},
    {testOrE16Iw_0x081, "Test Or E16,Iw 081"},
    {testAdcE16Iw_0x081, "Test Adc E16,Iw 081"},
    {testSbbE16Iw_0x081, "Test Sbb E16,Iw 081"},
    {testAndE16Iw_0x081, "Test And E16,Iw 081"},
    {testSubE16Iw_0x081, "Test Sub E16,Iw 081"},
    {testXorE16Iw_0x081, "Test Xor E16,Iw 081"},
    {testCmpE16Iw_0x081, "Test Cmp E16,Iw 081"},
    {testAddE32Id_0x281, "Test Add E32,Id 281"},
    {testOrE32Id_0x281, "Test Or E32,Id 281"},
    {testAdcE32Id_0x281, "Test Adc E32,Id 281"},
    {testSbbE32Id_0x281, "Test Sbb E32,Id 281"},
    {testAndE32Id_0x281, "Test And E32,Id 281"},
    {testSubE32Id_0x281, "Test Sub E32,Id 281"},
    {testXorE32Id_0x281, "Test Xor E32,Id 281"},
    {testCmpE32Id_0x281, "Test Cmp E32,Id 281"},
    {testAddE8Ib_0x082, "Test Add E8,Ib 082"},
    {testOrE8Ib_0x082, "Test Or E8,Ib 082"},
    {testAdcE8Ib_0x082, "Test Adc E8,Ib 082"},
    {testSbbE8Ib_0x082, "Test Sbb E8,Ib 082"},
    {testAndE8Ib_0x082, "Test And E8,Ib 082"},
    {testSubE8Ib_0x082, "Test Sub E8,Ib 082"},
    {testXorE8Ib_0x082, "Test Xor E8,Ib 082"},
    {testCmpE8Ib_0x082, "Test Cmp E8,Ib 082"},
    {testAddE8Ib_0x282, "Test Add E8,Ib 282"},
    {testOrE8Ib_0x282, "Test Or E8,Ib 282"},
    {testAdcE8Ib_0x282, "Test Adc E8,Ib 282"},
    {testSbbE8Ib_0x282, "Test Sbb E8,Ib 282"},
    {testAndE8Ib_0x282, "Test And E8,Ib 282"},
    {testSubE8Ib_0x282, "Test Sub E8,Ib 282"},
    {testXorE8Ib_0x282, "Test Xor E8,Ib 282"},
    {testCmpE8Ib_0x282, "Test Cmp E8,Ib 282"},
    {testAddE16Ib_0x083, "Test Add E16,Ib 083"},
    {testOrE16Ib_0x083, "Test Or E16,Ib 083"},
    {testAdcE16Ib_0x083, "Test Adc E16,Ib 083"},
    {testSbbE16Ib_0x083, "Test Sbb E16,Ib 083"},
    {testAndE16Ib_0x083, "Test And E16,Ib 083"},
    {testSubE16Ib_0x083, "Test Sub E16,Ib 083"},
    {testXorE16Ib_0x083, "Test Xor E16,Ib 083"},
    {testCmpE16Ib_0x083, "Test Cmp E16,Ib 083"},
    {testAddE32Ib_0x283, "Test Add E32,Ib 283"},
    {testOrE32Ib_0x283, "Test Or E32,Ib 283"},
    {testAdcE32Ib_0x283, "Test Adc E32,Ib 283"},
    {testSbbE32Ib_0x283, "Test Sbb E32,Ib 283"},
    {testAndE32Ib_0x283, "Test And E32,Ib 283"},
    {testSubE32Ib_0x283, "Test Sub E32,Ib 283"},
    {testXorE32Ib_0x283, "Test Xor E32,Ib 283"},
    {testCmpE32Ib_0x283, "Test Cmp E32,Ib 283"},
    {testTestR8R8_0x084, "Test Test R8,R8 084"},
    {testTestE8R8_0x084, "Test Test E8,R8 084"},
    {testTestR8R8_0x284, "Test Test R8,R8 284"},
    {testTestE8R8_0x284, "Test Test E8,R8 284"},
    {testTestR16R16_0x085, "Test Test R16,R16 085"},
    {testTestE16R16_0x085, "Test Test E16,R16 085"},
    {testTestR32R32_0x285, "Test Test R32,R32 285"},
    {testTestE32R32_0x285, "Test Test E32,R32 285"},
    {testXchgR8R8_0x086, "Test Xchg R8,R8 086"},
    {testXchgE8R8_0x086, "Test Xchg E8,R8 086"},
    {testXchgR8R8_0x286, "Test Xchg R8,R8 286"},
    {testXchgE8R8_0x286, "Test Xchg E8,R8 286"},
    {testXchgR16R16_0x087, "Test Xchg R16,R16 087"},
    {testXchgE16R16_0x087, "Test Xchg E16,R16 087"},
    {testXchgR32R32_0x287, "Test Xchg R32,R32 287"},
    {testXchgE32R32_0x287, "Test Xchg E32,R32 287"},
    {testMovR8R8_0x088, "Test Mov R8,R8 088"},
    {testMovE8R8_0x088, "Test Mov E8,R8 088"},
    {testMovR8R8_0x288, "Test Mov R8,R8 288"},
    {testMovE8R8_0x288, "Test Mov E8,R8 288"},
    {testMovR16R16_0x089, "Test Mov R16,R16 089"},
    {testMovE16R16_0x089, "Test Mov E16,R16 089"},
    {testMovR32R32_0x289, "Test Mov R32,R32 289"},
    {testMovE32R32_0x289, "Test Mov E32,R32 289"},
    {testMovR8R8_0x08a, "Test Mov R8,R8 08a"},
    {testMovR8E8_0x08a, "Test Mov R8,E8 08a"},
    {testMovR8R8_0x28a, "Test Mov R8,R8 28a"},
    {testMovR8E8_0x28a, "Test Mov R8,E8 28a"},
    {testMovR16R16_0x08b, "Test Mov R16,R16 08b"},
    {testMovR16E16_0x08b, "Test Mov R16,E16 08b"},
    {testMovR32R32_0x28b, "Test Mov R32,R32 28b"},
    {testMovR32E32_0x28b, "Test Mov R32,E32 28b"},
    {testMovE16Sreg_0x08c, "Test Mov E16,Sreg 08c"},
    {testMovE32Sreg_0x28c, "Test Mov E32,Sreg 28c"},
    {testLeaR16M_0x08d, "Test Lea R16,M 08d"},
    {testLeaR32M_0x28d, "Test Lea R32,M 28d"},
    {testMovSregE16_0x08e, "Test Mov Sreg,E16 08e"},
    {testMovSregE32_0x28e, "Test Mov Sreg,E32 28e"},
    {testPopE16_0x08f, "Test Pop E16 08f"},
    {testPopE32_0x28f, "Test Pop E32 28f"},
    {testXchgR16Ax_0x090, "Test Xchg AX,AX 090"},
    {testXchgR32Eax_0x290, "Test Xchg EAX,EAX 290"},
    {testXchgR16Ax_0x091, "Test Xchg CX,AX 091"},
    {testXchgR32Eax_0x291, "Test Xchg ECX,EAX 291"},
    {testXchgR16Ax_0x092, "Test Xchg DX,AX 092"},
    {testXchgR32Eax_0x292, "Test Xchg EDX,EAX 292"},
    {testXchgR16Ax_0x093, "Test Xchg BX,AX 093"},
    {testXchgR32Eax_0x293, "Test Xchg EBX,EAX 293"},
    {testXchgR16Ax_0x094, "Test Xchg SP,AX 094"},
    {testXchgR32Eax_0x294, "Test Xchg ESP,EAX 294"},
    {testXchgR16Ax_0x095, "Test Xchg BP,AX 095"},
    {testXchgR32Eax_0x295, "Test Xchg EBP,EAX 295"},
    {testXchgR16Ax_0x096, "Test Xchg SI,AX 096"},
    {testXchgR32Eax_0x296, "Test Xchg ESI,EAX 296"},
    {testXchgR16Ax_0x097, "Test Xchg DI,AX 097"},
    {testXchgR32Eax_0x297, "Test Xchg EDI,EAX 297"},
    {testCmpXchgE8R8_0x1b0, "Test CmpXchg E8,R8 1b0"},
    {testCmpXchgE8R8_0x3b0, "Test CmpXchg E8,R8 3b0"},
    {testCmpXchgE16R16_0x1b1, "Test CmpXchg E16,R16 1b1"},
    {testCmpXchgE32R32_0x3b1, "Test CmpXchg E32,R32 3b1"},
    {testCbw_0x098, "Test Cbw 098"},
    {testCwde_0x298, "Test Cwde 298"},
    {testCwd_0x099, "Test Cwd 099"},
    {testCdq_0x299, "Test Cdq 299"},
    {testPushF16_0x09c, "Test Pushf 09c"},
    {testPushF32_0x29c, "Test Pushf 29c"},
    {testSahf_0x09e, "Test Sahf 09e"},
    {testSahf_0x29e, "Test Sahf 29e"},
    {testLahf_0x09f, "Test Lahf 09f"},
    {testLahf_0x29f, "Test Lahf 29f"},
    {testMovAlOb_0x0a0, "Test Mov AL,Ob 0a0"},
    {testMovAlOb_0x2a0, "Test Mov AL,Ob 2a0"},
    {testMovAxOw_0x0a1, "Test Mov AX,Ow 0a1"},
    {testMovEaxOd_0x2a1, "Test Mov EAX,Od 2a1"},
    {testMovObAl_0x0a2, "Test Mov Ob,AL 0a2"},
    {testMovObAl_0x2a2, "Test Mov Ob,AL 2a2"},
    {testMovOwAx_0x0a3, "Test Mov Ow,AX 0a3"},
    {testMovOdEax_0x2a3, "Test Mov Od,EAX 2a3"},
    {testCallFar16_0x09a, "Test Call Far16 09a"},
    {testCallFar32_0x29a, "Test Call Far32 29a"},
    {testMovsb_0x0a4, "Test Movsb 0a4"},
    {testMovsb_0x2a4, "Test Movsb 2a4"},
    {testMovsw_0x0a5, "Test Movsw 0a5"},
    {testMovsd_0x2a5, "Test Movsd 2a5"},
    {testCmpsb_0x0a6, "Test Cmpsb 0a6"},
    {testCmpsb_0x2a6, "Test Cmpsb 2a6"},
    {testCmpsw_0x0a7, "Test Cmpsw 0a7"},
    {testCmpsd_0x2a7, "Test Cmpsd 2a7"},
    {testTestAlIb_0x0a8, "Test Test AL,Ib 0a8"},
    {testTestAlIb_0x2a8, "Test Test AL,Ib 2a8"},
    {testTestAxIw_0x0a9, "Test Test AX,Iw 0a9"},
    {testTestEaxId_0x2a9, "Test Test EAX,Id 2a9"},
    {testStosb_0x0aa, "Test Stosb 0aa"},
    {testStosb_0x2aa, "Test Stosb 2aa"},
    {testStosw_0x0ab, "Test Stosw 0ab"},
    {testStosd_0x2ab, "Test Stosd 2ab"},
    {testLodsb_0x0ac, "Test Lodsb 0ac"},
    {testLodsb_0x2ac, "Test Lodsb 2ac"},
    {testLodsw_0x0ad, "Test Lodsw 0ad"},
    {testLodsd_0x2ad, "Test Lodsd 2ad"},
    {testScasb_0x0ae, "Test Scasb 0ae"},
    {testScasb_0x2ae, "Test Scasb 2ae"},
    {testScasw_0x0af, "Test Scasw 0af"},
    {testScasd_0x2af, "Test Scasd 2af"},
    {testMmxMovd_0x36e_0x37e, "Test MMX Movd 36e/37e"},
    {testMmxMovq_0x36f_0x37f, "Test MMX Movq 36f/37f"},
    {testMmxUnpackPackCompare_0x360_0x361_0x362_0x363_0x364_0x365_0x366_0x367_0x368_0x369_0x36a_0x36b_0x374_0x375_0x376, "Test MMX Unpack/Pack/Compare"},
    {testMmxShiftImm_0x371_0x372_0x373, "Test MMX Shift Imm 371/372/373"},
    {testMmxShiftReg_0x3d1_0x3d2_0x3d3_0x3e1_0x3e2_0x3f1_0x3f2_0x3f3, "Test MMX Shift Reg"},
    {testMmxSaturatingArithmetic_0x3d8_0x3d9_0x3dc_0x3dd_0x3e8_0x3e9_0x3ec_0x3ed, "Test MMX Saturating Arithmetic"},
    {testMmxMultiplySubtract_0x3d4_0x3d5_0x3e5_0x3f5_0x3f8_0x3f9_0x3fa, "Test MMX Multiply/Subtract"},
    {testMmxPadd_0x3fc_0x3fd_0x3fe, "Test MMX Padd 3fc/3fd/3fe"},
    {testMmxLogic_0x3db_0x3df_0x3eb_0x3ef, "Test MMX Logic 3db/3df/3eb/3ef"},
    {testSseMovups_0x310_0x311, "Test SSE Movups 310/311"},
    {testSseMovss_0xf310_0xf311, "Test SSE Movss f310/f311"},
    {testSseLogic_0x354_0x355_0x356_0x357, "Test SSE Logic 354/355/356/357"},
    {testSseAddps_0x358, "Test SSE Addps 358"},
    {testSseMoveUnpack_0x312_0x313_0x316_0x317_0x328_0x329, "Test SSE Move/Unpack 312/313/316/317/328/329"},
    {testSseMovntps_0x32b, "Test SSE Movntps 32b"},
    {testSseConvert_0x32a_0x32c_0x32d, "Test SSE Convert 32a/32c/32d"},
    {testSseCompareFlags_0x32e_0x32f, "Test SSE Compare Flags 32e/32f"},
    {testSseMovmskAndApprox_0x350_0x351_0x352_0x353, "Test SSE Movmsk/Approx 350/351/352/353"},
    {testSseArithmetic_0xf358_0x359_0x35c_0x35d_0x35e_0x35f, "Test SSE Arithmetic 358/359/35c/35d/35e/35f"},
    {testSsePshufw_0x370, "Test SSE Pshufw 370"},
    {testSse2Move_0x110_0x310_0x111_0x311_0x112_0x113_0x116_0x117_0x128_0x129, "Test SSE2 Move"},
    {testSse2UnpackShuffle_0x114_0x115_0x160_0x161_0x162_0x168_0x169_0x16a_0x16c_0x16d_0x170_0x370_0x1c6, "Test SSE2 Unpack/Shuffle"},
    {testSse2PackCompare_0x163_0x164_0x165_0x166_0x167_0x16b_0x174_0x175_0x176, "Test SSE2 Pack/Compare"},
    {testSse2Shift_0x171_0x172_0x173_0x1d1_0x1d2_0x1d3_0x1e1_0x1e2_0x1f1_0x1f2_0x1f3, "Test SSE2 Shift"},
    {testSse2Convert_0x12a_0x32a_0x12c_0x32c_0x12d_0x32d_0x15a_0x35a_0x15b_0x35b_0x1e6_0x3e6, "Test SSE2 Convert"},
    {testSse2CompareFlags_0x12e_0x12f, "Test SSE2 Compare Flags"},
    {testSse2Arithmetic_0x151_0x351_0x154_0x155_0x156_0x157_0x158_0x358_0x159_0x359_0x15c_0x35c_0x15d_0x35d_0x15e_0x35e_0x15f_0x35f, "Test SSE2 Arithmetic"},
    {testSse2Transfers_0x16e_0x17e_0x37e_0x1d6_0x3d6_0x3c3_0x1e7_0x1f7, "Test SSE2 Transfers"},
    {testSse2PackedArithmetic_0x1d4_0x1d5_0x1d7_0x1d8_0x1d9_0x1da_0x1db_0x1dc_0x1dd_0x1de_0x1df_0x1e0_0x1e3_0x1e4_0x1e5_0x1e8_0x1e9_0x1ea_0x1eb_0x1ec_0x1ed_0x1ee_0x1ef_0x1f4_0x1f5_0x1f6_0x1f8_0x1f9_0x1fa_0x1fb_0x1fc_0x1fd_0x1fe, "Test SSE2 Packed Arithmetic"},
    {testMovR8Ib_0x0b0, "Test Mov R8,Ib 0b0"},
    {testMovR8Ib_0x2b0, "Test Mov R8,Ib 2b0"},
    {testMovR16Iw_0x0b8, "Test Mov R16,Iw 0b8"},
    {testMovR32Id_0x2b8, "Test Mov R32,Id 2b8"},
    {testShiftE8Ib_0x0c0, "Test Shift E8,Ib 0c0"},
    {testShiftE8Ib_0x2c0, "Test Shift E8,Ib 2c0"},
    {testShiftE16Ib_0x0c1, "Test Shift E16,Ib 0c1"},
    {testShiftE32Ib_0x2c1, "Test Shift E32,Ib 2c1"},
    {testMovE8Ib_0x0c6, "Test Mov E8,Ib 0c6"},
    {testMovE8Ib_0x2c6, "Test Mov E8,Ib 2c6"},
    {testMovE16Iw_0x0c7, "Test Mov E16,Iw 0c7"},
    {testMovE32Id_0x2c7, "Test Mov E32,Id 2c7"},
    {testLesR16M16_0x0c4, "Test Les R16,M16 c4"},
    {testLesR32M32_0x2c4, "Test Les R32,M32 2c4"},
    {testLdsR16M16_0x0c5, "Test Lds R16,M16 c5"},
    {testLdsR32M32_0x2c5, "Test Lds R32,M32 2c5"},
    {testRetn16Iw_0x0c2, "Test Retn16,Iw 0c2"},
    {testRetn32Iw_0x2c2, "Test Retn32,Iw 2c2"},
    {testRetn16_0x0c3, "Test Retn16 0c3"},
    {testRetn32_0x2c3, "Test Retn32 2c3"},
    {testCallJw_0x0e8, "Test Call Jw 0e8"},
    {testCallJd_0x2e8, "Test Call Jd 2e8"},
    {testCallR16_0x0ff, "Test Call R16 0ff"},
    {testCallE16_0x0ff, "Test Call E16 0ff"},
    {testCallFarE16_0x0ff, "Test Call Far E16 0ff"},
    {testCallR32_0x2ff, "Test Call R32 2ff"},
    {testCallE32_0x2ff, "Test Call E32 2ff"},
    {testCallFarE32_0x2ff, "Test Call Far E32 2ff"},
    {testEnter16_0x0c8, "Test Enter16 0c8"},
    {testEnter32_0x2c8, "Test Enter32 2c8"},
    {testLeave16_0x0c9, "Test Leave16 0c9"},
    {testLeave32_0x2c9, "Test Leave32 2c9"},
    {testShiftE8_0x0d0, "Test Shift E8,1 0d0"},
    {testShiftE8_0x2d0, "Test Shift E8,1 2d0"},
    {testShiftE16_0x0d1, "Test Shift E16,1 0d1"},
    {testShiftE32_0x2d1, "Test Shift E32,1 2d1"},
    {testShiftE8Cl_0x0d2, "Test Shift E8,CL 0d2"},
    {testShiftE8Cl_0x2d2, "Test Shift E8,CL 2d2"},
    {testShiftE16Cl_0x0d3, "Test Shift E16,CL 0d3"},
    {testShiftE32Cl_0x2d3, "Test Shift E32,CL 2d3"},
    {testAam_0x0d4, "Test AAM 0d4"},
    {testAad_0x0d5, "Test AAD 0d5"},
    {testSalc_0x0d6, "Test Salc 0d6"},
    {testSalc_0x2d6, "Test Salc 2d6"},
    {testXlat_0x0d7, "Test Xlat 0d7"},
    {testXlat_0x2d7, "Test Xlat 2d7"},
    {testFpuD8_0x0d8, "Test FPU D8 0d8"},
    {testFpuD8_0x2d8, "Test FPU D8 2d8"},
    {testFpuD9_0x0d9, "Test FPU D9 0d9"},
    {testFpuD9_0x2d9, "Test FPU D9 2d9"},
    {testFpuDA_0x0da, "Test FPU DA 0da"},
    {testFpuDA_0x2da, "Test FPU DA 2da"},
    {testFpuDB_0x0db, "Test FPU DB 0db"},
    {testFpuDB_0x2db, "Test FPU DB 2db"},
    {testFpuDC_0x0dc, "Test FPU DC 0dc"},
    {testFpuDC_0x2dc, "Test FPU DC 2dc"},
    {testFpuDD_0x0dd, "Test FPU DD 0dd"},
    {testFpuDD_0x2dd, "Test FPU DD 2dd"},
    {testFpuDE_0x0de, "Test FPU DE 0de"},
    {testFpuDE_0x2de, "Test FPU DE 2de"},
    {testFpuDF_0x0df, "Test FPU DF 0df"},
    {testFpuDF_0x2df, "Test FPU DF 2df"},
    {testLoopnz_0x0e0, "Test LoopNZ 0e0"},
    {testLoopnz_0x2e0, "Test LoopNZ 2e0"},
    {testLoopz_0x0e1, "Test LoopZ 0e1"},
    {testLoopz_0x2e1, "Test LoopZ 2e1"},
    {testLoop_0x0e2, "Test Loop 0e2"},
    {testLoop_0x2e2, "Test Loop 2e2"},
    {testJcxz_0x0e3, "Test Jcxz 0e3"},
    {testJecxz_0x2e3, "Test Jecxz 2e3"},
    {testJmpJw_0x0e9, "Test Jmp Jw 0e9"},
    {testJmpJd_0x2e9, "Test Jmp Jd 2e9"},
    {testCmc_0x0f5, "Test Cmc 0f5"},
    {testCmc_0x2f5, "Test Cmc 2f5"},
    {testTestE8Ib_0x0f6, "Test Test E8,Ib 0f6"},
    {testTestE8IbAlias_0x0f6, "Test Test E8,Ib /1 0f6"},
    {testNotE8_0x0f6, "Test Not E8 0f6"},
    {testNegE8_0x0f6, "Test Neg E8 0f6"},
    {testMulR8E8_0x0f6, "Test Mul R8/E8 0f6"},
    {testIMulR8E8_0x0f6, "Test IMul R8/E8 0f6"},
    {testDivR8E8_0x0f6, "Test Div R8/E8 0f6"},
    {testIDivR8E8_0x0f6, "Test IDiv R8/E8 0f6"},
    {testTestE8Ib_0x2f6, "Test Test E8,Ib 2f6"},
    {testTestE8IbAlias_0x2f6, "Test Test E8,Ib /1 2f6"},
    {testNotE8_0x2f6, "Test Not E8 2f6"},
    {testNegE8_0x2f6, "Test Neg E8 2f6"},
    {testMulR8E8_0x2f6, "Test Mul R8/E8 2f6"},
    {testIMulR8E8_0x2f6, "Test IMul R8/E8 2f6"},
    {testDivR8E8_0x2f6, "Test Div R8/E8 2f6"},
    {testIDivR8E8_0x2f6, "Test IDiv R8/E8 2f6"},
    {testTestE16Iw_0x0f7, "Test Test E16,Iw 0f7"},
    {testTestE16IwAlias_0x0f7, "Test Test E16,Iw /1 0f7"},
    {testNotE16_0x0f7, "Test Not E16 0f7"},
    {testNegE16_0x0f7, "Test Neg E16 0f7"},
    {testMulR16E16_0x0f7, "Test Mul R16/E16 0f7"},
    {testIMulR16E16_0x0f7, "Test IMul R16/E16 0f7"},
    {testDivR16E16_0x0f7, "Test Div R16/E16 0f7"},
    {testIDivR16E16_0x0f7, "Test IDiv R16/E16 0f7"},
    {testTestE32Id_0x2f7, "Test Test E32,Id 2f7"},
    {testTestE32IdAlias_0x2f7, "Test Test E32,Id /1 2f7"},
    {testNotE32_0x2f7, "Test Not E32 2f7"},
    {testNegE32_0x2f7, "Test Neg E32 2f7"},
    {testMulR32E32_0x2f7, "Test Mul R32/E32 2f7"},
    {testIMulR32E32_0x2f7, "Test IMul R32/E32 2f7"},
    {testDivR32E32_0x2f7, "Test Div R32/E32 2f7"},
    {testIDivR32E32_0x2f7, "Test IDiv R32/E32 2f7"},
    {testClc_0x0f8, "Test Clc 0f8"},
    {testClc_0x2f8, "Test Clc 2f8"},
    {testStc_0x0f9, "Test Stc 0f9"},
    {testStc_0x2f9, "Test Stc 2f9"},
    {testIncR8_0x0fe, "Test Inc R8 0fe"},
    {testIncE8_0x0fe, "Test Inc E8 0fe"},
    {testDecR8_0x0fe, "Test Dec R8 0fe"},
    {testDecE8_0x0fe, "Test Dec E8 0fe"},
    {testIncE16_0x0ff, "Test Inc E16 0ff"},
    {testDecE16_0x0ff, "Test Dec E16 0ff"},
    {testJmpE16_0x0ff, "Test Jmp E16 0ff"},
    {testJmpFarE16_0x0ff, "Test Jmp Far E16 0ff"},
    {testPushE16_0x0ff, "Test Push E16 0ff"},
    {testIncE32_0x2ff, "Test Inc E32 2ff"},
    {testDecE32_0x2ff, "Test Dec E32 2ff"},
    {testJmpE32_0x2ff, "Test Jmp E32 2ff"},
    {testJmpFarE32_0x2ff, "Test Jmp Far E32 2ff"},
    {testPushE32_0x2ff, "Test Push E32 2ff"},
    {testCmovR16E16_0x140_0x14f, "Test Cmov R16,E16 140-14f"},
    {testCmovR32E32_0x340_0x34f, "Test Cmov R32,E32 340-34f"},
    {testSetE8_0x190_0x19f, "Test Set E8 190-19f"},
    {testSetE8_0x390_0x39f, "Test Set E8 390-39f"},
    {testBtE16R16_0x1a3, "Test Bt E16,R16 1a3"},
    {testBtE32R32_0x3a3, "Test Bt E32,R32 3a3"},
    {testShldE16R16Ib_0x1a4, "Test Shld E16,R16,Ib 1a4"},
    {testShldE32R32Ib_0x3a4, "Test Shld E32,R32,Ib 3a4"},
    {testShldE16R16Cl_0x1a5, "Test Shld E16,R16,CL 1a5"},
    {testShldE32R32Cl_0x3a5, "Test Shld E32,R32,CL 3a5"},
    {testBtsE16R16_0x1ab, "Test Bts E16,R16 1ab"},
    {testBtsE32R32_0x3ab, "Test Bts E32,R32 3ab"},
    {testShrdE16R16Ib_0x1ac, "Test Shrd E16,R16,Ib 1ac"},
    {testShrdE32R32Ib_0x3ac, "Test Shrd E32,R32,Ib 3ac"},
    {testShrdE16R16Cl_0x1ad, "Test Shrd E16,R16,CL 1ad"},
    {testShrdE32R32Cl_0x3ad, "Test Shrd E32,R32,CL 3ad"},
    {testIMulR16E16_0x1af, "Test IMUL R16,E16 1af"},
    {testIMulR32E32_0x3af, "Test IMUL R32,E32 3af"},
    {testLssR16M16_0x1b2, "Test Lss R16,M16 1b2"},
    {testLssR32M32_0x3b2, "Test Lss R32,M32 3b2"},
    {testLfsR16M16_0x1b4, "Test Lfs R16,M16 1b4"},
    {testLfsR32M32_0x3b4, "Test Lfs R32,M32 3b4"},
    {testLgsR16M16_0x1b5, "Test Lgs R16,M16 1b5"},
    {testLgsR32M32_0x3b5, "Test Lgs R32,M32 3b5"},
    {testBtrE16R16_0x1b3, "Test Btr E16,R16 1b3"},
    {testBtrE32R32_0x3b3, "Test Btr E32,R32 3b3"},
    {testMovzxR16E8_0x1b6, "Test Movzx R16,E8 1b6"},
    {testMovzxR32E8_0x3b6, "Test Movzx R32,E8 3b6"},
    {testMovzxR32E16_0x3b7, "Test Movzx R32,E16 3b7"},
    {testGroup8E16Ib_0x1ba, "Test Group8 E16,Ib 1ba"},
    {testGroup8E32Ib_0x3ba, "Test Group8 E32,Ib 3ba"},
    {testBtcE16R16_0x1bb, "Test Btc E16,R16 1bb"},
    {testBtcE32R32_0x3bb, "Test Btc E32,R32 3bb"},
    {testBsfR16E16_0x1bc, "Test Bsf R16,E16 1bc"},
    {testBsfR32E32_0x3bc, "Test Bsf R32,E32 3bc"},
    {testBsrR16E16_0x1bd, "Test Bsr R16,E16 1bd"},
    {testBsrR32E32_0x3bd, "Test Bsr R32,E32 3bd"},
    {testMovsxR16E8_0x1be, "Test Movsx R16,E8 1be"},
    {testMovsxR32E8_0x3be, "Test Movsx R32,E8 3be"},
    {testMovsxR32E16_0x3bf, "Test Movsx R32,E16 3bf"},
    {testXaddE8R8_0x1c0, "Test Xadd E8,R8 1c0"},
    {testXaddE8R8_0x3c0, "Test Xadd E8,R8 3c0"},
    {testXaddE16R16_0x1c1, "Test Xadd E16,R16 1c1"},
    {testXaddE32R32_0x3c1, "Test Xadd E32,R32 3c1"},
    {testSseCompareImmediate_0x3c2, "Test SSE Compare Immediate 3c2"},
    {testSseInsertExtractShuffle_0x1c4_0x3c4_0x1c5_0x3c5_0x3c6, "Test SSE Insert/Extract/Shuffle"},
    {testCmpXchg8b_0x3c7, "Test CmpXchg8b 3c7"},
    {testBswap_0x3c8_0x3cf, "Test Bswap 3c8-3cf"},
    {testMmxPmovmskb_0x3d7, "Test MMX Pmovmskb 3d7"},
    {testMmxPminub_0x3da, "Test MMX Pminub 3da"},
    {testMmxPmaxub_0x3de, "Test MMX Pmaxub 3de"},
    {testMmxPavgb_0x3e0, "Test MMX Pavgb 3e0"},
    {testMmxPavgw_0x3e3, "Test MMX Pavgw 3e3"},
    {testMmxPmulhuw_0x3e4, "Test MMX Pmulhuw 3e4"},
    {testMmxMovntq_0x3e7, "Test MMX Movntq 3e7"},
    {testMmxPminsw_0x3ea, "Test MMX Pminsw 3ea"},
    {testMmxPmaxsw_0x3ee, "Test MMX Pmaxsw 3ee"},
    {testMmxPsadbw_0x3f6, "Test MMX Psadbw 3f6"},
    {testMmxMaskmovq_0x3f7, "Test MMX Maskmovq 3f7"},
    {testSelfModifying, "Test Self Modifying Code"},
    {testSelfModifyingMovsb, "Test Self Modifying Code using movsb"},
    {testSelfModifyingFront, "Test Self Modifying Code Same Block(Previous)"},
    {testSelfModifyingBack, "Test Self Modifying Code Same Block(Next)"},
#ifdef BOXEDWINE_MULTI_THREADED
    {testLockedInc, "Test Multi-threaded locked inc"},
#endif
    {testWaitPid, "Test waitpid child selection"},
    {testHardLinksShareIdentityDataAndXattrs, "Test hard links share identity, data, and xattrs"},
    {testReadDirectoryReturnsIsDir, "Test read on directory returns EISDIR"},
    {testUtimensatPreservesAccessTimeInStat, "Test utimensat preserves access time in stat"},
    {testFutimensPreservesAccessTimeInFstat, "Test futimens preserves access time in fstat"},
    {testFutimensTime64SignExtendedSecondsPreservesAccessTimeInFstat, "Test futimens time64 sign-extended seconds in fstat"},
    {testDirectoryReparseSidecarReplacedAfterRemoveAndRecreate, "Test directory reparse sidecar replacement after recreate"},
    {testDotDotAfterDotResolvesToParentDirectory, "Test ./.. resolves to parent directory"},
    {testUtf8NamesSurviveNativeFilesystemReload, "Test UTF-8 file names survive native filesystem reload"},
    {testTrailingDotNamesCanBeUnlinked, "Test trailing-dot file names can be unlinked"},
    {testDirectorySeekCanStoreOpaquePosition, "Test directory seek can store opaque position"},
    {testStartupArgsDefaultUtf8LocaleEnvironment, "Test startup args default UTF-8 locale environment"},
};

} // namespace

void failed(const char* msg, ...) {
    totalFails++;
}

int runTestTests(size_t startEntry = 0, size_t requestedCount = 0, U32 workerCount = 0) {
    size_t entryCount = sizeof(TEST_ENTRIES) / sizeof(TEST_ENTRIES[0]);

    if (startEntry > entryCount) {
        startEntry = entryCount;
    }
    size_t runCount = entryCount - startEntry;
    if (requestedCount && requestedCount < runCount) {
        runCount = requestedCount;
    }

#ifdef __EMSCRIPTEN__
    if (workerCount != 1) {
        workerCount = 1;
    }
#else
    if (!workerCount) {
        workerCount = std::thread::hardware_concurrency();
    }
#endif
    if (!workerCount) {
        workerCount = 1;
    }
    if (workerCount > entryCount) {
        workerCount = (U32)entryCount;
    }
    
    printf("Running %zu Test tests with %d threads", runCount, workerCount);
    if (startEntry || runCount != entryCount) {
        printf(" starting at %zu", startEntry);
    }
    printf("\n");
    fflush(stdout);
    
    KSystem::startMicroCounter();
    KSystem::init();
    KSystem::videoOption = VIDEO_NO_WINDOW;
    U32 startTime = KSystem::getMilliesSinceStart();
    testRunParallel(TEST_ENTRIES + startEntry, runCount, workerCount);
    U32 stopTime = KSystem::getMilliesSinceStart();
    printf("%d tests FAILED in %ds\n", totalFails, (stopTime - startTime) / 1000);
    return totalFails != 0;
}

int runTestTestsFromArgs(int argc, char** argv) {
    size_t startEntry = argc > 1 ? (size_t)strtoul(argv[1], nullptr, 0) : 0;
    size_t requestedCount = argc > 2 ? (size_t)strtoul(argv[2], nullptr, 0) : 0;
    U32 workerCount = argc > 3 ? (U32)strtoul(argv[3], nullptr, 0) : 0;
    return runTestTests(startEntry, requestedCount, workerCount);
}

#ifdef __MACH__
#if defined(__GNUC__)
#define BOXEDWINE_TEST_EXPORT __attribute__((visibility("default")))
#else
#define BOXEDWINE_TEST_EXPORT
#endif

extern "C" BOXEDWINE_TEST_EXPORT int runCpuTestsMac(void);

extern "C" BOXEDWINE_TEST_EXPORT int runCpuTestsMac(void) {
    const char* start = getenv("BOXEDWINE_TEST_START");
    const char* count = getenv("BOXEDWINE_TEST_COUNT");
    const char* threads = getenv("BOXEDWINE_TEST_THREADS");
    if (start || count || threads) {
        return runTestTests(
            start ? (size_t)strtoul(start, nullptr, 0) : 0,
            count ? (size_t)strtoul(count, nullptr, 0) : 0,
            threads ? (U32)strtoul(threads, nullptr, 0) : 0);
    }
    return runTestTests();
}

#undef BOXEDWINE_TEST_EXPORT
#else
int main(int argc, char** argv) {
    return runTestTestsFromArgs(argc, argv);
}
#endif

#endif

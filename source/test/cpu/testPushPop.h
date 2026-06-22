/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_PUSH_POP_H__
#define __TEST_PUSH_POP_H__

void testPushR16_0x050();
void testPushR32_0x250();
void testPopR16_0x058();
void testPopR32_0x258();
void testPushA16_0x060();
void testPushA32_0x260();
void testPopA16_0x061();
void testPopA32_0x261();
void testPushIw_0x068();
void testPushId_0x268();
void testPushIb16_0x06a();
void testPushIb32_0x26a();
void testPopE16_0x08f();
void testPopE32_0x28f();
void testPushF16_0x09c();
void testPushF32_0x29c();
void testCliRaisesProtectionFault();
void testPrefixedCliRaisesProtectionFault();
void testOverlongPrefixedCliDoesNotPoisonCodeCache();
void testInt2dRaisesInterruptProtectionFault();
void testInt3ImmediateRaisesBreakpoint();
void testPortIoRaisesProtectionFault();
void testHltRaisesProtectionFault();
void testInvalidInterruptRaisesProtectionFault();
void testControlRegisterAccessRaisesProtectionFault();
void testNullSegmentMoffsRaisesProtectionFault();
void testPopSsFromCodeSelectorRaisesProtectionFault();
void testInstructionFetchFaultSetsExecuteBit();
void testSingleStepRaisesTrap();
void testSingleStepRaisesTrapAfterRet();
void testIcebpRaisesSingleStepTrap();
void testDivpsRaisesSimdException();
void testDivssRaisesSimdException();
void testX87FwaitRaisesPendingException();
void testHardwareBreakpointRaisesTrap();
void testHardwareBreakpointIgnoresNonExecutableAddress();
void testDataHardwareBreakpointRaisesTrap();
void testPushE16_0x0ff();
void testPushE32_0x2ff();

#endif

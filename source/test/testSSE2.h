/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __TEST_SSE2_H__
#define __TEST_SSE2_H__

void testSse2MovUps110();
void testSse2MovSd310();
void testSse2MovPd111();
void testSse2MovSd311();
void testSse2MovLpd112();
void testSse2MovLpd113();
void testSse2Unpcklpd114();
void testSse2Unpckhpd115();
void testSse2Movhpd116();
void testSse2Movhpd117();
void testSse2Movapd128();
void testSse2Movapd129();
void testSse2Cvtpi2pd12a();
void testSse2Cvtsi2sd32a();
void testSse2Movntpd12b();
void testSseCvttpd2pi12c();
void testSse2Cvttsd2si32c();
void testSse2Cvtpd2pi12d();
void testSse2Cvtsd2si32d();
void testSse2Ucomisd12e();
void testSse2Comisd12f();
void testSse2Movmskpd150();
void testSse2Sqrtpd151();
void testSse2Sqrtsd351();
void testSse2Andpd154();
void testSse2Andnpd155();
void testSse2Orpd156();
void testSse2Xorpd157();
void testSse2Addpd158();
void testSse2Addsd358();
void testSse2Mulpd159();
void testSse2Mulsd359();
void testSse2Cvtpd2ps15a();
void testSse2Cvtps2pd35a();
void testSse2Cvtsd2ss35a();
void testSse2Cvtss2sd35a();
void testSse2Cvtps2dq15b();
void testSse2Cvtdq2ps35b();
void testSse2Cvttps2dq35b();
void testSse2Subpd15c();
void testSse2Subsd35c();
void testSse2Minpd15d();
void testSse2Minsd35d();
void testSse2Divpd15e();
void testSse2Divsd35e();
void testSse2Maxpd15f();
void testSse2Maxsd35f();
void testSse2Punpcklbw160();
void testSse2xPunpcklwd161();
void testSse2Punpckldq162();
void testSse2Packsswb163();
void testSse2Pcmpgtb164();
void testSse2Pcmpgtw165();
void testSse2Pcmpgtd166();
void testSse2Packuswb167();
void testSse2Punpckhbw168();
void testSse2Punpckhwd169();
void testSse2Punpckhdq16a();
void testSse2Packssdw16b();
void testSse2Punpcklqdq16c();
void testSse2Punpckhqdq16d();
void testSse2Movd16e();
void testSse2Movdqa16f();
void testSse2Movdqu36f();
void testSse2Pshufd170();
void testSse2Pshuflw370();
void testSse2Pshufhw370();
void testSse2Psrlw171();
void testSse2Psraw171();
void testSse2Psllw171();
void testSse2Psrld172();
void testSse2Psrad172();
void testSse2Pslld172();
void testSse2Psrlq173();
void testSse2Psrldq173();
void testSse2Psllq173();
void testSse2Pslldq173();
void testSse2Pcmpeqb174();
void testSse2Pcmpeqw175();
void testSse2Pcmpeqd176();
void testSse2Movd17e();
void testSse2Movq37e();
void testSse2Movdqa17f();
void testSse2Movdqu37f();
void testSse2Cmppd1c2();
void testSse2Cmpsd3c2();
void testSse2Movnti3c3();
void testPinsrw1c4();
void testSse2Shufpd1c6();
void testSse2Psrlw1d1();
void testSse2Psrld1d2();
void testSse2Psrlq1d3();
void testSse2Paddq1d4();
void testSse2Pmullw1d5();
void testSse2Movq1d6();
void testSse2Movdq2q3d6();
void testSse2Movq2dq3d6();
void testSse2Pmovmskb1d7();
void testSse2Psubusb1d8();
void testSse2Psubusw1d9();
void testSse2Pminub1da();
void testSse2Pand1db();
void testSse2Paddusb1dc();
void testSse2Paddusw1dd();
void testSse2Pmaxub1de();
void testSse2Pandn1df();
void testSse2Pavgb1e0();
void testSse2Psraw1e1();
void testSse2Psrad1e2();
void testSse2Pavgw1e3();
void testSse2Pmulhuw1e4();
void testSse2Pmulhw1e5();
void testSse2Cvttpd2dq1e6();
void testSse2Cvtpd2dq3e6();
void testSse2Cvtdq2pd3e6();
void testSse2Movntdq1e7();
void testSse2Psubsb1e8();
void testSse2Psubsw1e9();
void testSse2Pminsw1ea();
void testSse2Por1eb();
void testSse2Paddsb1ec();
void testSse2Paddsw1ed();
void testSse2Pmaxsw1ee();
void testSse2Pxor1ef();
void testSse2Psllw1f1();
void testSse2Pslld1f2();
void testSse2Psllq1f3();
void testSse2Pmuludq1f4();
void testSse2Pmaddwd1f5();
void testSse2Psadbw1f6();
void testSse2Maskmovdqu1f7();
void testSse2Psubb1f8();
void testSse2Psubw1f9();
void testSse2Psubd1fa();
void testSse2Psubq1fb();
void testSse2Paddb1fc();
void testSse2Paddw1fd();
void testSse2Paddd1fe();

#endif
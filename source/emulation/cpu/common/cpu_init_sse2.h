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

INIT_CPU(AddpdXmmXmm, addpdXmmXmm)
INIT_CPU(AddpdXmmE128, addpdXmmE128)
INIT_CPU(AddsdXmmXmm, addsdXmmXmm)
INIT_CPU(AddsdXmmE64, addsdXmmE64)
INIT_CPU(SubpdXmmXmm, subpdXmmXmm)
INIT_CPU(SubpdXmmE128, subpdXmmE128)
INIT_CPU(SubsdXmmXmm, subsdXmmXmm)
INIT_CPU(SubsdXmmE64, subsdXmmE64)
INIT_CPU(MulpdXmmXmm, mulpdXmmXmm)
INIT_CPU(MulpdXmmE128, mulpdXmmE128)
INIT_CPU(MulsdXmmXmm, mulsdXmmXmm)
INIT_CPU(MulsdXmmE64, mulsdXmmE64)
INIT_CPU(DivpdXmmXmm, divpdXmmXmm)
INIT_CPU(DivpdXmmE128, divpdXmmE128)
INIT_CPU(DivsdXmmXmm, divsdXmmXmm)
INIT_CPU(DivsdXmmE64, divsdXmmE64)
INIT_CPU(MaxpdXmmXmm, maxpdXmmXmm)
INIT_CPU(MaxpdXmmE128, maxpdXmmE128)
INIT_CPU(MaxsdXmmXmm, maxsdXmmXmm)
INIT_CPU(MaxsdXmmE64, maxsdXmmE64)
INIT_CPU(MinpdXmmXmm, minpdXmmXmm)
INIT_CPU(MinpdXmmE128, minpdXmmE128)
INIT_CPU(MinsdXmmXmm, minsdXmmXmm)
INIT_CPU(MinsdXmmE64, minsdXmmE64)
INIT_CPU(PaddbXmmXmm, paddbXmmXmm)
INIT_CPU(PaddbXmmE128, paddbXmmE128)
INIT_CPU(PaddwXmmXmm, paddwXmmXmm)
INIT_CPU(PaddwXmmE128, paddwXmmE128)
INIT_CPU(PadddXmmXmm, padddXmmXmm)
INIT_CPU(PadddXmmE128, padddXmmE128)
INIT_CPU(PaddqMmxMmx, paddqMmxMmx)
INIT_CPU(PaddqMmxE64, paddqMmxE64)
INIT_CPU(PaddqXmmXmm, paddqXmmXmm)
INIT_CPU(PaddqXmmE128, paddqXmmE128)
INIT_CPU(PaddsbXmmXmm, paddsbXmmXmm)
INIT_CPU(PaddsbXmmE128, paddsbXmmE128)
INIT_CPU(PaddswXmmXmm, paddswXmmXmm)
INIT_CPU(PaddswXmmE128, paddswXmmE128)
INIT_CPU(PaddusbXmmXmm, paddusbXmmXmm)
INIT_CPU(PaddusbXmmE128, paddusbXmmE128)
INIT_CPU(PadduswXmmXmm, padduswXmmXmm)
INIT_CPU(PadduswXmmE128, padduswXmmE128)
INIT_CPU(PsubbXmmXmm, psubbXmmXmm)
INIT_CPU(PsubbXmmE128, psubbXmmE128)
INIT_CPU(PsubwXmmXmm, psubwXmmXmm)
INIT_CPU(PsubwXmmE128, psubwXmmE128)
INIT_CPU(PsubdXmmXmm, psubdXmmXmm)
INIT_CPU(PsubdXmmE128, psubdXmmE128)
INIT_CPU(PsubqMmxMmx, psubqMmxMmx)
INIT_CPU(PsubqMmxE64, psubqMmxE64)
INIT_CPU(PsubqXmmXmm, psubqXmmXmm)
INIT_CPU(PsubqXmmE128, psubqXmmE128)
INIT_CPU(PsubsbXmmXmm, psubsbXmmXmm)
INIT_CPU(PsubsbXmmE128, psubsbXmmE128)
INIT_CPU(PsubswXmmXmm, psubswXmmXmm)
INIT_CPU(PsubswXmmE128, psubswXmmE128)
INIT_CPU(PsubusbXmmXmm, psubusbXmmXmm)
INIT_CPU(PsubusbXmmE128, psubusbXmmE128)
INIT_CPU(PsubuswXmmXmm, psubuswXmmXmm)
INIT_CPU(PsubuswXmmE128, psubuswXmmE128)
INIT_CPU(PmaddwdXmmXmm, pmaddwdXmmXmm)
INIT_CPU(PmaddwdXmmE128, pmaddwdXmmE128)
INIT_CPU(PmulhwXmmXmm, pmulhwXmmXmm)
INIT_CPU(PmulhwXmmE128, pmulhwXmmE128)
INIT_CPU(PmullwXmmXmm, pmullwXmmXmm)
INIT_CPU(PmullwXmmE128, pmullwXmmE128)
INIT_CPU(PmuludqMmxMmx, pmuludqMmxMmx)
INIT_CPU(PmuludqMmxE64, pmuludqMmxE64)
INIT_CPU(PmuludqXmmXmm, pmuludqXmmXmm)
INIT_CPU(PmuludqXmmE128, pmuludqXmmE128)
INIT_CPU(SqrtpdXmmXmm, sqrtpdXmmXmm)
INIT_CPU(SqrtpdXmmE128, sqrtpdXmmE128)
INIT_CPU(SqrtsdXmmXmm, sqrtsdXmmXmm)
INIT_CPU(SqrtsdXmmE64, sqrtsdXmmE64)
INIT_CPU(AndnpdXmmXmm, andnpdXmmXmm)
INIT_CPU(AndnpdXmmE128, andnpdXmmE128)
INIT_CPU(AndpdXmmXmm, andpdXmmXmm)
INIT_CPU(AndpdXmmE128, andpdXmmE128)
INIT_CPU(PandXmmXmm, pandXmmXmm)
INIT_CPU(PandXmmE128, pandXmmE128)
INIT_CPU(PandnXmmXmm, pandnXmmXmm)
INIT_CPU(PandnXmmE128, pandnXmmE128)
INIT_CPU(PorXmmXmm, porXmmXmm)
INIT_CPU(PorXmmXmmE128, porXmmXmmE128)
INIT_CPU(PslldqXmm, pslldqXmm)
INIT_CPU(PsllqXmm, psllqXmm)
INIT_CPU(PsllqXmmXmm, psllqXmmXmm)
INIT_CPU(PsllqXmmE128, psllqXmmE128)
INIT_CPU(PslldXmm, pslldXmm)
INIT_CPU(PslldXmmXmm, pslldXmmXmm)
INIT_CPU(PslldXmmE128, pslldXmmE128)
INIT_CPU(PsllwXmm, psllwXmm)
INIT_CPU(PsllwXmmXmm, psllwXmmXmm)
INIT_CPU(PsllwXmmE128, psllwXmmE128)
INIT_CPU(PsradXmm, psradXmm)
INIT_CPU(PsradXmmXmm, psradXmmXmm)
INIT_CPU(PsradXmmE128, psradXmmE128)
INIT_CPU(PsrawXmm, psrawXmm)
INIT_CPU(PsrawXmmXmm, psrawXmmXmm)
INIT_CPU(PsrawXmmE128, psrawXmmE128)
INIT_CPU(PsrldqXmm, psrldqXmm)
INIT_CPU(PsrlqXmm, psrlqXmm)
INIT_CPU(PsrlqXmmXmm, psrlqXmmXmm)
INIT_CPU(PsrlqXmmE128, psrlqXmmE128)
INIT_CPU(PsrldXmm, psrldXmm)
INIT_CPU(PsrldXmmXmm, psrldXmmXmm)
INIT_CPU(PsrldXmmE128, psrldXmmE128)
INIT_CPU(PsrlwXmm, psrlwXmm)
INIT_CPU(PsrlwXmmXmm, psrlwXmmXmm)
INIT_CPU(PsrlwXmmE128, psrlwXmmE128)
INIT_CPU(PxorXmmXmm, pxorXmmXmm)
INIT_CPU(PxorXmmE128, pxorXmmE128)
INIT_CPU(OrpdXmmXmm, orpdXmmXmm)
INIT_CPU(OrpdXmmE128, orpdXmmE128)
INIT_CPU(XorpdXmmXmm, xorpdXmmXmm)
INIT_CPU(XorpdXmmE128, xorpdXmmE128)
INIT_CPU(CmppdXmmXmm, cmppdXmmXmm)
INIT_CPU(CmppdXmmE128, cmppdXmmE128)
INIT_CPU(CmpsdXmmXmm, cmpsdXmmXmm)
INIT_CPU(CmpsdXmmE64, cmpsdXmmE64)
INIT_CPU(ComisdXmmXmm, comisdXmmXmm)
INIT_CPU(ComisdXmmE64, comisdXmmE64)
INIT_CPU(UcomisdXmmXmm, ucomisdXmmXmm)
INIT_CPU(UcomisdXmmE64, ucomisdXmmE64)
INIT_CPU(PcmpgtbXmmXmm, pcmpgtbXmmXmm)
INIT_CPU(PcmpgtbXmmE128, pcmpgtbXmmE128)
INIT_CPU(PcmpgtwXmmXmm, pcmpgtwXmmXmm)
INIT_CPU(PcmpgtwXmmE128, pcmpgtwXmmE128)
INIT_CPU(PcmpgtdXmmXmm, pcmpgtdXmmXmm)
INIT_CPU(PcmpgtdXmmE128, pcmpgtdXmmE128)
INIT_CPU(PcmpeqbXmmXmm, pcmpeqbXmmXmm)
INIT_CPU(PcmpeqbXmmE128, pcmpeqbXmmE128)
INIT_CPU(PcmpeqwXmmXmm, pcmpeqwXmmXmm)
INIT_CPU(PcmpeqwXmmE128, pcmpeqwXmmE128)
INIT_CPU(PcmpeqdXmmXmm, pcmpeqdXmmXmm)
INIT_CPU(PcmpeqdXmmE128, pcmpeqdXmmE128)
INIT_CPU(Cvtdq2pdXmmXmm, cvtdq2pdXmmXmm)
INIT_CPU(Cvtdq2pdXmmE128, cvtdq2pdXmmE128)
INIT_CPU(Cvtdq2psXmmXmm, cvtdq2psXmmXmm)
INIT_CPU(Cvtdq2psXmmE128, cvtdq2psXmmE128)
INIT_CPU(Cvtpd2piMmxXmm, cvtpd2piMmxXmm)
INIT_CPU(Cvtpd2piMmxE128, cvtpd2piMmxE128)
INIT_CPU(Cvtpd2dqXmmXmm, cvtpd2dqXmmXmm)
INIT_CPU(Cvtpd2dqXmmE128, cvtpd2dqXmmE128)
INIT_CPU(Cvtpd2psXmmXmm, cvtpd2psXmmXmm)
INIT_CPU(Cvtpd2psXmmE128, cvtpd2psXmmE128)
INIT_CPU(Cvtpi2pdXmmMmx, cvtpi2pdXmmMmx)
INIT_CPU(Cvtpi2pdXmmE64, cvtpi2pdXmmE64)
INIT_CPU(Cvtps2dqXmmXmm, cvtps2dqXmmXmm)
INIT_CPU(Cvtps2dqXmmE128, cvtps2dqXmmE128)
INIT_CPU(Cvtps2pdXmmXmm, cvtps2pdXmmXmm)
INIT_CPU(Cvtps2pdXmmE64, cvtps2pdXmmE64)
INIT_CPU(Cvtsd2siR32Xmm, cvtsd2siR32Xmm)
INIT_CPU(Cvtsd2siR32E64, cvtsd2siR32E64)
INIT_CPU(Cvtsd2ssXmmXmm, cvtsd2ssXmmXmm)
INIT_CPU(Cvtsd2ssXmmE64, cvtsd2ssXmmE64)
INIT_CPU(Cvtsi2sdXmmR32, cvtsi2sdXmmR32)
INIT_CPU(Cvtsi2sdXmmE32, cvtsi2sdXmmE32)
INIT_CPU(Cvtss2sdXmmXmm, cvtss2sdXmmXmm)
INIT_CPU(Cvtss2sdXmmE32, cvtss2sdXmmE32)
INIT_CPU(Cvttpd2piMmxXmm, cvttpd2piMmxXmm)
INIT_CPU(Cvttpd2piMmE128, cvttpd2piMmE128)
INIT_CPU(Cvttpd2dqXmmXmm, cvttpd2dqXmmXmm)
INIT_CPU(Cvttpd2dqXmmE128, cvttpd2dqXmmE128)
INIT_CPU(Cvttps2dqXmmXmm, cvttps2dqXmmXmm)
INIT_CPU(Cvttps2dqXmmE128, cvttps2dqXmmE128)
INIT_CPU(Cvttsd2siR32Xmm, cvttsd2siR32Xmm)
INIT_CPU(Cvttsd2siR32E64, cvttsd2siR32E64)
INIT_CPU(MovqXmmXmm, movqXmmXmm)
INIT_CPU(MovqE64Xmm, movqE64Xmm)
INIT_CPU(MovqXmmE64, movqXmmE64)
INIT_CPU(MovsdXmmXmm, movsdXmmXmm)
INIT_CPU(MovsdXmmE64, movsdXmmE64)
INIT_CPU(MovsdE64Xmm, movsdE64Xmm)
INIT_CPU(MovapdXmmXmm, movapdXmmXmm)
INIT_CPU(MovapdXmmE128, movapdXmmE128)
INIT_CPU(MovapdE128Xmm, movapdE128Xmm)
INIT_CPU(MovupdXmmXmm, movupdXmmXmm)
INIT_CPU(MovupdXmmE128, movupdXmmE128)
INIT_CPU(MovupdE128Xmm, movupdE128Xmm)
INIT_CPU(MovhpdXmmE64, movhpdXmmE64)
INIT_CPU(MovhpdE64Xmm, movhpdE64Xmm)
INIT_CPU(MovlpdXmmE64, movlpdXmmE64)
INIT_CPU(MovlpdE64Xmm, movlpdE64Xmm)
INIT_CPU(MovmskpdR32Xmm, movmskpdR32Xmm)
INIT_CPU(MovdXmmR32, movdXmmR32)
INIT_CPU(MovdXmmE32, movdXmmE32)
INIT_CPU(MovdR32Xmm, movdR32Xmm)
INIT_CPU(MovdE32Xmm, movdE32Xmm)
INIT_CPU(MovdqaXmmXmm, movdqaXmmXmm)
INIT_CPU(MovdqaXmmE128, movdqaXmmE128)
INIT_CPU(MovdqaE128Xmm, movdqaE128Xmm)
INIT_CPU(MovdquXmmXmm, movdquXmmXmm)
INIT_CPU(MovdquXmmE128, movdquXmmE128)
INIT_CPU(MovdquE128Xmm, movdquE128Xmm)
INIT_CPU(Movdq2qMmxXmm, movdq2qMmxXmm)
INIT_CPU(Movq2dqXmmMmx, movq2dqXmmMmx)
INIT_CPU(MovntpdE128Xmm, movntpdE128Xmm)
INIT_CPU(MovntdqE128Xmm, movntdqE128Xmm)
INIT_CPU(MovntiE32R32, movntiE32R32)
INIT_CPU(MaskmovdquE128XmmXmm, maskmovdquE128XmmXmm)
INIT_CPU(PshufdXmmXmm, pshufdXmmXmm)
INIT_CPU(PshufdXmmE128, pshufdXmmE128)
INIT_CPU(PshufhwXmmXmm, pshufhwXmmXmm)
INIT_CPU(PshufhwXmmE128, pshufhwXmmE128)
INIT_CPU(PshuflwXmmXmm, pshuflwXmmXmm)
INIT_CPU(PshuflwXmmE128, pshuflwXmmE128)
INIT_CPU(UnpckhpdXmmXmm, unpckhpdXmmXmm)
INIT_CPU(UnpckhpdXmmE128, unpckhpdXmmE128)
INIT_CPU(UnpcklpdXmmXmm, unpcklpdXmmXmm)
INIT_CPU(UnpcklpdXmmE128, unpcklpdXmmE128)
INIT_CPU(PunpckhbwXmmXmm, punpckhbwXmmXmm)
INIT_CPU(PunpckhbwXmmE128, punpckhbwXmmE128)
INIT_CPU(PunpckhwdXmmXmm, punpckhwdXmmXmm)
INIT_CPU(PunpckhwdXmmE128, punpckhwdXmmE128)
INIT_CPU(PunpckhdqXmmXmm, punpckhdqXmmXmm)
INIT_CPU(PunpckhdqXmmE128, punpckhdqXmmE128)
INIT_CPU(PunpckhqdqXmmXmm, punpckhqdqXmmXmm)
INIT_CPU(PunpckhqdqXmmE128, punpckhqdqXmmE128)
INIT_CPU(PunpcklbwXmmXmm, punpcklbwXmmXmm)
INIT_CPU(PunpcklbwXmmE128, punpcklbwXmmE128)
INIT_CPU(PunpcklwdXmmXmm, punpcklwdXmmXmm)
INIT_CPU(PunpcklwdXmmE128, punpcklwdXmmE128)
INIT_CPU(PunpckldqXmmXmm, punpckldqXmmXmm)
INIT_CPU(PunpckldqXmmE128, punpckldqXmmE128)
INIT_CPU(PunpcklqdqXmmXmm, punpcklqdqXmmXmm)
INIT_CPU(PunpcklqdqXmmE128, punpcklqdqXmmE128)
INIT_CPU(PackssdwXmmXmm, packssdwXmmXmm)
INIT_CPU(PackssdwXmmE128, packssdwXmmE128)
INIT_CPU(PacksswbXmmXmm, packsswbXmmXmm)
INIT_CPU(PacksswbXmmE128, packsswbXmmE128)
INIT_CPU(PackuswbXmmXmm, packuswbXmmXmm)
INIT_CPU(PackuswbXmmE128, packuswbXmmE128)
INIT_CPU(ShufpdXmmXmm, shufpdXmmXmm)
INIT_CPU(ShufpdXmmE128, shufpdXmmE128)
INIT_CPU(Pause, pause)
INIT_CPU(PavgbXmmXmm, pavgbXmmXmm)
INIT_CPU(PavgbXmmE128, pavgbXmmE128)
INIT_CPU(PavgwXmmXmm, pavgwXmmXmm)
INIT_CPU(PavgwXmmE128, pavgwXmmE128)
INIT_CPU(PsadbwXmmXmm, psadbwXmmXmm)
INIT_CPU(PsadbwXmmE128, psadbwXmmE128)
INIT_CPU(PextrwR32Xmm, pextrwR32Xmm)
INIT_CPU(PextrwE16Xmm, pextrwE16Xmm)
INIT_CPU(PinsrwXmmR32, pinsrwXmmR32)
INIT_CPU(PinsrwXmmE16, pinsrwXmmE16)
INIT_CPU(PmaxswXmmXmm, pmaxswXmmXmm)
INIT_CPU(PmaxswXmmE128, pmaxswXmmE128)
INIT_CPU(PmaxubXmmXmm, pmaxubXmmXmm)
INIT_CPU(PmaxubXmmE128, pmaxubXmmE128)
INIT_CPU(PminswXmmXmm, pminswXmmXmm)
INIT_CPU(PminswXmmE128, pminswXmmE128)
INIT_CPU(PminubXmmXmm, pminubXmmXmm)
INIT_CPU(PminubXmmE128, pminubXmmE128)
INIT_CPU(PmovmskbR32Xmm, pmovmskbR32Xmm)
INIT_CPU(PmulhuwXmmXmm, pmulhuwXmmXmm)
INIT_CPU(PmulhuwXmmE128, pmulhuwXmmE128)
INIT_CPU(Lfence, lfence)
INIT_CPU(Mfence, mfence)
INIT_CPU(Clflush, clflush)
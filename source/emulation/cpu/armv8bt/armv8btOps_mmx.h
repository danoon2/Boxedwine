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

#ifndef __ARMV8BTOPS_MMX_H__
#define __ARMV8BTOPS_MMX_H__

void opPunpcklbwMmx(Armv8btAsm* data);
void opPunpcklbwE64(Armv8btAsm* data);
void opPunpcklwdMmx(Armv8btAsm* data);
void opPunpcklwdE64(Armv8btAsm* data);
void opPunpckldqMmx(Armv8btAsm* data);
void opPunpckldqE64(Armv8btAsm* data);
void opPacksswbMmx(Armv8btAsm* data);
void opPacksswbE64(Armv8btAsm* data);
void opPcmpgtbMmx(Armv8btAsm* data);
void opPcmpgtbE64(Armv8btAsm* data);
void opPcmpgtwMmx(Armv8btAsm* data);
void opPcmpgtwE64(Armv8btAsm* data);
void opPcmpgtdMmx(Armv8btAsm* data);
void opPcmpgtdE64(Armv8btAsm* data);
void opPackuswbMmx(Armv8btAsm* data);
void opPackuswbE64(Armv8btAsm* data);
void opPunpckhbwMmx(Armv8btAsm* data);
void opPunpckhbwE64(Armv8btAsm* data);
void opPunpckhwdMmx(Armv8btAsm* data);
void opPunpckhwdE64(Armv8btAsm* data);
void opPunpckhdqMmx(Armv8btAsm* data);
void opPunpckhdqE64(Armv8btAsm* data);
void opPackssdwMmx(Armv8btAsm* data);
void opPackssdwE64(Armv8btAsm* data);
void opMovPqR32(Armv8btAsm* data);
void opMovPqE32(Armv8btAsm* data);
void opMovPqMmx(Armv8btAsm* data);
void opMovPqE64(Armv8btAsm* data);
void opPsrlw(Armv8btAsm* data);
void opPsraw(Armv8btAsm* data);
void opPsllw(Armv8btAsm* data);
void opPsrld(Armv8btAsm* data);
void opPsrad(Armv8btAsm* data);
void opPslld(Armv8btAsm* data);
void opPsrlq(Armv8btAsm* data);
void opPsllq(Armv8btAsm* data);
void opPcmpeqbMmx(Armv8btAsm* data);
void opPcmpeqbE64(Armv8btAsm* data);
void opPcmpeqwMmx(Armv8btAsm* data);
void opPcmpeqwE64(Armv8btAsm* data);
void opPcmpeqdMmx(Armv8btAsm* data);
void opPcmpeqdE64(Armv8btAsm* data);
void opMovR32Pq(Armv8btAsm* data);
void opMovE32Pq(Armv8btAsm* data);
void opMovMmxPq(Armv8btAsm* data);
void opMovE64Pq(Armv8btAsm* data);
void opPsrlwMmx(Armv8btAsm* data);
void opPsrlwE64(Armv8btAsm* data);
void opPsrldMmx(Armv8btAsm* data);
void opPsrldE64(Armv8btAsm* data);
void opPsrlqMmx(Armv8btAsm* data);
void opPsrlqE64(Armv8btAsm* data);
void opPmullwMmx(Armv8btAsm* data);
void opPmullwE64(Armv8btAsm* data);
void opPsubusbMmx(Armv8btAsm* data);
void opPsubusbE64(Armv8btAsm* data);
void opPsubuswMmx(Armv8btAsm* data);
void opPsubuswE64(Armv8btAsm* data);
void opPandMmx(Armv8btAsm* data);
void opPandE64(Armv8btAsm* data);
void opPaddusbMmx(Armv8btAsm* data);
void opPaddusbE64(Armv8btAsm* data);
void opPadduswMmx(Armv8btAsm* data);
void opPadduswE64(Armv8btAsm* data);
void opPandnMmx(Armv8btAsm* data);
void opPandnE64(Armv8btAsm* data);
void opPsrawMmx(Armv8btAsm* data);
void opPsrawE64(Armv8btAsm* data);
void opPsradMmx(Armv8btAsm* data);
void opPsradE64(Armv8btAsm* data);
void opPmulhwMmx(Armv8btAsm* data);
void opPmulhwE64(Armv8btAsm* data);
void opPsubsbMmx(Armv8btAsm* data);
void opPsubsbE64(Armv8btAsm* data);
void opPsubswMmx(Armv8btAsm* data);
void opPsubswE64(Armv8btAsm* data);
void opPorMmx(Armv8btAsm* data);
void opPorE64(Armv8btAsm* data);
void opPaddsbMmx(Armv8btAsm* data);
void opPaddsbE64(Armv8btAsm* data);
void opPaddswMmx(Armv8btAsm* data);
void opPaddswE64(Armv8btAsm* data);
void opPxorMmx(Armv8btAsm* data);
void opPxorE64(Armv8btAsm* data);
void opPsllwMmx(Armv8btAsm* data);
void opPsllwE64(Armv8btAsm* data);
void opPslldMmx(Armv8btAsm* data);
void opPslldE64(Armv8btAsm* data);
void opPsllqMmx(Armv8btAsm* data);
void opPsllqE64(Armv8btAsm* data);
void opPmaddwdMmx(Armv8btAsm* data);
void opPmaddwdE64(Armv8btAsm* data);
void opPsubbMmx(Armv8btAsm* data);
void opPsubbE64(Armv8btAsm* data);
void opPsubwMmx(Armv8btAsm* data);
void opPsubwE64(Armv8btAsm* data);
void opPsubdMmx(Armv8btAsm* data);
void opPsubdE64(Armv8btAsm* data);
void opPaddbMmx(Armv8btAsm* data);
void opPaddbE64(Armv8btAsm* data);
void opPaddwMmx(Armv8btAsm* data);
void opPaddwE64(Armv8btAsm* data);
void opPadddMmx(Armv8btAsm* data);
void opPadddE64(Armv8btAsm* data);
void opPavgbMmxMmx(Armv8btAsm* data);
void opPavgbMmxE64(Armv8btAsm* data);
void opPavgwMmxMmx(Armv8btAsm* data);
void opPavgwMmxE64(Armv8btAsm* data);
void opPsadbwMmxMmx(Armv8btAsm* data);
void opPsadbwMmxE64(Armv8btAsm* data);
void opPextrwR32Mmx(Armv8btAsm* data);
void opPextrwE16Mmx(Armv8btAsm* data);
void opPinsrwMmxR32(Armv8btAsm* data);
void opPinsrwMmxE16(Armv8btAsm* data);
void opPmaxswMmxMmx(Armv8btAsm* data);
void opPmaxswMmxE64(Armv8btAsm* data);
void opPmaxubMmxMmx(Armv8btAsm* data);
void opPmaxubMmxE64(Armv8btAsm* data);
void opPminswMmxMmx(Armv8btAsm* data);
void opPminswMmxE64(Armv8btAsm* data);
void opPminubMmxMmx(Armv8btAsm* data);
void opPminubMmxE64(Armv8btAsm* data);
void opPmovmskbR32Mmx(Armv8btAsm* data);
void opPmulhuwMmxMmx(Armv8btAsm* data);
void opPmulhuwMmxE64(Armv8btAsm* data);
void opMaskmovqEDIMmxMmx(Armv8btAsm* data);
void opMovntqE64Mmx(Armv8btAsm* data);
void opPaddqMmxMmx(Armv8btAsm* data);
void opPaddqMmxE64(Armv8btAsm* data);
void opPsubqMmxMmx(Armv8btAsm* data);
void opPsubqMmxE64(Armv8btAsm* data);
void opPmuludqMmxMmx(Armv8btAsm* data);
void opPmuludqMmxE64(Armv8btAsm* data);
void opMovdq2qMmxXmm(Armv8btAsm* data);
void opMovq2dqXmmMmx(Armv8btAsm* data);
#endif
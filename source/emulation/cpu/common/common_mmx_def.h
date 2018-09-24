/*
 *  Copyright (C) 2016  The BoxedWine Team
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

/* State Management */
MMX_0(emms)

/* Data Movement */
MMX_RR(movPqR32)
MMX_RE(movPqE32)
MMX_RR(movR32Pq)
MMX_RE(movE32Pq)
MMX_RR(movPqMmx)
MMX_RE(movPqE64)
MMX_RE(movE64Pq)
MMX_RR(movMmxPq)

/* Boolean Logic */
MMX_RR(pxorMmx)
MMX_RE(pxorE64)
MMX_RR(porMmx)
MMX_RE(porE64)
MMX_RR(pandMmx)
MMX_RE(pandE64)
MMX_RR(pandnMmx)
MMX_RE(pandnE64)

/* Shift */
MMX_RR(psllwMmx)
MMX_RE(psllwE64)
MMX_RR(psrlwMmx)
MMX_RE(psrlwE64)
MMX_RR(psrawMmx)
MMX_RE(psrawE64)
MMX_RI(psllw)
MMX_RI(psraw)
MMX_RI(psrlw)
MMX_RR(pslldMmx)
MMX_RE(pslldE64)
MMX_RR(psrldMmx)
MMX_RE(psrldE64)
MMX_RR(psradMmx)
MMX_RE(psradE64)
MMX_RI(pslld)
MMX_RI(psrad)
MMX_RI(psrld)
MMX_RR(psllqMmx)
MMX_RE(psllqE64)
MMX_RR(psrlqMmx)
MMX_RE(psrlqE64)
MMX_RI(psllq)
MMX_RI(psrlq)

/* Math */
MMX_RR(paddbMmx)
MMX_RE(paddbE64)
MMX_RR(paddwMmx)
MMX_RE(paddwE64)
MMX_RR(padddMmx)
MMX_RE(padddE64)
MMX_RR(paddsbMmx)
MMX_RE(paddsbE64)
MMX_RR(paddswMmx)
MMX_RE(paddswE64)
MMX_RR(paddusbMmx)
MMX_RE(paddusbE64)
MMX_RR(padduswMmx)
MMX_RE(padduswE64)
MMX_RR(psubbMmx)
MMX_RE(psubbE64)
MMX_RR(psubwMmx)
MMX_RE(psubwE64)
MMX_RR(psubdMmx)
MMX_RE(psubdE64)
MMX_RR(psubsbMmx)
MMX_RE(psubsbE64)
MMX_RR(psubswMmx)
MMX_RE(psubswE64)
MMX_RR(psubusbMmx)
MMX_RE(psubusbE64)
MMX_RR(psubuswMmx)
MMX_RE(psubuswE64)
MMX_RR(pmulhwMmx)
MMX_RE(pmulhwE64)
MMX_RR(pmullwMmx)
MMX_RE(pmullwE64)
MMX_RR(pmaddwdMmx)
MMX_RE(pmaddwdE64)

/* Comparison */
MMX_RR(pcmpeqbMmx)
MMX_RE(pcmpeqbE64)
MMX_RR(pcmpeqwMmx)
MMX_RE(pcmpeqwE64)
MMX_RR(pcmpeqdMmx)
MMX_RE(pcmpeqdE64)
MMX_RR(pcmpgtbMmx)
MMX_RE(pcmpgtbE64)
MMX_RR(pcmpgtwMmx)
MMX_RE(pcmpgtwE64)
MMX_RR(pcmpgtdMmx)
MMX_RE(pcmpgtdE64)

/* Data Packing */
MMX_RR(packsswbMmx)
MMX_RE(packsswbE64)
MMX_RR(packssdwMmx)
MMX_RE(packssdwE64)
MMX_RR(packuswbMmx)
MMX_RE(packuswbE64)
MMX_RR(punpckhbwMmx)
MMX_RE(punpckhbwE64)
MMX_RR(punpckhwdMmx)
MMX_RE(punpckhwdE64)
MMX_RR(punpckhdqMmx)
MMX_RE(punpckhdqE64)
MMX_RR(punpcklbwMmx)
MMX_RE(punpcklbwE64)
MMX_RR(punpcklwdMmx)
MMX_RE(punpcklwdE64)
MMX_RR(punpckldqMmx)
MMX_RE(punpckldqE64)

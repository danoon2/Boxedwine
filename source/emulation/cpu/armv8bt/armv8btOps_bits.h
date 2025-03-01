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

#ifndef __ARMV8BTOPS_BITS_H__
#define __ARMV8BTOPS_BITS_H__

void opBtR16R16(Armv8btAsm* data);
void opBtE16R16(Armv8btAsm* data);
void opBtR32R32(Armv8btAsm* data);
void opBtE32R32(Armv8btAsm* data);
void opBtsR16R16(Armv8btAsm* data);
void opBtsE16R16(Armv8btAsm* data);
void opBtsR32R32(Armv8btAsm* data);
void opBtsE32R32(Armv8btAsm* data);
void opBtrR16R16(Armv8btAsm* data);
void opBtrE16R16(Armv8btAsm* data);
void opBtrR32R32(Armv8btAsm* data);
void opBtrE32R32(Armv8btAsm* data);
void opBsfR16R16(Armv8btAsm* data);
void opBsfR16E16(Armv8btAsm* data);
void opBsfR32R32(Armv8btAsm* data);
void opBsfR32E32(Armv8btAsm* data);
void opBsrR16R16(Armv8btAsm* data);
void opBsrR16E16(Armv8btAsm* data);
void opBsrR32R32(Armv8btAsm* data);
void opBsrR32E32(Armv8btAsm* data);
void opBtcR16R16(Armv8btAsm* data);
void opBtcE16R16(Armv8btAsm* data);
void opBtcR32R32(Armv8btAsm* data);
void opBtcE32R32(Armv8btAsm* data);
void opBtR16(Armv8btAsm* data);
void opBtE16(Armv8btAsm* data);
void opBtsR16(Armv8btAsm* data);
void opBtsE16(Armv8btAsm* data);
void opBtrR16(Armv8btAsm* data);
void opBtrE16(Armv8btAsm* data);
void opBtcR16(Armv8btAsm* data);
void opBtcE16(Armv8btAsm* data);
void opBtR32(Armv8btAsm* data);
void opBtE32(Armv8btAsm* data);
void opBtsR32(Armv8btAsm* data);
void opBtsE32(Armv8btAsm* data);
void opBtrR32(Armv8btAsm* data);
void opBtrE32(Armv8btAsm* data);
void opBtcR32(Armv8btAsm* data);
void opBtcE32(Armv8btAsm* data);

#endif

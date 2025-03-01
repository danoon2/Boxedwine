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

#ifndef __ARMV8BTOPS_STRING_H__
#define __ARMV8BTOPS_STRING_H__

void opCmpsb(Armv8btAsm* data);
void opCmpsw(Armv8btAsm* data);
void opCmpsd(Armv8btAsm* data);

void opMovsb(Armv8btAsm* data);
void opMovsw(Armv8btAsm* data);
void opMovsd(Armv8btAsm* data);

void opStosb(Armv8btAsm* data);
void opStosw(Armv8btAsm* data);
void opStosd(Armv8btAsm* data);

void opLodsb(Armv8btAsm* data);
void opLodsw(Armv8btAsm* data);
void opLodsd(Armv8btAsm* data);

void opScasb(Armv8btAsm* data);
void opScasw(Armv8btAsm* data);
void opScasd(Armv8btAsm* data);

#endif
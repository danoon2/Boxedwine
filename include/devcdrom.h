/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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

#ifndef __DEVCDROM_H__
#define __DEVCDROM_H__

class FsIso;

FsOpenNode* openDevCDROM(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);

// The emulated CD-ROM drive holds one disc from a list of ISO images, or none while
// the tray is open. The disc in the drive is mounted at mountPath and Wine reaches it
// through driveLink (its dosdevices drive letter, empty for none). The link only
// exists while a disc is in the drive, so Wine sees an empty drive after an eject.
void cdromInit(const std::vector<BString>& discs, const BString& mountPath, const BString& driveLink);
bool cdromInsert(U32 index); // ejects the current disc first
void cdromEject();
void cdromNextDisc(); // inserts the disc after the last one inserted, wrapping around
// the mounted ISO if path is on it, so statfs can report it as iso9660
std::shared_ptr<FsIso> getCdromForPath(const BString& path);

#endif

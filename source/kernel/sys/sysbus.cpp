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

#include "boxedwine.h"
#include "../../io/fsfilenode.h"
#include "kstat.h"

std::shared_ptr<FsNode> createSysBusUsb(const std::shared_ptr<FsNode> busNode);

std::shared_ptr<FsNode> createSysBus(const std::shared_ptr<FsNode> sysNode) {
	std::shared_ptr<FsNode> busNode = sysNode->getChildByName(B("bus"));

	if (!busNode) {
		busNode = Fs::addFileNode(B("/sys/bus"), B(""), B(""), true, sysNode);
	}
	std::shared_ptr<FsNode> usbNode = createSysBusUsb(busNode);

	return busNode;
}
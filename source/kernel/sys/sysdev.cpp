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

std::shared_ptr<FsNode> createSysDev(const std::shared_ptr<FsNode> sysNode) {
	std::shared_ptr<FsNode> devNode = sysNode->getChildByName(B("dev"));
	if (!devNode) {
		devNode = Fs::addFileNode(B("/sys/dev"), B(""), B(""), true, sysNode);
	}

	std::shared_ptr<FsNode> blockNode = devNode->getChildByName(B("block"));
	if (!blockNode) {
		blockNode = Fs::addFileNode(B("/sys/dev/block"), B(""), B(""), true, devNode);
	}

	Fs::addDynamicLinkFile(B("/sys/dev/block/8:0"), k_mdev(0, 0), blockNode, false, B("../../devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda"));
	Fs::addDynamicLinkFile(B("/sys/dev/block/8:1"), k_mdev(0, 0), blockNode, false, B("../../devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1"));

	std::shared_ptr<FsNode> charNode = devNode->getChildByName(B("char"));
	if (!charNode) {
		charNode = Fs::addFileNode(B("/sys/dev/char"), B(""), B(""), true, devNode);
	}
	Fs::addDynamicLinkFile(B("/sys/dev/char/189:0"), k_mdev(0, 0), charNode, false, B("../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1"));
	Fs::addDynamicLinkFile(B("/sys/dev/char/189:1"), k_mdev(0, 0), charNode, false, B("../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1"));

	return devNode;
}
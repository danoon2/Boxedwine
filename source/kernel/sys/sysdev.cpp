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
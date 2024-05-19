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
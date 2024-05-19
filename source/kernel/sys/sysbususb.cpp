#include "boxedwine.h"
#include "../../io/fsfilenode.h"
#include "kstat.h"

void sysAddUsbDrivers(const std::shared_ptr<FsNode>& usbNode) {
	std::shared_ptr<FsNode> usbDriversNode = usbNode->getChildByName(B("drivers"));
	if (!usbDriversNode) {
		usbDriversNode = Fs::addFileNode(B("/sys/bus/usb/drivers"), B(""), B(""), true, usbNode);
	}

	std::shared_ptr<FsNode> usbDriverNode = usbDriversNode->getChildByName(B("usb"));
	if (!usbDriverNode) {
		usbDriverNode = Fs::addFileNode(B("/sys/bus/usb/drivers/usb"), B(""), B(""), true, usbDriversNode);
	}

	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/usb/1-1"), k_mdev(0, 0), usbDriverNode, false, B("../../../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1"));
	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/usb/module"), k_mdev(0, 0), usbDriverNode, false, B("../../../../module/usbcore"));
	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/usb/usb1"), k_mdev(0, 0), usbDriverNode, false, B("../../../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1"));

	std::shared_ptr<FsNode> hubDriverNode = usbDriversNode->getChildByName(B("hub"));
	if (!hubDriverNode) {
		hubDriverNode = Fs::addFileNode(B("/sys/bus/usb/drivers/hub"), B(""), B(""), true, usbDriversNode);
	}

	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/hub/1-0:1.0"), k_mdev(0, 0), hubDriverNode, false, B("../../../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0"));
	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/usb/module"), k_mdev(0, 0), hubDriverNode, false, B("../../../../module/usbcore"));

	std::shared_ptr<FsNode> hidDriverNode = usbDriversNode->getChildByName(B("usbhid"));
	if (!hidDriverNode) {
		hidDriverNode = Fs::addFileNode(B("/sys/bus/usb/drivers/usbhid"), B(""), B(""), true, usbDriversNode);
	}

	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/hub/1-1:1.0"), k_mdev(0, 0), hidDriverNode, false, B("../../../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0"));
	Fs::addDynamicLinkFile(B("/sys/bus/usb/drivers/usb/module"), k_mdev(0, 0), hidDriverNode, false, B("../../../../module/usbhid"));
}

std::shared_ptr<FsNode> createSysBusUsb(const std::shared_ptr<FsNode> busNode) {
	std::shared_ptr<FsNode> usbNode = busNode->getChildByName(B("usb"));
	if (!usbNode) {
		usbNode = Fs::addFileNode(B("/sys/bus/usb"), B(""), B(""), true, busNode);
	}
	sysAddUsbDrivers(usbNode);
	return usbNode;
}
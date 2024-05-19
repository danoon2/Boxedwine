#include "boxedwine.h"
#include "../../io/fsfilenode.h"
#include "kstat.h"

void sysAddMouse(const std::shared_ptr<FsNode>& busNode, const std::shared_ptr<FsNode>& devicesNode) {
	std::shared_ptr<FsNode> usbNode = busNode->getChildByName(B("usb"));
	if (!usbNode) {
		usbNode = Fs::addFileNode(B("/sys/bus/usb"), B(""), B(""), true, busNode);
	}

	std::shared_ptr<FsNode> usbDevicesNode = usbNode->getChildByName(B("devices"));
	if (!usbDevicesNode) {
		usbDevicesNode = Fs::addFileNode(B("/sys/bus/usb/devices"), B(""), B(""), true, usbNode);
	}
	Fs::addDynamicLinkFile(B("/sys/bus/usb/devices/1-1"), k_mdev(0, 0), usbDevicesNode, false, B("../../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1"));
	Fs::addDynamicLinkFile(B("/sys/bus/usb/devices/usb1"), k_mdev(0, 0), usbDevicesNode, false, B("../../../devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1"));
	
	std::shared_ptr<FsNode> pciNode = devicesNode->getChildByName(B("pci0000:00"));
	if (!pciNode) {
		pciNode = Fs::addFileNode(B("/sys/devices/pci0000:00"), B(""), B(""), true, devicesNode);
	}
	std::shared_ptr<FsNode> pciSub1Node = pciNode->getChildByName(B("0000:00:11.0"));
	if (!pciSub1Node) {
		pciSub1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0"), B(""), B(""), true, pciNode);
	}
	std::shared_ptr<FsNode> pciSub2Node = pciSub1Node->getChildByName(B("0000:02:00.0"));
	if (!pciSub2Node) {
		pciSub2Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0"), B(""), B(""), true, pciSub1Node);
	}
	std::shared_ptr<FsNode> usb1Node = pciSub2Node->getChildByName(B("usb1"));
	if (!usb1Node) {
		usb1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1"), B(""), B(""), true, pciSub2Node);
	}
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/authorized"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/authorized_default"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/avoid_reset_quirk"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bcdDevice"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0419"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bConfigurationValue"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bDeviceClass"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("09"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bDeviceProtocol"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bDeviceSubClass"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bmAttributes"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("e0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bMaxPacketSize0"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("64"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bMaxPower"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0mA"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bNumConfigurations"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/bNumInterfaces"), K__S_IREAD, k_mdev(0, 0), usb1Node, B(" 1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/busnum"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/configuration"), K__S_IREAD, k_mdev(0, 0), usb1Node, B(""));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/dev"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("189:0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/devnum"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/devpath"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/driver"), k_mdev(0, 0), usb1Node, false, B("../../../../../bus/usb/drivers/usb"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/idProduct"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0001"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/idVendor"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1d6b"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/interface_authorized_default"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/ltm_capable"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("no"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/manufacturer"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("Linux 4.19.0-9-686-pae uhci_hcd"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/maxchild"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("2"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/product"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("UHCI Host Controller"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/quirks"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0x0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/removable"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("unknown"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/rx_lanes"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/serial"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("0000:02:00.0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/speed"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("12"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/subsystem"), k_mdev(0, 0), usb1Node, false, B("../../../../../bus/usb"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/tx_lanes"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/uevent"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("MAJOR=189\nMINOR=0\nDEVNAME=bus/usb/001/001\nDEVTYPE=usb_device\nDRIVER=usb\nPRODUCT=1d6b/1/419\nTYPE=9/0/0\nBUSNUM=001\nDEVNUM=001"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/urbnum"), K__S_IREAD, k_mdev(0, 0), usb1Node, B("2637678"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/version"), K__S_IREAD, k_mdev(0, 0), usb1Node, B(" 1.10"));

	std::shared_ptr<FsNode> usb1Device0Node = usb1Node->getChildByName(B("1-0:1.0"));
	if (!usb1Device0Node) {
		usb1Device0Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0"), B(""), B(""), true, usb1Node);
	}

	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/authorized"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/bAlternateSetting"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B(" 0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/bInterfaceClass"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("09"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/bInterfaceNumber"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/bInterfaceProtocol"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/bInterfaceSubClass"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/bNumEndpoints"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("01"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/driver"), k_mdev(0, 0), usb1Device0Node, false, B("../../../../../../bus/usb/drivers/hub"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/modalias"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("usb:v1D6Bp0001d0419dc09dsc00dp00ic09isc00ip00in00"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/subsystem"), k_mdev(0, 0), usb1Device0Node, false, B("../../../../../../bus/usb"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/supports_autosuspend"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B("1"));

	std::shared_ptr<FsNode> usb1Port1Node = usb1Device0Node->getChildByName(B("usb1-port1"));
	if (!usb1Port1Node) {
		usb1Port1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/usb1-port1"), B(""), B(""), true, usb1Device0Node);
	}
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/usb1-port1/connect_type"), K__S_IREAD, k_mdev(0, 0), usb1Port1Node, B("unknown"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/usb1-port1/device"), k_mdev(0, 0), usb1Port1Node, false, B("../../1-1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/usb1-port1/over_current_count"), K__S_IREAD, k_mdev(0, 0), usb1Port1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/usb1-port1/quirks"), K__S_IREAD, k_mdev(0, 0), usb1Port1Node, B("00000000"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-0:1.0/usb1-port1/uevent"), K__S_IREAD, k_mdev(0, 0), usb1Port1Node, B(""));

	std::shared_ptr<FsNode> usb1Device1Node = usb1Node->getChildByName(B("1-1"));
	if (!usb1Device1Node) {
		usb1Device1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1"), B(""), B(""), true, usb1Node);
	}

	std::shared_ptr<FsNode> usb1Device1Sub1Node = usb1Node->getChildByName(B("1-1:1.0"));
	if (!usb1Device1Sub1Node) {
		usb1Device1Sub1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0"), B(""), B(""), true, usb1Device1Node);
	}

	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/authorized"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/bAlternateSetting"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B(" 0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/bInterfaceClass"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("03"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/bInterfaceNumber"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/bInterfaceProtocol"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("02"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/bInterfaceSubClass"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("01"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/bNumEndpoints"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("01"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/driver"), k_mdev(0, 0), usb1Device1Sub1Node, false, B("../../../../../../../bus/usb/drivers/usbhid"));
//	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/interface"), K__S_IREAD, k_mdev(0, 0), usb1Device0Node, B(""));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/modalias"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("usb:v0E0Fp0003d0103dc00dsc00dp00ic03isc01ip02in00"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/subsystem"), k_mdev(0, 0), usb1Device1Sub1Node, false, B("../../../../../../../bus/usb"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/supports_autosuspend"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/1-1:1.0/uevent"), K__S_IREAD, k_mdev(0, 0), usb1Device1Sub1Node, B("DEVTYPE=usb_interface\nDRIVER=usbhid\nPRODUCT=e0f/3/103\nTYPE=0/0/0\nINTERFACE=3/1/2\nMODALIAS=usb:v0E0Fp0003d0103dc00dsc00dp00ic03isc01ip02in00"));

	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/authorized"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/avoid_reset_quirk"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bcdDevice"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0103"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bConfigurationValue"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bDeviceClass"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bDeviceProtocol"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bDeviceSubClass"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bmAttributes"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("c0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bMaxPacketSize0"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("8"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bMaxPower"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0mA"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bNumConfigurations"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/bNumInterfaces"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B(" 1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/busnum"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/configuration"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("Boxedwine"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/dev"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("189:1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/devnum"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("2"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/devpath"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("1"));	
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/driver"), k_mdev(0, 0), usb1Device1Node, false, B("../../../../../../bus/usb/drivers/usb"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/idProduct"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0003"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/idVendor"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0e0f"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/ltm_capable"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("no"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/manufacturer"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("Boxedwine"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/maxchild"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/port"), k_mdev(0, 0), usb1Device1Node, false, B("../1-0:1.0/usb1-port1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/product"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("USB Mouse"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/quirks"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("0x0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/removable"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("unknown"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/rx_lanes"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/uevent"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("MAJOR=189\nMINOR=1\nDEVNAME=bus/usb/001/002\nDEVTYPE=usb_device\nDRIVER=usb\nPRODUCT=e0f/3/103\nTYPE=0/0/0\nBUSNUM=001\nDEVNUM=002"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/urbnum"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B("16"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb1/1-1/version"), K__S_IREAD, k_mdev(0, 0), usb1Device1Node, B(" 1.10"));
}
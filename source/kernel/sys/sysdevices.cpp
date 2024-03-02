#include "boxedwine.h"
#include "../../io/fsfilenode.h"
#include "kstat.h"

std::shared_ptr<FsNode> createSysDevices(const std::shared_ptr<FsNode> sysNode) {
	std::shared_ptr<FsNode> devicesNode = sysNode->getChildByName(B("devices"));
	if (!devicesNode) {
		devicesNode = Fs::addFileNode(B("/sys/devices"), B(""), B(""), true, sysNode);
	}

	std::shared_ptr<FsNode> pciNode = devicesNode->getChildByName(B("pci0000:00"));
	if (!pciNode) {
		pciNode = Fs::addFileNode(B("/sys/devices/pci0000:00"), B(""), B(""), true, devicesNode);
	}
	std::shared_ptr<FsNode> pciSub1Node = pciNode->getChildByName(B("0000:00:10.0"));
	if (!pciSub1Node) {
		pciSub1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0"), B(""), B(""), true, pciNode);
	}

	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/ari_enabled"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/broken_parity_status"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/class"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0x010000"));
	// config?
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/consistent_dma_mask_bits"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("64"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/d3cold_allowed"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/device"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0x0030"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/dma_mask_bits"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("64"));	
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/driver"), k_mdev(0, 0), pciSub1Node, false, B("../../../bus/pci/drivers/mptspi"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/driver_override"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("(null)"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/enable"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/irq"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("17"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/local_cpulist"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0-")+BString::valueOf(Platform::getCpuCount()-1));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/local_cpus"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("f"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/modalias"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("pci:v00001000d00000030sv000015ADsd00001976bc01sc00i00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/msi_bus"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/msi_bus"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("1"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/subsystem"), k_mdev(0, 0), pciSub1Node, false, B("../../../bus/pci"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/subsystem_device"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0x1976"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/subsystem_vendor"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0x15ad"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/uevent"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("DRIVER=mptspi\nPCI_CLASS=10000\nPCI_ID=1000:0030\nPCI_SUBSYS_ID=15AD:1976\nPCI_SLOT_NAME=0000:00:10.0\nMODALIAS=pci:v00001000d00000030sv000015ADsd00001976bc01sc00i00"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/vendor"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("0x1000"));

	std::shared_ptr<FsNode> hostNode = pciSub1Node->getChildByName(B("host2"));
	if (!hostNode) {
		hostNode = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0/host2"), B(""), B(""), true, pciSub1Node);
	}
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/subsystem"), k_mdev(0, 0), pciSub1Node, false, B("../../../../bus/scsi"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/uevent"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("DEVTYPE=scsi_host"));

	std::shared_ptr<FsNode> targetNode = hostNode->getChildByName(B("target2:0:0"));
	if (!targetNode) {
		targetNode = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0"), B(""), B(""), true, hostNode);
	}
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/uevent"), K__S_IREAD, k_mdev(0, 0), pciSub1Node, B("DEVTYPE=scsi_target"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/subsystem"), k_mdev(0, 0), pciSub1Node, false, B("../../../../../bus/scsi"));

	std::shared_ptr<FsNode> target2Node = targetNode->getChildByName(B("2:0:0:0"));
	if (!target2Node) {
		target2Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0"), B(""), B(""), true, targetNode);
	}

	std::shared_ptr<FsNode> blockNode = target2Node->getChildByName(B("block"));
	if (!blockNode) {
		blockNode = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block"), B(""), B(""), true, target2Node);
	}

	std::shared_ptr<FsNode> sdaNode = blockNode->getChildByName(B("sda"));
	if (!sdaNode) {
		sdaNode = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda"), B(""), B(""), true, blockNode);
	}
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/alignment_offset"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("0"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/bdi"), k_mdev(0, 0), sdaNode, false, B("../../../../../../../virtual/bdi/8:0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/capability"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("50"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/dev"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("8:0"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/device"), k_mdev(0, 0), sdaNode, false, B("../../../2:0:0:0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/discard_alignment"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/events"), K__S_IREAD, k_mdev(0, 0), sdaNode, B(""));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/events_async"), K__S_IREAD, k_mdev(0, 0), sdaNode, B(""));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/events_poll_msecs"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("-1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/ext_range"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("256"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/hidden"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/range"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("16"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/removable"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/size"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("83886080"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/stat"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("   15678      392   673438     5308    12421    16581   269904     4713        0     2880     6628        0        0        0        0 "));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/uevent"), K__S_IREAD, k_mdev(0, 0), sdaNode, B("MAJOR=8\nMINOR=0\nDEVNAME=sda\nDEVTYPE=disk"));

	std::shared_ptr<FsNode> sda1Node = sdaNode->getChildByName(B("sda1"));
	if (!sda1Node) {
		sda1Node = Fs::addFileNode(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1"), B(""), B(""), true, sdaNode);
	}
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/alignment_offset"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/dev"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("8:1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/discard_alignment"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/inflight"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("       0        0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/partition"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("1"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/ro"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("0"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/size"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("75495424"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/stat"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("   15509      392   664658     5295    12461    16615   270496     4732        0     2876     6608        0        0        0        0"));
	Fs::addDynamicLinkFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/subsystem"), k_mdev(0, 0), sdaNode, false, B("../../../../../../../../../class/block"));
	Fs::addVirtualFile(B("/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda/sda1/uevent"), K__S_IREAD, k_mdev(0, 0), sda1Node, B("MAJOR=8\nMINOR=1\nDEVNAME=sda1\nDEVTYPE=partition\nPARTN=1"));

	std::shared_ptr<FsNode> virtualNode = devicesNode->getChildByName(B("virtual"));
	if (!virtualNode) {
		virtualNode = Fs::addFileNode(B("/sys/devices/virtual"), B(""), B(""), true, devicesNode);
	}

	std::shared_ptr<FsNode> dmiNode = virtualNode->getChildByName(B("dmi"));
	if (!dmiNode) {
		dmiNode = Fs::addFileNode(B("/sys/devices/virtual/dmi"), B(""), B(""), true, virtualNode);
	}

	std::shared_ptr<FsNode> dmiIdNode = dmiNode->getChildByName(B("id"));
	if (!dmiIdNode) {
		dmiIdNode = Fs::addFileNode(B("/sys/devices/virtual/dmi/id"), B(""), B(""), true, dmiNode);
	}
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/bios_date"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("11/12/2020"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/bios_vendor"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("Phoenix Technologies LTD"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/bios_version"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("6.00"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/board_asset_tag"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B(""));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/board_name"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("440BX Desktop Reference Platform"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/board_serial"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("None"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/board_vendor"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("Intel Corporation"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/board_version"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("None"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/chassis_asset_tag"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("No Asset Tag"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/chassis_serial"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("None"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/chassis_type"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("1"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/chassis_vendor"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("No Enclosure"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/chassis_version"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("N/A"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/modalias"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("dmi:bvnPhoenixTechnologiesLTD:bvr6.00:bd11/12/2020:svnVMware,Inc.:pnVMwareVirtualPlatform:pvrNone:rvnIntelCorporation:rn440BXDesktopReferencePlatform:rvrNone:cvnNoEnclosure:ct1:cvrN/A:"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/product_family"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B(""));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/product_name"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("Virtual Platform"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/product_serial"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("56 4d e7 f8 ec ed 58 a6-f2 4b ab aa f1 37 49 00"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/product_sku"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B(""));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/product_uuid"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("f8e74d56-edec-a658-f24b-abaaf1374900"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/product_version"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("None"));
	Fs::addDynamicLinkFile(B("/sys/devices/virtual/dmi/id/subsystem"), k_mdev(0, 0), dmiIdNode, false, B("../../../../class/dmi"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/sys_vendor"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("Boxedwine"));
	Fs::addVirtualFile(B("/sys/devices/virtual/dmi/id/uevent"), K__S_IREAD, k_mdev(0, 0), dmiIdNode, B("MODALIAS=dmi:bvnPhoenixTechnologiesLTD:bvr6.00:bd11/12/2020:svnVMware,Inc.:pnVMwareVirtualPlatform:pvrNone:rvnIntelCorporation:rn440BXDesktopReferencePlatform:rvrNone:cvnNoEnclosure:ct1:cvrN/A:"));

	return devicesNode;
}
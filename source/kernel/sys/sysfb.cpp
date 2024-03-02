#include "boxedwine.h"
#include "../../io/fsfilenode.h"
#include "kstat.h"

U32 fbGetBpp();

void sysAddFrameBuffer(const std::shared_ptr<FsNode>& sysNode, const std::shared_ptr<FsNode>& devicesNode) {
	std::shared_ptr<FsNode> classNode = sysNode->getChildByName(B("class"));

	if (!classNode) {
		classNode = Fs::addFileNode(B("/sys/class"), B(""), B(""), true, sysNode);
	}

	std::shared_ptr<FsNode> graphicsNode = classNode->getChildByName(B("graphics"));

	if (!graphicsNode) {
		graphicsNode = Fs::addFileNode(B("/sys/class/graphics"), B(""), B(""), true, classNode);
	}
	Fs::addDynamicLinkFile(B("/sys/class/graphics/fb0"), k_mdev(0, 0), graphicsNode, false, B("../../devices/virtual/graphics/fb0"));
	Fs::addDynamicLinkFile(B("/sys/class/graphics/fbcon"), k_mdev(0, 0), graphicsNode, false, B("../../devices/virtual/graphics/fbcon"));

	std::shared_ptr<FsNode> virtualNode = devicesNode->getChildByName(B("virtual"));
	if (!virtualNode) {
		virtualNode = Fs::addFileNode(B("/sys/devices/virtual"), B(""), B(""), true, devicesNode);
	}

	std::shared_ptr<FsNode> deviceGraphicsNode = virtualNode->getChildByName(B("graphics"));

	if (!deviceGraphicsNode) {
		deviceGraphicsNode = Fs::addFileNode(B("/sys/devices/virtual/graphics"), B(""), B(""), true, virtualNode);
	}

	std::shared_ptr<FsNode> fbConNode = deviceGraphicsNode->getChildByName(B("fbcon"));

	if (!fbConNode) {
		fbConNode = Fs::addFileNode(B("/sys/devices/virtual/graphics/fbcon"), B(""), B(""), true, deviceGraphicsNode);
	}

	std::shared_ptr<FsNode> fbNode = deviceGraphicsNode->getChildByName(B("fb0"));

	if (!fbNode) {
		fbNode = Fs::addFileNode(B("/sys/devices/virtual/graphics/fb0"), B(""), B(""), true, deviceGraphicsNode);
	}

	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fbcon/cursor_blink"), K__S_IREAD, k_mdev(0, 0), fbConNode, B("0"));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fbcon/rotate"), K__S_IREAD, k_mdev(0, 0), fbConNode, B("0"));
	Fs::addDynamicLinkFile(B("/sys/devices/virtual/graphics/fbcon/subsystem"), k_mdev(0, 0), graphicsNode, false, B("../../../../class/graphics"));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fbcon/uevent"), K__S_IREAD, k_mdev(0, 0), fbConNode, B(""));

	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fb0/bits_per_pixel"), K__S_IREAD, k_mdev(0, 0), fbNode, BString::valueOf(fbGetBpp()));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fb0/blank"), K__S_IREAD, k_mdev(0, 0), fbNode, B(""));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fb0/dev"), K__S_IREAD, k_mdev(0, 0), fbNode, B("29:0"));
	Fs::addDynamicLinkFile(B("/sys/devices/virtual/graphics/fb0/device"), k_mdev(0, 0), fbNode, false, B("../../../0000:00:0f.0"));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fb0/name"), K__S_IREAD, k_mdev(0, 0), fbNode, B("svgadrmfb"));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fb0/pan"), K__S_IREAD, k_mdev(0, 0), fbNode, B("0,0"));
	Fs::addDynamicLinkFile(B("/sys/devices/virtual/graphics/fb0/subsystem"), k_mdev(0, 0), fbNode, false, B("../../../../../class/graphics"));
	Fs::addVirtualFile(B("/sys/devices/virtual/graphics/fb0/uevent"), K__S_IREAD, k_mdev(0, 0), fbNode, B("MAJOR=29\nMINOR=0\nDEVNAME=fb0"));
}
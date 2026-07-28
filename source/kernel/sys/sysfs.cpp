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
#include "syscpuonline.h"
#include "syscpumaxfreq.h"
#include "syscpuscalingcurfreq.h"
#include "syscpuscalingmaxfreq.h"
#include "knativesocket.h"

std::shared_ptr<FsNode> createSysBus(const std::shared_ptr<FsNode> sysNode);
std::shared_ptr<FsNode> createSysDevices(const std::shared_ptr<FsNode> sysNode);
std::shared_ptr<FsNode> createSysDev(const std::shared_ptr<FsNode> sysNode);
void sysAddFrameBuffer(const std::shared_ptr<FsNode>& sysNode, const std::shared_ptr<FsNode>& devicesNode);

void sysAddMouse(const std::shared_ptr<FsNode>& busNode, const std::shared_ptr<FsNode>& devicesNode);

static BString sysfsValue(const BString& value) {
    return value + B("\n");
}

static U32 getSysfsCpuCount() {
    U32 count = Platform::getCpuCount();
#ifdef BOXEDWINE_MULTI_THREADED
    if (KSystem::cpuAffinityCountForApp && KSystem::cpuAffinityCountForApp < count) {
        count = KSystem::cpuAffinityCountForApp;
    }
#endif
    if (!count) {
        count = 1;
    }
    if (count > 32) {
        count = 32;
    }
    return count;
}

static BString sysfsCpuMask(U32 cpu) {
    return sysfsValue(BString::valueOf((U32)1 << cpu, 16));
}

static BString sysfsAllCpuMask(U32 cpuCount) {
    U32 mask = cpuCount == 32 ? 0xffffffff : (((U32)1 << cpuCount) - 1);
    return sysfsValue(BString::valueOf(mask, 16));
}

static BString sysfsCpuList(U32 cpu) {
    return sysfsValue(BString::valueOf(cpu));
}

static BString sysfsAllCpuList(U32 cpuCount) {
    if (cpuCount == 1) {
        return sysfsValue(B("0"));
    }
    return sysfsValue(B("0-") + BString::valueOf(cpuCount - 1));
}

static void addSysfsCpuCacheIndex(const BString& cpuPath, const std::shared_ptr<FsNode>& cacheNode, U32 cpu, U32 cpuCount, U32 index, U32 level, const BString& type, const BString& size, bool shared) {
    BString path = cpuPath + B("/cache/index") + BString::valueOf(index);
    std::shared_ptr<FsNode> indexNode = Fs::addFileNode(path, B(""), B(""), true, cacheNode);
    BString cpuMask = shared ? sysfsAllCpuMask(cpuCount) : sysfsCpuMask(cpu);
    BString cpuList = shared ? sysfsAllCpuList(cpuCount) : sysfsCpuList(cpu);

    Fs::addVirtualFile(path + B("/shared_cpu_map"), K__S_IREAD, k_mdev(0, 0), indexNode, cpuMask);
    Fs::addVirtualFile(path + B("/shared_cpu_list"), K__S_IREAD, k_mdev(0, 0), indexNode, cpuList);
    Fs::addVirtualFile(path + B("/level"), K__S_IREAD, k_mdev(0, 0), indexNode, sysfsValue(BString::valueOf(level)));
    Fs::addVirtualFile(path + B("/type"), K__S_IREAD, k_mdev(0, 0), indexNode, sysfsValue(type));
    Fs::addVirtualFile(path + B("/coherency_line_size"), K__S_IREAD, k_mdev(0, 0), indexNode, sysfsValue(B("64")));
    Fs::addVirtualFile(path + B("/ways_of_associativity"), K__S_IREAD, k_mdev(0, 0), indexNode, sysfsValue(B("8")));
    Fs::addVirtualFile(path + B("/size"), K__S_IREAD, k_mdev(0, 0), indexNode, sysfsValue(size));
}

static void addSysfsCpuTopology(const BString& cpuPath, const std::shared_ptr<FsNode>& cpuCoreNode, U32 cpu) {
    BString path = cpuPath + B("/topology");
    std::shared_ptr<FsNode> topologyNode = Fs::addFileNode(path, B(""), B(""), true, cpuCoreNode);

    Fs::addVirtualFile(path + B("/physical_package_id"), K__S_IREAD, k_mdev(0, 0), topologyNode, sysfsValue(BString::valueOf(cpu)));
    Fs::addVirtualFile(path + B("/core_id"), K__S_IREAD, k_mdev(0, 0), topologyNode, sysfsValue(B("0")));
    Fs::addVirtualFile(path + B("/thread_siblings"), K__S_IREAD, k_mdev(0, 0), topologyNode, sysfsCpuMask(cpu));
    Fs::addVirtualFile(path + B("/thread_siblings_list"), K__S_IREAD, k_mdev(0, 0), topologyNode, sysfsCpuList(cpu));
}

static void addSysfsCpuCache(const BString& cpuPath, const std::shared_ptr<FsNode>& cpuCoreNode, U32 cpu, U32 cpuCount) {
    BString path = cpuPath + B("/cache");
    std::shared_ptr<FsNode> cacheNode = Fs::addFileNode(path, B(""), B(""), true, cpuCoreNode);

    addSysfsCpuCacheIndex(cpuPath, cacheNode, cpu, cpuCount, 0, 1, B("Data"), B("32K"), false);
    addSysfsCpuCacheIndex(cpuPath, cacheNode, cpu, cpuCount, 1, 1, B("Instruction"), B("32K"), false);
    addSysfsCpuCacheIndex(cpuPath, cacheNode, cpu, cpuCount, 2, 2, B("Unified"), B("256K"), false);
    addSysfsCpuCacheIndex(cpuPath, cacheNode, cpu, cpuCount, 3, 3, B("Unified"), B("8192K"), true);
}

static BString sysfsHexValue(U32 value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "0x%x\n", value);
    return BString::copy(buffer);
}

static BString sysfsMacAddress(const KEmulatedNetworkInterface& iface) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x\n",
        iface.mac[0], iface.mac[1], iface.mac[2], iface.mac[3], iface.mac[4], iface.mac[5]);
    return BString::copy(buffer);
}

static BString sysfsNetUevent(const KEmulatedNetworkInterface& iface) {
    return B("INTERFACE=") + iface.name + B("\nIFINDEX=") + BString::valueOf(iface.index) + B("\n");
}

static void addSysfsNetworkInterface(const std::shared_ptr<FsNode>& netNode, const KEmulatedNetworkInterface& iface) {
    BString name = BString::copy(iface.name);
    BString ifacePath = B("/sys/class/net/") + name;
    std::shared_ptr<FsNode> ifaceNode = Fs::addFileNode(ifacePath, B(""), B(""), true, netNode);

    Fs::addVirtualFile(ifacePath + B("/address"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsMacAddress(iface));
    Fs::addVirtualFile(ifacePath + B("/broadcast"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(B("ff:ff:ff:ff:ff:ff")));
    Fs::addVirtualFile(ifacePath + B("/carrier"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(B("1")));
    Fs::addVirtualFile(ifacePath + B("/dormant"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(B("0")));
    Fs::addVirtualFile(ifacePath + B("/flags"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsHexValue(iface.flags));
    Fs::addVirtualFile(ifacePath + B("/ifindex"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(BString::valueOf(iface.index)));
    Fs::addVirtualFile(ifacePath + B("/iflink"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(BString::valueOf(iface.index)));
    Fs::addVirtualFile(ifacePath + B("/mtu"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(BString::valueOf(iface.mtu)));
    Fs::addVirtualFile(ifacePath + B("/operstate"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(B("up")));
    Fs::addVirtualFile(ifacePath + B("/tx_queue_len"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(B("1000")));
    Fs::addVirtualFile(ifacePath + B("/type"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsValue(BString::valueOf(iface.hardwareType)));
    Fs::addVirtualFile(ifacePath + B("/uevent"), K__S_IREAD, k_mdev(0, 0), ifaceNode, sysfsNetUevent(iface));
}

static void sysAddNetworkClass(const std::shared_ptr<FsNode>& sysNode) {
    std::shared_ptr<FsNode> classNode = Fs::getNodeFromLocalPath(B(""), B("/sys/class"), true);
    if (!classNode) {
        classNode = Fs::addFileNode(B("/sys/class"), B(""), B(""), true, sysNode);
    }

    std::shared_ptr<FsNode> netNode = Fs::getNodeFromLocalPath(B(""), B("/sys/class/net"), true);
    if (!netNode) {
        netNode = Fs::addFileNode(B("/sys/class/net"), B(""), B(""), true, classNode);
    }

    const std::vector<KEmulatedNetworkInterface>& interfaces = getEmulatedNetworkInterfaces();
    for (U32 i = 0; i < interfaces.size(); ++i) {
        addSysfsNetworkInterface(netNode, interfaces[i]);
    }
}

void createSysfs(const std::shared_ptr<FsNode> rootNode) {
    std::shared_ptr<FsNode> sysNode = Fs::getNodeFromLocalPath(B(""), B("/sys"), true);

    if (!sysNode) {
        sysNode = Fs::addFileNode(B("/sys"), B(""), B(""), true, rootNode);
    }

    std::shared_ptr<FsNode> busNode = createSysBus(sysNode);
    std::shared_ptr<FsNode> devicesNode = createSysDevices(sysNode);
    createSysDev(sysNode);

    sysAddMouse(busNode, devicesNode);
    sysAddFrameBuffer(sysNode, devicesNode);
    sysAddNetworkClass(sysNode);

    std::shared_ptr<FsNode> devicesSystemNode = Fs::addFileNode(B("/sys/devices/system"), B(""), B(""), true, devicesNode);
    std::shared_ptr<FsNode> cpuNode = Fs::addFileNode(B("/sys/devices/system/cpu"), B(""), B(""), true, devicesSystemNode);

    U32 cpuCount = getSysfsCpuCount();
    Fs::addVirtualFile(B("/sys/devices/system/cpu/present"), K__S_IREAD, k_mdev(0, 0), cpuNode, sysfsAllCpuList(cpuCount));
    Fs::addVirtualFile(B("/sys/devices/system/cpu/online"), openSysCpuOnline, K__S_IREAD, k_mdev(0, 0), cpuNode);

    for (U32 i = 0; i < cpuCount; i++) {
        BString cpuPath = B("/sys/devices/system/cpu/cpu") + BString::valueOf(i);
        std::shared_ptr<FsNode> cpuCoreNode = Fs::addFileNode(cpuPath, B(""), B(""), true, cpuNode);
        addSysfsCpuTopology(cpuPath, cpuCoreNode, i);
        addSysfsCpuCache(cpuPath, cpuCoreNode, i, cpuCount);

        if (Platform::getCpuFreqMHz()) {
            std::shared_ptr<FsNode> cpuFreqCoreNode = Fs::addFileNode("/sys/devices/system/cpu/cpu" + BString::valueOf(i)+"/cpufreq", B(""), B(""), true, cpuCoreNode);
            Fs::addVirtualFile("/sys/devices/system/cpu/cpu" + BString::valueOf(i) + "/cpufreq/scaling_cur_freq", openSysCpuScalingCurrentFrequency, K__S_IREAD, k_mdev(0, 0), cpuFreqCoreNode, i);
            Fs::addVirtualFile("/sys/devices/system/cpu/cpu" + BString::valueOf(i) + "/cpufreq/cpuinfo_max_freq", openSysCpuMaxFrequency, K__S_IREAD, k_mdev(0, 0), cpuFreqCoreNode, i);
            Fs::addVirtualFile("/sys/devices/system/cpu/cpu" + BString::valueOf(i) + "/cpufreq/scaling_max_freq", openSysCpuScalingMaxFrequency, K__S_IREAD, k_mdev(0, 0), cpuFreqCoreNode, i);
        }
    }


}

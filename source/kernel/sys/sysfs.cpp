#include "boxedwine.h"
#include "../../io/fsfilenode.h"
#include "kstat.h"
#include "syscpuonline.h"
#include "syscpumaxfreq.h"
#include "syscpuscalingcurfreq.h"
#include "syscpuscalingmaxfreq.h"

std::shared_ptr<FsNode> createSysBus(const std::shared_ptr<FsNode> sysNode);
std::shared_ptr<FsNode> createSysDevices(const std::shared_ptr<FsNode> sysNode);
std::shared_ptr<FsNode> createSysDev(const std::shared_ptr<FsNode> sysNode);
void sysAddFrameBuffer(const std::shared_ptr<FsNode>& sysNode, const std::shared_ptr<FsNode>& devicesNode);

void sysAddMouse(const std::shared_ptr<FsNode>& busNode, const std::shared_ptr<FsNode>& devicesNode);

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

    std::shared_ptr<FsNode> devicesSystemNode = Fs::addFileNode(B("/sys/devices/system"), B(""), B(""), true, devicesNode);
    std::shared_ptr<FsNode> cpuNode = Fs::addFileNode(B("/sys/devices/system/cpu"), B(""), B(""), true, devicesSystemNode);

    U32 cpuCount = Platform::getCpuCount();
    Fs::addVirtualFile(B("/sys/devices/system/cpu/present"), K__S_IREAD, k_mdev(0, 0), cpuNode, "0-"+BString::valueOf(cpuCount-1));
    Fs::addVirtualFile(B("/sys/devices/system/cpu/online"), openSysCpuOnline, K__S_IREAD, k_mdev(0, 0), cpuNode);
    if (Platform::getCpuFreqMHz()) {
        for (U32 i = 0; i < cpuCount; i++) {
            std::shared_ptr<FsNode> cpuCoreNode = Fs::addFileNode("/sys/devices/system/cpu/cpu" + BString::valueOf(i), B(""), B(""), true, cpuNode);
            std::shared_ptr<FsNode> cpuFreqCoreNode = Fs::addFileNode("/sys/devices/system/cpu/cpu" + BString::valueOf(i)+"/cpufreq", B(""), B(""), true, cpuCoreNode);
            Fs::addVirtualFile("/sys/devices/system/cpu/cpu" + BString::valueOf(i) + "/cpufreq/scaling_cur_freq", openSysCpuScalingCurrentFrequency, K__S_IREAD, k_mdev(0, 0), cpuFreqCoreNode, i);
            Fs::addVirtualFile("/sys/devices/system/cpu/cpu" + BString::valueOf(i) + "/cpufreq/cpuinfo_max_freq", openSysCpuMaxFrequency, K__S_IREAD, k_mdev(0, 0), cpuFreqCoreNode, i);
            Fs::addVirtualFile("/sys/devices/system/cpu/cpu" + BString::valueOf(i) + "/cpufreq/scaling_max_freq", openSysCpuScalingMaxFrequency, K__S_IREAD, k_mdev(0, 0), cpuFreqCoreNode, i);
        }
    }


}

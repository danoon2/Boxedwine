/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include <sys/time.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <SDL.h>
#include <sys/mman.h>
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../../source/emulation/cpu/binaryTranslation/btCpu.h"
#endif
#include "pixelformat.h"
#include UNISTD

unsigned long long int Platform::getSystemTimeAsMicroSeconds() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	return ((unsigned long long int)tv.tv_sec) * 1000000l + (tv.tv_usec);
}

void Platform::init() {
}

void Platform::writeCodeToMemory(void* address, U32 len, std::function<void()> callback) {
#ifdef BOXEDWINE_MAC_JIT
    if (__builtin_available(macOS 11.0, *)) {
        pthread_jit_write_protect_np(false);
    }
#endif
    callback();
#ifdef BOXEDWINE_MAC_JIT
    if (__builtin_available(macOS 11.0, *)) {
        pthread_jit_write_protect_np(true);
    }
#endif
#ifndef __EMSCRIPTEN__
    // GCC, this is required for ARM, but for x86 it will just do nothing
    __builtin___clear_cache((char*)address, (char*)address+len);
#endif //__EMSCRIPTEN__
}

//#ifdef __EMSCRIPTEN__
#ifdef __THIS_HANGS__
// error TypeError: asm.js type error: missing definition of function _testSetjmp
#include <emscripten.h>
void Platform::startMicroCounter()
{    
}

unsigned long long int Platform::getMicroCounter()
{
    return (unsigned long long int)(emscripten_get_now()*1000000.0);
}
#else
long long int CounterStart;
void Platform::startMicroCounter()
{
    CounterStart = getSystemTimeAsMicroSeconds();
}

unsigned long long int Platform::getMicroCounter()
{
    return getSystemTimeAsMicroSeconds()-CounterStart;
}
#endif

void Platform::listNodes(BString nativePath, std::vector<ListNodeResult>& results) {
	DIR *dp = NULL;
	struct dirent *dptr = NULL;

	dp = opendir(nativePath.c_str());
	if (dp) {
        while(NULL != (dptr = readdir(dp))) {
			if (strcmp(dptr->d_name, ".") && strcmp(dptr->d_name, ".."))  {
				results.push_back(ListNodeResult(BString::copy(dptr->d_name), (dptr->d_type & DT_DIR)!=0));
			}
        }
        closedir(dp);
    }
}

#ifndef __MACH__
int getPixelFormats(PixelFormat* pfs, int maxPfs) {
    pfs[1].nSize = 40;
    pfs[1].nVersion = 1;
    pfs[1].dwFlags = K_PFD_SUPPORT_OPENGL|K_PFD_DRAW_TO_WINDOW|K_PFD_DOUBLEBUFFER;
    pfs[1].iPixelType = K_PFD_TYPE_RGBA;                  
    pfs[1].cRedBits = 8;
    pfs[1].cGreenBits = 8;
    pfs[1].cBlueBits = 8;
    pfs[1].cAlphaBits = 0;
    pfs[1].cAccumRedBits = 16;
    pfs[1].cAccumGreenBits = 16;
    pfs[1].cAccumBlueBits = 16;
    pfs[1].cAccumAlphaBits = 16;
    pfs[1].cAccumBits = 64;
    pfs[1].cDepthBits = 24;
    pfs[1].cStencilBits = 8;
    pfs[1].cColorBits = 32;
    memcpy(&pfs[2], &pfs[1], sizeof(PixelFormat));
    pfs[1].dwFlags|=K_PFD_GENERIC_FORMAT;
    return 2;
}
#endif

int Platform::nativeSocketPair(S32 socks[2]) {
    return socketpair(AF_LOCAL, SOCK_STREAM, 0, socks);
}

U32 Platform::getCpuFreqMHz() {
    return 0; // :TODO:
}

U32 Platform::getCpuCurScalingFreqMHz(U32 cpuIndex) {
    return 0; // :TODO:
}

U32 Platform::getCpuMaxScalingFreqMHz(U32 cpuIndex) {
    return 0; // :TODO:
}


U32 Platform::getCpuCount() {
#ifdef BOXEDWINE_MULTI_THREADED
    return (U32)SDL_GetCPUCount();
#else
    return 1;
#endif
}

U32 Platform::nanoSleep(U64 nano) {
    struct timespec req, rem;

    if (nano > 999999999)
    {
        req.tv_sec = (int)(nano / 1000000000l);                  /* Must be Non-Negative */
        req.tv_nsec = (nano - ((long)req.tv_sec * 1000000000l)); /* Must be in range of 0 to 999999999 */
    } else {
        req.tv_sec = 0;        /* Must be Non-Negative */
        req.tv_nsec = nano;    /* Must be in range of 0 to 999999999 */
    }

    nanosleep(&req, &rem);
    return 0;
}

#ifdef __MACH__
extern "C" {
void MacPlatormSetThreadPriority();
void MacPlatformOpenFileLocation(const char* str);
const char* MacPlatformGetResourcePath(const char* pName);
}

void Platform::openFileLocation(BString location) {
    MacPlatformOpenFileLocation(location.c_str());
}

BString Platform::getResourceFilePath(BString location) {
    return BString::copy(MacPlatformGetResourcePath(location.c_str()));
}

void Platform::setCurrentThreadPriorityHigh() {
#ifdef BOXEDWINE_MULTI_THREADED
    MacPlatormSetThreadPriority();
#endif
}
#else
BString Platform::getResourceFilePath(BString location) {
    return BString::empty;
}

void Platform::openFileLocation(BString location) {
    BString cmd = B("xdg-open \"");
    cmd+=location;
    cmd+="\"";
    system(cmd.c_str());
}

void Platform::setCurrentThreadPriorityHigh() {

}

#endif

U32 Platform::getPageAllocationGranularity() {
    static U32 pageSize = getpagesize() / K_PAGE_SIZE;
    return pageSize;
}

U32 Platform::getPagePermissionGranularity() {
    static U32 pageSize = getpagesize() / K_PAGE_SIZE;
    return pageSize;
}

U32 Platform::allocateNativeMemory(U64 address) {
    if (mprotect((void*)address, getPageAllocationGranularity() << K_PAGE_SHIFT, PROT_READ | PROT_WRITE) < 0) {
        kpanic("allocNativeMemory mprotect failed: %s", strerror(errno));
    }
    return 0;
}

#ifdef __MACH__
#include <mach/mach.h>

static bool isAddressRangeInUse(void* p, U64 len) {
    // get task for pid
    vm_map_t target_task = mach_task_self();

    vm_address_t iter = (vm_address_t)p;
    vm_address_t addr = iter;
    vm_size_t lsize = 0;
    uint32_t depth;
    struct vm_region_submap_info_64 info;
    mach_msg_type_number_t count = VM_REGION_SUBMAP_INFO_COUNT_64;

    kern_return_t result = vm_region_recurse_64(target_task, &addr, &lsize, &depth, (vm_region_info_t)&info, &count);
    if (result == KERN_INVALID_ADDRESS) {
        return false;
    }
    else if (result) {
        kpanic("isAddressRangeInUse vm_region_recurse_64 failed: %d", (int)result);
    }
    if (addr >= (U64)p + len) {
        return false;
    }
    return true;
}
#else

static bool isAddressRangeInUse(void* p, U64 len) {
    FILE* file = fopen("/proc/self/maps", "r");
    if (!file) {
        kpanic("reservereNext4GBMemory : cannot open /proc/self/maps, %s\n", strerror(errno));
        return false;
    }

    char buf[1024];
    while (!feof(file)) {
        char addr1[20], addr2[20];

        fgets(buf, 1024, file);

        int index = 0;
        int startIndex = 0;

        //addr1
        while (buf[index] != '-') {
            addr1[index - startIndex] = buf[index];
            index++;
        }
        addr1[index] = '\0';
        index++;
        //addr2
        startIndex = index;
        while (buf[index] != '\t' && buf[index] != ' ') {
            addr2[index - startIndex] = buf[index];
            index++;
        }
        addr2[index - startIndex] = '\0';

        unsigned long startAddress;
        unsigned long endAddress;
        sscanf(addr1, "%lx", (long unsigned*)&startAddress);
        sscanf(addr2, "%lx", (long unsigned*)&endAddress);
        if (startAddress >= (U64)p && startAddress < (U64)p + len) {
            fclose(file);
            return true;
        }
        if (endAddress >= (U64)p && endAddress < (U64)p + len) {
            fclose(file);
            return true;
        }
        if (startAddress<(U64)p + len && endAddress>(U64)p) {
            fclose(file);
            return true;
        }
    }
    fclose(file);
    return false;
}
#endif

static U64 nextMemoryId = 2;

BString Platform::procStat() {
    return BReadFile(B("/proc/stat")).readAll();
}

U32 Platform::updateNativePermission(U64 address, U32 permission, U32 len) {
    U32 proto = 0;
    if ((permission & PAGE_READ) || (permission & PAGE_EXEC)) {
        proto |= PROT_READ;
    }
    if (permission & PAGE_WRITE) {
        proto |= PROT_WRITE;
    }
    if (!proto) {
        proto = PROT_NONE;
    }
    if (len == 0) {
        len = getPagePermissionGranularity() << K_PAGE_SHIFT;
    }
    mprotect((void*)address, len, proto);
    return 0;
}

void Platform::releaseNativeMemory(void* address, U64 len) {
    munmap(address, len);
}

U8* Platform::alloc64kBlock(U32 count, bool executable) {
    int prot = PROT_WRITE | PROT_READ;
    if (executable) {
        prot |= PROT_EXEC;
    }
    U8* result = (U8*)mmap(NULL, 64 * 1024 * count, prot, MAP_ANONYMOUS | MAP_PRIVATE | MAP_BOXEDWINE, -1, 0);
    if (result == MAP_FAILED) {
        kpanic("alloc64kBlock: failed to commit memory : %s", strerror(errno));
    }
    return result;
}

#ifdef BOXEDWINE_MULTI_THREADED
#ifdef __MACH__
#include <mach/mach.h>

void Platform::setCpuAffinityForThread(KThread* thread, U32 count) {
    if (KSystem::cpuAffinityCountForApp) {
        U32 cores = Platform::getCpuCount();
        if (cores <= 1) {
            return;
        }
        if (count > 1) {
            count = 1;
        }
        
        thread_port_t port = pthread_mach_thread_np((pthread_t)((BtCPU*)thread->cpu)->nativeHandle);
        struct thread_affinity_policy policy;

        // Threads with the same affinity tag will be scheduled to share an L2 cache "if possible". 
        policy.affinity_tag = 1;
        thread_policy_set(port, THREAD_AFFINITY_POLICY, (thread_policy_t) &policy, THREAD_AFFINITY_POLICY_COUNT);
    }
}
#else
void Platform::setCpuAffinityForThread(KThread* thread, U32 count) {
    if (KSystem::cpuAffinityCountForApp) {
        U32 cores = Platform::getCpuCount();
        if (cores <= 1) {
            return;
        }
        if (count > CPU_SETSIZE) {
            count = CPU_SETSIZE;
        }
        if (count > cores) {
            count = cores;
        }
        cpu_set_t mask;
        CPU_ZERO(&mask);
        for (U32 i = 0; i < count; i++) {
            CPU_SET(i, &mask);
        }
        klog("Process %s (PID=%d) set thread %d cpu affinity to %X", thread->process->name.c_str(), thread->process->id, thread->id, count);

        sched_setaffinity((pid_t)((BtCPU*)thread->cpu)->nativeHandle, sizeof(cpu_set_t), &mask);
    }
}
#endif
#endif

#ifdef BOXEDWINE_X64
#include <cpuid.h>
bool platformHasBMI2() {
    int regs[4];

    __cpuid_count(7, 0, regs[0], regs[1], regs[2], regs[3]);
    if (regs[1] & (1 << 8)) {
        return true;
    }
    return false;
}
#endif

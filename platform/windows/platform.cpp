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
#include <winsock2.h>
#include "pixelformat.h"
#include "../source/emulation/cpu/binaryTranslation/btCpu.h"
#include <VersionHelpers.h>

LONGLONG PCFreq;
LONGLONG CounterStart;

void Platform::init() {
    if (!IsWindows8OrGreater()) {
        SetEnvironmentVariable("SDL_AUDIODRIVER", "directsound");
    }
}

void Platform::startMicroCounter()
{
    LARGE_INTEGER li;

    QueryPerformanceFrequency(&li);

    PCFreq = li.QuadPart;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

ULONGLONG Platform::getMicroCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (li.QuadPart-CounterStart)*1000000/PCFreq;
}

ULONGLONG Platform::getSystemTimeAsMicroSeconds() {
    FILETIME tm;
    ULONGLONG t;

    GetSystemTimeAsFileTime( &tm );
    t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
    t-=116444736000000000l;
    t/=10;
    /*
    if (!startTime) {
        startTime = t;
    } else {
        ULONGLONG diff = t - startTime;
        t = startTime+diff/20;
    }
    */
    return t;
}

void Platform::listNodes(const std::string& nativePath, std::vector<ListNodeResult>& results) {
    std::string path;
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    path = nativePath+"\\*.*";;
    hFind = FindFirstFile(path.c_str(), &findData); 
    if(hFind != INVALID_HANDLE_VALUE)  { 		
        do  { 
            if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))  {
                results.push_back(ListNodeResult(findData.cFileName, (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0));
            }
        } while(FindNextFile(hFind, &findData)); 
        FindClose(hFind); 
    }
}

int Platform::nativeSocketPair(S32 socks[2]) {
    union {
       struct sockaddr_in inaddr;
       struct sockaddr addr;
    } a;
    SOCKET listener;
    int e;
    socklen_t addrlen = sizeof(a.inaddr);
    DWORD flags = 0;
    int reuse = 1;

    if (socks == 0) {
      WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) 
        return SOCKET_ERROR;

    memset(&a, 0, sizeof(a));
    a.inaddr.sin_family = AF_INET;
    a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.inaddr.sin_port = 0; 

    socks[0] = socks[1] = (U32)INVALID_SOCKET;
    do {
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,  (char*) &reuse, (socklen_t) sizeof(reuse)) == -1)
            break;
        if  (bind(listener, &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
            break;
        if  (getsockname(listener, &a.addr, &addrlen) == SOCKET_ERROR)
            break;
        if (listen(listener, 1) == SOCKET_ERROR)
            break;
        socks[0] = (U32)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, flags);
        if (socks[0] == (U32)INVALID_SOCKET)
            break;
        if (connect(socks[0], &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
            break;
        socks[1] = (U32)accept(listener, NULL, NULL);
        if (socks[1] == INVALID_SOCKET)
            break;

        closesocket(listener);
        return 0;
    } while (0);

    e = WSAGetLastError();
    closesocket(listener);
    closesocket(socks[0]);
    closesocket(socks[1]);
    WSASetLastError(e);
    return SOCKET_ERROR;
}

typedef struct _PROCESSOR_POWER_INFORMATION
{
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;
typedef LONG (WINAPI *CNPI)(POWER_INFORMATION_LEVEL,PVOID,ULONG,PVOID,ULONG);
static CNPI Pwrinfo;
static U32 cachedCpuMaxFreq;

U32 Platform::getCpuFreqMHz() {
    if (cachedCpuMaxFreq) {
        return cachedCpuMaxFreq;
    }
    NTSTATUS ret;
    ULONG size;
    LPBYTE pBuffer = NULL;
    unsigned int num_cpus;
    SYSTEM_INFO system_info;
    system_info.dwNumberOfProcessors = 0;

    // Get the number of CPUs.
    GetSystemInfo(&system_info);
    if (system_info.dwNumberOfProcessors == 0) {
        num_cpus = 1;
    } else {
        num_cpus = system_info.dwNumberOfProcessors;
    }

    // Allocate size.
    size = num_cpus * sizeof(PROCESSOR_POWER_INFORMATION);
    pBuffer = (BYTE*)LocalAlloc(LPTR, size);
    if (!pBuffer) {
        return NULL;
    }

    // Syscall.
    CNPI Pwrinfo = (CNPI)GetProcAddress(LoadLibrary(TEXT("powrprof.dll")), "CallNtPowerInformation");
    ret = Pwrinfo(ProcessorInformation, NULL, 0, pBuffer, size);
    if (ret != 0) {
        LocalFree(pBuffer);
        return NULL;
    }

    PROCESSOR_POWER_INFORMATION* ppi = (PROCESSOR_POWER_INFORMATION *)pBuffer;
    cachedCpuMaxFreq = ppi->MaxMhz;
    LocalFree(pBuffer);
    return cachedCpuMaxFreq;
}

U32 Platform::getCpuCurScalingFreqMHz(U32 cpuIndex) {
    NTSTATUS ret;
    ULONG size;
    LPBYTE pBuffer = NULL;
    unsigned int num_cpus = Platform::getCpuCount();    

    if (cpuIndex>=num_cpus) {
        return 0;
    }
    // Allocate size.
    size = num_cpus * sizeof(PROCESSOR_POWER_INFORMATION);
    pBuffer = (BYTE*)LocalAlloc(LPTR, size);
    if (!pBuffer) {
        return NULL;
    }

    // Syscall.
    CNPI Pwrinfo = (CNPI)GetProcAddress(LoadLibrary(TEXT("powrprof.dll")), "CallNtPowerInformation");
    ret = Pwrinfo(ProcessorInformation, NULL, 0, pBuffer, size);
    if (ret != 0) {
        LocalFree(pBuffer);
        return NULL;
    }

    PROCESSOR_POWER_INFORMATION* ppi = (PROCESSOR_POWER_INFORMATION *)pBuffer;
    U32 result = ppi[cpuIndex].CurrentMhz;
    LocalFree(pBuffer);
    return result;
}

U32 Platform::getCpuMaxScalingFreqMHz(U32 cpuIndex) {
    NTSTATUS ret;
    ULONG size;
    LPBYTE pBuffer = NULL;
    unsigned int num_cpus = Platform::getCpuCount();    

    if (cpuIndex>=num_cpus) {
        return 0;
    }
    // Allocate size.
    size = num_cpus * sizeof(PROCESSOR_POWER_INFORMATION);
    pBuffer = (BYTE*)LocalAlloc(LPTR, size);
    if (!pBuffer) {
        return NULL;
    }

    // Syscall.
    CNPI Pwrinfo = (CNPI)GetProcAddress(LoadLibrary(TEXT("powrprof.dll")), "CallNtPowerInformation");
    ret = Pwrinfo(ProcessorInformation, NULL, 0, pBuffer, size);
    if (ret != 0) {
        LocalFree(pBuffer);
        return NULL;
    }

    PROCESSOR_POWER_INFORMATION* ppi = (PROCESSOR_POWER_INFORMATION *)pBuffer;
    U32 result = ppi[cpuIndex].MhzLimit;
    LocalFree(pBuffer);
    return result;
}


U32 Platform::getCpuCount() {
#ifdef BOXEDWINE_MULTI_THREADED
    SYSTEM_INFO system_info;
    system_info.dwNumberOfProcessors = 0;

    // Get the number of CPUs.
    GetSystemInfo(&system_info);
    if (system_info.dwNumberOfProcessors == 0) {
        return 1;
    } else {
        return system_info.dwNumberOfProcessors;
    }
#else
    return 1;
#endif
}

int getPixelFormats(PixelFormat* pfd, int maxPfs) {
    PIXELFORMATDESCRIPTOR p;
    HDC hdc = GetDC(GetDesktopWindow());
    int count = DescribePixelFormat(hdc, 0, 0, NULL);
    int result = 1;
    int i;

    for (i=1;i<=count && result<maxPfs;i++) {
        DescribePixelFormat(hdc, i, sizeof(p), &p);
        if ((p.dwFlags & PFD_SUPPORT_OPENGL) && p.cColorBits<=32 && !(p.dwFlags & PFD_GENERIC_FORMAT)) {
            pfd[result].nSize = 40;
            pfd[result].nVersion = 1;
            pfd[result].dwFlags = p.dwFlags;
            pfd[result].iPixelType = p.iPixelType;
            pfd[result].cColorBits = p.cColorBits;
            pfd[result].cRedBits = p.cRedBits;
            pfd[result].cRedShift = p.cRedShift;
            pfd[result].cGreenBits = p.cGreenBits;
            pfd[result].cGreenShift = p.cGreenShift;
            pfd[result].cBlueBits = p.cBlueBits;
            pfd[result].cBlueShift = p.cBlueShift;
            pfd[result].cAlphaBits = p.cAlphaBits;
            pfd[result].cAlphaShift = p.cAlphaShift;
            pfd[result].cAccumBits = p.cAccumBits;
            pfd[result].cAccumRedBits = p.cAccumRedBits;
            pfd[result].cAccumGreenBits = p.cAccumGreenBits;
            pfd[result].cAccumBlueBits = p.cAccumBlueBits;
            pfd[result].cAccumAlphaBits = p.cAccumAlphaBits;
            pfd[result].cDepthBits = p.cDepthBits;
            pfd[result].cStencilBits = p.cStencilBits;
            pfd[result].cAuxBuffers = p.cAuxBuffers;
            pfd[result].iLayerType = p.iLayerType;
            pfd[result].bReserved = p.bReserved;
            pfd[result].dwLayerMask = p.dwLayerMask;
            pfd[result].dwVisibleMask = p.dwVisibleMask;
            pfd[result].dwDamageMask = p.dwDamageMask;
            result++;
            fprintf(stderr, "Pixel Format: %d bit (%d%d%d%d) %s:%s depth=%d stencil=%d accum=%d\n", (int)p.cColorBits, (int)p.cRedBits, (int)p.cBlueBits, (int)p.cGreenBits, (int)p.cAlphaBits, (p.dwFlags & K_PFD_GENERIC_FORMAT)?"not accelerated":"accelerated", (p.dwFlags & K_PFD_DOUBLEBUFFER)?"double buffered":"single buffered", (int)p.cDepthBits, (int)p.cStencilBits, (int)p.cAccumBits);
        }
    }
    if (result==1) {
        for (i=1;i<=count && result<maxPfs;i++) {
            DescribePixelFormat(hdc, i, sizeof(p), &p);
            if ((p.dwFlags & PFD_SUPPORT_OPENGL) && p.cColorBits<=32) {
                pfd[result].nSize = 40;
                pfd[result].nVersion = 1;
                pfd[result].dwFlags = p.dwFlags;
                pfd[result].iPixelType = p.iPixelType;
                pfd[result].cColorBits = p.cColorBits;
                pfd[result].cRedBits = p.cRedBits;
                pfd[result].cRedShift = p.cRedShift;
                pfd[result].cGreenBits = p.cGreenBits;
                pfd[result].cGreenShift = p.cGreenShift;
                pfd[result].cBlueBits = p.cBlueBits;
                pfd[result].cBlueShift = p.cBlueShift;
                pfd[result].cAlphaBits = p.cAlphaBits;
                pfd[result].cAlphaShift = p.cAlphaShift;
                pfd[result].cAccumBits = p.cAccumBits;
                pfd[result].cAccumRedBits = p.cAccumRedBits;
                pfd[result].cAccumGreenBits = p.cAccumGreenBits;
                pfd[result].cAccumBlueBits = p.cAccumBlueBits;
                pfd[result].cAccumAlphaBits = p.cAccumAlphaBits;
                pfd[result].cDepthBits = p.cDepthBits;
                pfd[result].cStencilBits = p.cStencilBits;
                pfd[result].cAuxBuffers = p.cAuxBuffers;
                pfd[result].iLayerType = p.iLayerType;
                pfd[result].bReserved = p.bReserved;
                pfd[result].dwLayerMask = p.dwLayerMask;
                pfd[result].dwVisibleMask = p.dwVisibleMask;
                pfd[result].dwDamageMask = p.dwDamageMask;
                result++;
                fprintf(stderr, "Pixel Format: %d bit (%d%d%d%d) %s:%s depth=%d stencil=%d accum=%d\n", p.cColorBits, p.cRedBits, p.cBlueBits, p.cGreenBits, p.cAlphaBits, (p.dwFlags & K_PFD_GENERIC_FORMAT)?"not accelerated":"accelerated", (p.dwFlags & K_PFD_DOUBLEBUFFER)?"double buffered":"single buffered", p.cDepthBits, p.cStencilBits, p.cAccumBits);
            }
        }
    }
    return result;
}

const char* Platform::getResourceFilePath(const std::string& location) {
    return NULL;
}

void Platform::openFileLocation(const std::string& location) {
    ShellExecute(NULL, "open", location.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void Platform::setCurrentThreadPriorityHigh() {

}

#ifdef BOXEDWINE_MULTI_THREADED
void Platform::setCpuAffinityForThread(KThread* thread, U32 count) {
    if (KSystem::cpuAffinityCountForApp) {
        U32 cores = Platform::getCpuCount();
        if (cores <= 1) {
            return;
        }
        if (cores > 63) {
            cores = 63;
        }
        U64 mask;
        if (count == 0) {
            mask = (1 << cores) - 1;
        } else if (count == 1) {
            mask = 1 << 1; // rumor has it that core 0 isn't the best one to use
        } else {
            mask = (1 << count) - 1;
        }
        klog("Process %s (PID=%d) set thread %d cpu affinity to %X", thread->process->name.c_str(), thread->process->id, thread->id, mask);
        SetThreadAffinityMask((HANDLE)((BtCPU*)thread->cpu)->nativeHandle, mask);
    }
}
#endif

#ifdef BOXEDWINE_X64
bool platformHasBMI2() {
    int regs[4];

    __cpuidex(regs, 7, 0);
    if (regs[1] & (1 << 8)) {
        return true;
    }
    return false;
}
#endif

#ifndef __BOXEDWINE_H__
#define __BOXEDWINE_H__

#define BOXEDWINE_VERSION_STR "25R1"
#define BOXEDWINE_VERSION_DISPLAY "25.0.0 (pre-beta)"

#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <set>
#include <list>
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>
#include <assert.h>

#include <errno.h>

#if defined(__MACH__) || defined(BOXEDWINE_NEED_ATOMIC_REF)
#include "../platform/mac/atomic_ref.h"
#endif

#ifdef BOXEDWINE_X64
#define BOXEDWINE_4K_PAGE_SIZE
// with BOXEDWINE_USE_SSE_FOR_FPU enabled, quake 2 was about 8% slower, mdk perf was about 3% slower
#define BOXEDWINE_USE_SSE_FOR_FPU1
#endif

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES) || defined(BOXEDWINE_OPENGL_OSMESA)
#define BOXEDWINE_OPENGL
#endif

#ifdef BOXEDWINE_MAC_JIT
#include "TargetConditionals.h"
#if TARGET_CPU_ARM64
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_ARMV8BT
#define BOXEDWINE_MULTI_THREADED
#define MAP_BOXEDWINE MAP_JIT
// libraries not built for x64
#define BOXEDWINE_OPENGL_OSMESA
#else
#undef BOXEDWINE_MAC_JIT
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_X64
#define BOXEDWINE_MULTI_THREADED
#define MAP_BOXEDWINE 0
#endif
#else
#define MAP_BOXEDWINE 0
#endif

#include "platformtypes.h"
#include "../source/util/bstring.h"
#include "../source/util/bhashtable.h"

#include "platform.h"

struct int2Float {
    union {
        U32 i;
        float f;
    };
};

struct long2Double {
    union {
        U64 l;
        double d;
    };
};

#include "log.h"

class KProcess;
typedef std::shared_ptr<KProcess> KProcessPtr;
typedef std::weak_ptr<KProcess> KProcessWeakPtr;

#include "../source/emulation/softmmu/soft_ram.h"
#include "../source/util/bfile.h"
#include "../source/util/klist.h"
#include "ktimercallback.h"
#include "../source/util/synchronization.h"
#include "../source/util/karray.h"
#include "../source/util/stringutil.h"
#include "../source/util/vectorutils.h"
#include "../source/util/fileutils.h"

#include "kmemory.h"
#include "../source/emulation/cpu/common/cpu.h"
#include "kpoll.h"
#include "kthread.h"
#include "kfilelock.h"
#include "kobject.h"
#include "ktimer.h"
#include "kfiledescriptor.h"
#include "../source/io/fs.h"
#include "../source/io/fsnode.h"
#include "../source/io/fsopennode.h"
#include "kfile.h"
#include "ksystem.h"
#include "kprocess.h"
#include "kscheduler.h"
#include "recorder.h"
#include "player.h"

#include "log.h"
#include "kerror.h"

#endif

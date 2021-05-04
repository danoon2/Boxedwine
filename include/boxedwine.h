#ifndef __BOXEDWINE_H__
#define __BOXEDWINE_H__

#define BOXEDWINE_VERSION_STR "201"
#define BOXEDWINE_VERSION_DISPLAY "20.1.7"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <functional>
#include <set>
#include <list>
#include <filesystem>

#include <errno.h>

#ifdef BOXEDWINE_MAC_JIT
#include "TargetConditionals.h"
#if TARGET_CPU_ARM64
#define BOXEDWINE_DYNAMIC_ARMV8
#define BOXEDWINE_DYNAMIC
#define MAP_BOXEDWINE MAP_JIT
#else
#define BOXEDWINE_X64_DEBUG_NO_EXCEPTIONS
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_X64
#define BOXEDWINE_MULTI_THREADED
#define BOXEDWINE_64BIT_MMU
#define MAP_BOXEDWINE 0
#endif
#else
#define MAP_BOXEDWINE 0
#endif

#include "../source/util/boxedptr.h"

#include "platform.h"
#include "log.h"

#include "../source/util/klist.h"
#include "ktimer.h"
#include "../source/util/synchronization.h"
#include "../source/util/karray.h"
#include "../source/util/stringutil.h"
#include "../source/util/vectorutils.h"
#include "../source/util/fileutils.h"

#include "../source/emulation/cpu/common/cpu.h"
#include "kpoll.h"
#include "memory.h"
#include "kthread.h"
#include "kfilelock.h"
#include "kobject.h"
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

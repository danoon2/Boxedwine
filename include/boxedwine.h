#ifndef __BOXEDWINE_H__
#define __BOXEDWINE_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <functional>
#include <set>

#include "../source/util/boxedptr.h"

#include "platform.h"
#include "log.h"

#include "../source/util/ringbuffer.h"
#include "../source/util/klist.h"
#include "../source/util/karray.h"
#include "../source/util/stringutil.h"

#include "../source/emulation/cpu/common/cpu.h"
#include "kpoll.h"
#include "ktimer.h"
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
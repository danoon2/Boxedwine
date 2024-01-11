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
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#ifdef BOXEDWINE_64BIT_MMU

#include <unistd.h>
#include <sys/mman.h>

#ifndef BOXEDWINE_MULTI_THREADED
#include <signal.h>
#define __USE_GNU
#define _XOPEN_SOURCE
#include <ucontext.h>

// from llvm
#ifdef __MACH__
#elif defined(__aarch64__)
// Android headers in the older NDK releases miss this definition.
struct __sanitizer_esr_context {
  struct _aarch64_ctx head;
  uint64_t esr;
};
static bool Aarch64GetESR(ucontext_t *ucontext, U64 *esr) {
  static const U32 kEsrMagic = 0x45535201;
  U8 *aux = ucontext->uc_mcontext.__reserved;
  while (true) {
    _aarch64_ctx *ctx = (_aarch64_ctx *)aux;
    if (ctx->size == 0) break;
    if (ctx->magic == kEsrMagic) {
      *esr = ((__sanitizer_esr_context *)ctx)->esr;
      return true;
    }
    aux += ctx->size;
  }
  return false;
}
#endif

static void handler(int sig, siginfo_t* info, void* context)
{
    KThread* thread = KThread::currentThread();
    U32 address = getHostAddress(thread, (void*)info->si_addr);
    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = thread->memory->getNativePage(page);
    if (thread->process->memory->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY) {
        U32 emulatedPage = thread->memory->getEmulatedPage(nativePage);
        for (int i=0;i<K_NATIVE_PAGES_PER_PAGE;i++) {
            thread->process->memory->clearCodePageFromCache(emulatedPage + i);
        }
        // will continue
    } else {
#ifdef __MACH__
#if defined(__aarch64__)
        bool readAccess = (((ucontext_t*)context)->uc_mcontext->__es.__esr & 1) == 0;
#else
        bool readAccess = (((ucontext_t*)context)->uc_mcontext->__es.__err & 1) == 0;
#endif
#elif defined (__aarch64__)
        DecodedOp* op = DecodedBlock::currentBlock->getOp(thread->cpu->getEipAddress());
        bool readAccess = true; // :TODO: ??? where is this in the signal info
        if (op) {
            readAccess = instructionInfo[op->inst].writeMemWidth == 0;
        }
        static const U64 ESR_ELx_WNR = 1U << 6;
        U64 esr;
        if (Aarch64GetESR((ucontext_t*)context, &esr)) {
            readAccess = (esr & ESR_ELx_WNR) == 0;
        }
#else
        bool readAccess = (((ucontext_t*)context)->uc_mcontext.gregs[REG_ERR] & 1) == 0;
#endif
        
        if (!readAccess && (thread->process->memory->flags[page] & PROT_WRITE)) {
            void* p = (void*)(thread->memory->id + (thread->memory->getNativePage(page) << K_NATIVE_PAGE_SHIFT));
            mprotect(p, K_NATIVE_PAGE_SIZE, PROT_READ | PROT_WRITE);
            return;
        }
        if (info->si_code==SEGV_MAPERR) {
            thread->seg_mapper(address, readAccess, !readAccess, true);
        } else {
            thread->seg_access(address, readAccess, !readAccess, true);
        }
        // above functions will long jmp out
    }
}

void platformRunThreadSlice(KThread* thread) {
    static bool initializedHandler = false;
    if (!initializedHandler) {
        struct sigaction sa;
        struct sigaction oldsa;
        sa.sa_sigaction = handler;
        sa.sa_flags = SA_SIGINFO;
#ifdef __MACH__
        sigaction(SIGBUS, &sa, &oldsa);
#else
        sigaction(SIGSEGV, &sa, &oldsa);
#endif
        initializedHandler = true;
#ifdef __MACH__
        // proc hand -p true -s false SIGBUS
        // in the debug output window, (lldb) enter the above command in order to run while debugging on Mac
        
        // set a break point on this line then enter the above commands.
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS, MACH_PORT_NULL, EXCEPTION_DEFAULT, 0);
#endif
    }
    runThreadSlice(thread);
}
#endif

#endif

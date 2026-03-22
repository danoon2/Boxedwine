# CPU Emulation

Boxedwine runs x86 Linux binaries using Tiny Core Linux 15 as the base file system.  The main purpose of Boxedwine is to run the 32-bit version of Wine in order to run 32-bit Windows applications and games.  Wine is not an emulator so it will expect the host to run on an x86 processor in order to run the x86 compiled code of the Windows apps/games.

Boxedwine is an emulator and will emulate x86 instructions.  Because of this it is easy to run Boxedwine on other hardware architectures such as ARM.  The simplist CPU emulation is to decode an x86 instruction then run the code associated with that instruction, for example you can think of a giant switch statement that loops, handling each instruction.  Boxedwine has a 2 main types of CPU emulation, "normal" which is kind of like the giant switch/loop I just mentioned and JIT (Just in Time) which converts each x86 instruction to the native host instruction at run time.  The normal CPU emulator is slow but since it doesn't know about the host, it will just work on all architectures.  The JIT CPU emulator is much faster but has only been implemented for x86, x64 and ARMv8 (ARM64).

All CPU emulation supports x87 (FPU), MMX, SSE and SSE2.

CPU emulation is also well unit tested including comparison to actual hardware results when running with MSVC and 32-bit, see:

https://github.com/danoon2/Boxedwine/tree/master/source/test

## Normal CPU
The normal CPU emulator is the slowest but compatible with all platforms and architectures since it doesn't contain any knowledge of the hosts CPU architecture.  Currently it is only used for the WASM/Emscripten build for the web target.

The source for the normal CPU emulator can be found here:

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/normal

The normal CPU emulator will decode a group of x86 instructions where each group is terminated when there is a branching instruction, like jmp, call, ret, etc.  So the groups are usually pretty small, on average about 5 instructions.  After it is decoded, the instructions are copied into "DecodedOp" and stored in KMemoryData.opCache so that can easily re-used.  Of course the code will need to watch the memory for writes in order to invalidate that cache.  The code to handle the cache invalidation is here:

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/softmmu/soft_code_page.cpp

A DecodedOp points to the next DecodedOp, so when a DecodedOp is called it will automatically call the next op in the group.  Because the next op that will be called can be anything (not known at Boxedwine compile time), this will use an indirect call (think virtual call for c++/java).  These are slower than normal direct calls because the host CPU cannot predict where the code will jump to.

The normal CPU can run in single or multi-threaded mode.  For single-threaded mode, there is a scheduler that tries to give each thread some time, but its honestly not that great and can result in stuttering

https://github.com/danoon2/Boxedwine/tree/master/source/kernel/kscheduler.cpp#L168

The multi-thread CPU is about 50% faster than the single-thread.  The main trick to getting this right is handling the lock instruction prefix.  Here is the code that handles that.

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/common/common_lock.cpp


## JIT
This is the same as the normal CPU emulator but for groups of DecodedOp's that are run more frequently it will recompile that group to native host code to be faster.  DecodedOp will contain a pointer to the JIT code for that instruction, so when the main CPU loops calls the next instruction, it will ask the DecodedOp to provide the Normal CPU function or the JIT if it has already been converted.

see: void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op)
https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/jit/jitCodeGen.cpp#L718

The JIT contains lots of code to properly handle memory and CPU flags for all of the x86 instructions
https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/jit

Each host/platform architecture will need some specific code for the JIT to work, here are the two that implemented as of now

- x86/x64: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/x32
- armv8: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv8
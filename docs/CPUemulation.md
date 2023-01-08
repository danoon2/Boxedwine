# CPU Emulation

Boxedwine runs x86 Linux binaries, so far only binaries from Debian 10 have been tested.  The main purpose of Boxedwine is to run the 32-bit version of Wine in order to run 32-bit Windows applications and games.  Wine is not an emulator so it will expect the host to run on an x86 processor inorder to run the x86 compiled code of the Windows apps/games.

Boxedwine is an emulator and will emulate all x86 instructions.  Because of this it is easy to run Boxedwine on other hardware architectures such as ARM.  The simplist CPU emulation is to decode an x86 instruction then run the code associated with that instruction, for example you can think of a giant switch statement that loops, handling each instruction.  Boxedwine has a 2 main types of cpu emulation, "normal" which is kind of like the giant switch/loop I just mentioned and "binary translation" which converts each x86 instruction to the native host instruction at run time.  The normal cpu emulator is slow but since it doesn't know about the host, it will just work on all architectures.  The binary translator cpu emulator is very fast but requires a large effort to bring up for each new architecture.  Currently the binary translator is written for x64 and ARMv8 (ARM64).

All cpu emulation support x87 (FPU), mmx, sse and sse2.

CPU emulation is also well unit tested including comparison to actual hardware results when running with MSVC and 32-bit, see:

https://github.com/danoon2/Boxedwine/tree/master/source/test

## Normal CPU
The normal cpu emulator is the slowest but compatible with all platforms and architectures since it doesn't contain any knowledge of the hosts CPU architecture.  Currently it is only used for the WASM/Emscripten build for the web target.

The source for the normal cpu emulator can be found here:

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/normal

The normal cpu emulator will decode a block of x86 instructions where each block is terminated when there is a branching instruction, like jmp, call, rtn, etc.  So the blocks are usually pretty small, on average about 5 instructions.  After it is decoded the instructions are stored in a "DecodedBlock" and that block will be stored for the process so that it can be re-used.  Of course the code will need to watch the memory for writes in order to invalidate that cache.  The code to handle the cache is here:

https://github.com/danoon2/Boxedwine/blob/master/source/emulation/softmmu/soft_code_page.cpp

A "DecodedBlock" has a linked list of "DecodedOp", so when a block is called you only call the first "DecodedOp" and that op will call the next.  Because the next op that will be called can be anything (not known at Boxedwine compile time), this will use an indirect call (think virtual call for c++/java).  These are slower than normal direct calls because the host cpu cannot predict where the code will jump to.

The normal cpu does not handle the "lock" instruction in x86.  This is a very important instruction when it comes to synchronizing memory access between threads. In a multi-threaded emulator, not handling this will cause crashes.  Because of this the normal cpu emulator will also emulate threads and thread scheduling.  For the scheduling code, see

https://github.com/danoon2/Boxedwine/blob/master/source/kernel/kscheduler.cpp#L168


## Normal CPU + JIT
This is the same as the normal cpu emulator but for blocks that are run more frequently it will recompile the block to be faster.  It will use a combination of inlining some x86 instructions and for more complicated instructions it will just call a function to handle it from within the block.  Here is the code that handles the start of the JIT

https://github.com/danoon2/Boxedwine/blob/master/source/emulation/cpu/x32/x32CPU.cpp#L2188

Each architecture that wants to JIT the normal cpu emulator needs its own code

x86: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/x32
armv7: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv7
armv8: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv8

They all share common code that will break the x86 code into smaller instructions, microps, so that each architecture only has to implement the microps.  This allows each platform to implement the JIT in only about 1000-2000 lines of code.  See:

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/dynamic

This code will also remove unnessary x86 flag calculation in order speed things up a bit.  In general, the JIT will increase performance by about 2x.  The main speed up comes from removing indirect calls that make it hard for the host cpu branch predictor.

So if a block has 3 instructions the normal cpu emulator will call them like this

op -> op -> op

where those 2 arrows are indirect function calls (generated at Boxedwine compile time)

The jit will change it to a new function

{
    op1;
    op2;
    op3;
}

Where some of the ops might be inlined and some might call out to other functions.  But this is faster because those called functions will use a direct call instead of a indirect call and the host CPU can branch predict this.

## Binary Translator

The binary translator is much faster and it is also multi-threaded.  Each emulated thread will have its own host thread.  When building, BOXEDWINE_BINARY_TRANSLATOR, BOXEDWINE_MULTI_THREADED and BOXEDWINE_64BIT_MMU will need to be enabled/defined.  Because memory is handled by using a single continuous 4GB chunk of virtual memory per emulated process, thus allowing a single offset to be added to all memory instruction, the binary translator only works on 64-bit systems with large amounts of virtual memory.

Currently binary translation has only been implemented for 2 architectures:

x64: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/x64
ARMv8: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv8bt

The x64 version has been tested on Windows, Linux and Mac
The ARMv8 version has been tested on a Raspberry Pi 4 and Mac M1

The binary translator will translate each x86 instruction to machine code compatible with the host CPU.

Some instructions on x64 translate 1 to 1 from x86 like
```
add eax, 2
```
-> 
```
add eax, 2
```

All memory instructions require an offset which for x64 is stored in R14

```
add eax, DWORD PTR[rcx+4]
```
->
```
lea r9d, [rcx + 4]
add eax, DWORD PTR[r9 + r14]
```
So when memory is used, a single instruction will often be translated into 2 or 3 instructions for x64.  It is important when doing the address calculation that address does not overflow, which is why even if the above 2 instructions could be combined into a single instruction on x64 like DWORD PTR[rcx + r14 + 4], it won't be.  The rcx+4 could overflow the 32-bit memory address because address calculations are 64-bit.

The FPU is fully emulated for the binary translator, even on the x64 host.  Because of this the FPU should be considered slow.
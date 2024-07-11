# CPU Emulation

Boxedwine runs x86 Linux binaries, so far only binaries from Debian 10 have been tested.  The main purpose of Boxedwine is to run the 32-bit version of Wine in order to run 32-bit Windows applications and games.  Wine is not an emulator so it will expect the host to run on an x86 processor inorder to run the x86 compiled code of the Windows apps/games.

Boxedwine is an emulator and will emulate all x86 instructions.  Because of this it is easy to run Boxedwine on other hardware architectures such as ARM.  The simplist CPU emulation is to decode an x86 instruction then run the code associated with that instruction, for example you can think of a giant switch statement that loops, handling each instruction.  Boxedwine has a 2 main types of CPU emulation, "normal" which is kind of like the giant switch/loop I just mentioned and "binary translation" which converts each x86 instruction to the native host instruction at run time.  The normal CPU emulator is slow but since it doesn't know about the host, it will just work on all architectures.  The binary translator CPU emulator is very fast but requires a large effort to bring up for each new architecture.  Currently the binary translator is written for x64 and ARMv8 (ARM64).

All CPU emulation supports x87 (FPU), MMX, SSE and SSE2.

CPU emulation is also well unit tested including comparison to actual hardware results when running with MSVC and 32-bit, see:

https://github.com/danoon2/Boxedwine/tree/master/source/test

## Normal CPU
The normal CPU emulator is the slowest but compatible with all platforms and architectures since it doesn't contain any knowledge of the hosts CPU architecture.  Currently it is only used for the WASM/Emscripten build for the web target.

The source for the normal CPU emulator can be found here:

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/normal

The normal CPU emulator will decode a block of x86 instructions where each block is terminated when there is a branching instruction, like jmp, call, ret, etc.  So the blocks are usually pretty small, on average about 5 instructions.  After it is decoded, the instructions are stored in a "DecodedBlock" and that block will be stored for the process so that it can be re-used.  Of course the code will need to watch the memory for writes in order to invalidate that cache.  The code to handle the cache is here:

https://github.com/danoon2/Boxedwine/blob/master/source/emulation/softmmu/soft_code_page.cpp

A "DecodedBlock" has a linked list of "DecodedOp", so when a block is called you only call the first "DecodedOp" and that op will call the next.  Because the next op that will be called can be anything (not known at Boxedwine compile time), this will use an indirect call (think virtual call for c++/java).  These are slower than normal direct calls because the host CPU cannot predict where the code will jump to.

The normal CPU does not handle the "lock" instruction in x86.  This is a very important instruction when it comes to synchronizing memory access between threads. In a multi-threaded emulator, not handling this will cause crashes.  Because of this the normal CPU emulator will also emulate threads and thread scheduling.  For the scheduling code, see

https://github.com/danoon2/Boxedwine/blob/master/source/kernel/kscheduler.cpp#L168


## Normal CPU + JIT
This is the same as the normal CPU emulator but for blocks that are run more frequently it will recompile the block to be faster.  It will use a combination of inlining some x86 instructions and for more complicated instructions it will just call a function to handle it from within the block.  Here is the code that handles the start of the JIT

https://github.com/danoon2/Boxedwine/blob/master/source/emulation/cpu/x32/x32CPU.cpp#L2188

Each architecture that wants to JIT the normal CPU emulator needs its own code

- x86: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/x32
- armv7: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv7
- armv8: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv8

They all share common code that will break the x86 code into smaller instructions, microps, so that each architecture only has to implement the microps.  This allows each platform to implement the JIT in only about 1000-2000 lines of code.  See:

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/dynamic

This code will also remove unnessary x86 flag calculation in order speed things up a bit.  In general, the JIT will increase performance by about 2x.  The main speed up comes from removing indirect calls that make it hard for the host CPU branch predictor.

So if a block has 3 instructions the normal CPU emulator will call them like this
```
op -> op -> op
```
where those 2 arrows are indirect function calls (generated at Boxedwine compile time)

The jit will change it to a new function
```
{
    op1;
    op2;
    op3;
}
```
Where some of the ops might be inlined and some might call out to other functions.  But this is faster because those called functions will use a direct call instead of a indirect call and the host CPU can branch predict this.

## Binary Translator

The binary translator is much faster and it is also multi-threaded.  Each emulated thread will have its own host thread.  When building, BOXEDWINE_BINARY_TRANSLATOR, BOXEDWINE_MULTI_THREADED and BOXEDWINE_64BIT_MMU or BOXEDWINE_ARMV8BT will need to be enabled/defined.  

Currently binary translation has only been implemented for 2 architectures:

- x64: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/x64
- ARMv8: https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/armv8bt

The x64 version has been tested on Windows, Linux and Mac

The ARMv8 version has been tested on a Raspberry Pi 4/5 and Mac M1

The binary translator will translate each x86 instruction to machine code compatible with the host CPU.

Some instructions on x64 translate 1 to 1 from x86 like
```
add eax, 2
```
converts to
```
add eax, 2
```

Memory pages are fully emulated.  So this code will work even on hosts with a large page size.  To prevent calling code when accessing memory, each page will store its read and write offsets in an array cache on a per emulated process basis.  That array will contain a 0/null entry if the app doesn't have permission for that page.  This allows the translated code to quickly check the read/write array and do the memory operation directly. 

pseudo to read memory from binary translator

```
U32 readMemory32(U32 address) {
    U32 page = address >> 12;
    U32 offset = address & 0xfff;

    U8* hostPage = processMemory->mmuReadPtr[page];

    // test if we have permission, and that the read won't go past the end of the page
    if (!hostPage || (offset > 0xffc)) {
        // this will exit running in the generated emulation code and move to the Boxedwine host code via calling emulateSingleOp
        jumpToEmulateSingleOp();
    }
    return *(U32*)hostPage + offset;
}
```

x64 implementation of above pseudo code

https://github.com/danoon2/Boxedwine/tree/master/source/emulation/cpu/x64/x64Asm.cpp#L942
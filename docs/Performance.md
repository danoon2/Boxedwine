3 kinds of CPU emulation:
1. Binary Translator (BT) - All CPU instructions are translated to the host's native CPU instruction set. This is also multi-threaded.
2. Just in Time (JIT) - This is the normal emulation which dynamically inlines often used code blocks.
3. Normal - No dynamic code

Build 21.0.0
||Core i7 6700K Win64 (BT)|Core i7 6700K Win32 (JIT)|Raspberry Pi 64-bit (BT)
|---|---|---|---|
|MDK Perf - OpenGL|1250 (MAX)|73|FAILED
|MDK Perf - GDI|1050|74|132
|Quake 2 800x600 OpenGL|355.5 fps|71.7 fps|29.2
|Quake 2 800x600 Software|54.7 fps|5.3 fps|11.2
|3D Mark 2001 SE|23367|||

`Quake 2 command line: +timedemo 1 +map demo1.dm2`
All tests used Wine 10 unless otherwise noted.

x64/Win64 Windows 11 on Intel i7-14700 (28 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|79fps 1083MB|73fps 809MB (99fps with Wine 11)
|Cinebench 11.5 CPU Test|6.15 1796MB|10.02 1403MB (63% faster)

x86/Win32 Windows 11 on Intel i7-14700 (28 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|32fps 395MB|57.3 fps 450MB
|Cinebench 11.5 CPU Test|1.16 957MB|3.90 983MB (236% faster)

ArmV8 Windows 11 Snapdragon X - X126100 (8 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|46.3fps 1027MB|57.2fps 852MB
|Cinebench 11.5 CPU Test|1.85 1688MB|4.14 1477MB (124% faster)

ArmV8 Mac OS 15.6 on Mac Mini M4 (10 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|74.6fps|84.3fps
|Cinebench 11.5 CPU Test|3.16|4.71 (49% faster)

ArmV8 Asahi Linux on Mac Mini M1 (8 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|48.0fps|63.3fps
|Cinebench 11.5 CPU Test|1.46|2.67 (83% faster)

`Quake 2 command line: +timedemo 1 +map demo1.dm2`
### General Tips

1. If you launch Boxedwine without any arguments, it will launch a UI. This UI can be used to launch apps and games, but when it launches them it will do so in another process. This can make debugging those app and games more difficult. To get around this, you can define BOXEDWINE_UI_LAUNCH_IN_PROCESS in your build and the UI will launch the apps and games in the same process.

2. If _DEBUG is set, this is set on Mac/Windows debug targets, you will see extra information in the console when running an app or game. For example, it will show you files that Wine tried to open but couldn't find. This is a good place to start when seeing why an app or game isn't working, perhaps it tried to load a dependency but couldn't find it.

3. If you want to see the command line that was used to launch Boxedwine from the UI you can open the container folder and lastLog.txt should be there. This file starts with the command line used and also contains the console output of the last time the container was used.
### Useful break points

1. It is rare for 32-bit apps and game to trigger an exception/signal. So if your 32-bit app or game is crashing, try setting a break point in KThread::runSignal. This gets called when Boxedwine tries to communicate to Wine that an exception has happened. For easier to understand call stacks, it is best to debug the normal CPU core.

[kernel/kthread.cpp void KThread::runSignal(U32 signal, U32 trapNo, U32 errorNo)](https://github.com/danoon2/Boxedwine/blob/master/source/kernel/kthread.cpp)

2. I always keep a break point near the top of kpanic, that way if it gets called I can see the call stack and some context before the app/game exits.

[util/log.cpp kpanic](https://github.com/danoon2/Boxedwine/blob/master/source/util/log.cpp#L31)

### Windows Tips
If you want to see the console while debugging, you can right click on the Boxedwine project in Visual Studio to see the "Boxedwine Property Pages". From there you can selected Linker, then System. The first item in Linker/System is SubSystem. The SubSystem is set to Windows, but you can change it to Console.
### In Progress

1. Improve performance on Raspberry Pi for 64-bit OS
2. Make official build for Mac Intel/Arm

Update (12/30/2023)
1. Armv8 on Raspberry Pi is working well, just a couple bugs left, mainly around 16-bit apps
2. Mac Intel builds are available on the build server (click green check next to checkin, then click "details", then click artifacts on top right of Jenkins)
3. Mac Arm still has some issues, mainly around the 16k page size and trying to emulate a 4k page size

### Sometime in the distant future

1. Add support to mount ISO's
2. Gecko support
3. .NET and Java support
4. Copy/Paste support
5. Improve performance for Emscripten/Web build
6. Add the ability to launch Dosbox for the few Windows games that use Dos installers.
7. Joystick support using SDL
8. Mouse Capture support
9. Add Android support
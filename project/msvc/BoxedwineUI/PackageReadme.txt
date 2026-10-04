Boxedwine for Windows

Extract the complete folder, then open Boxedwine.exe.

This app requires the Microsoft .NET 10 Desktop Runtime. If it is missing,
Boxedwine.exe shows a Windows dialog offering to open Microsoft's download page.
Download and install the Desktop Runtime, then open Boxedwine.exe again.

https://dotnet.microsoft.com/en-us/download/dotnet/10.0

Choose Windows x64 for Win64 / win-x64, or Windows Arm64 for WinARM64 / win-arm64.
Use the Desktop Runtime section. The SDK is not required, and the plain .NET
Runtime or ASP.NET Core Runtime alone does not include the desktop components.
An existing matching .NET 10 Desktop Runtime installation is reused.

Keep Runtime/BoxedwineEngine.exe and Resources with the app. The Runtime folder
contains the Boxedwine emulator; it does not contain the .NET runtime.
Wine packages are downloaded when selected in Boxedwine.
OpenGL defaults are in Settings > Graphics; per-app overrides are in App Settings
> Advanced. x64 defaults to Native, Arm64 to Mesa LLVMpipe. Alternative OpenGL
drivers are downloaded when first needed and shared across apps.

In the combined build ZIP, Boxedwine_console.exe is the command-line/automation
build. That executable and the legacy Win32 frontend do not require .NET.

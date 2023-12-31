# Ahead of time compilation

The purpose of ahead of time (AOT) compilation is to improve CPU performance on platforms that don't support just in time (JIT) code generation.  Currently the only platform that doesn't support JIT is WASM/Emscriptem for the web.

AOT requires 2 steps.  Step 1 is to generated the AOT code file.  Step 2 is to create a build using the AOT code file.

## High level steps

- Create a build and define BOXEDWINE_GENERATE_SOURCE and BOXEDWINE_DYNAMIC.
- Run the build and your app then exit cleanly.  You will need to pass in a command line argument, -aot fileName, where fileName is the path to the file that will be generated.
- Create a build that includes your generated file and define BOXEDWINE_USE_GENERATE_SOURCE and BOXEDWINE_DYNAMIC

AOT does not work with other dynamic JIT compilers and it does not work with the binary translators.  It only works swith a plain normal single threaded CPU core.

## On Windows

- Using Visual Studio, build the target Release AOT.  
- Run your program from the command line and add the command line option, -aot filePath

If you want to test and run with the generate file in Windows you can

- Using Visual Studio, select the target Release AOT.
- Change the C++ preprocessor option BOXEDWINE_GENERATE_SOURCE to BOXEDWINE_USE_GENERATE_SOURCE in Visual Studio.  This is located by right clicking on "Boxedwine (Visual Studio .." in the Solution Explorer on the left and selecting "Properties" at the bottom on the popup menu.
- Copy your generated file to source/emulation/cpu/aot/gen.cpp, this will copy over the existing blank file
- Build and run
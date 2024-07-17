This is a work in progress: I will need to test this on a clean Mac

1) Download Mesa, I used https://archive.mesa3d.org//mesa-21.1.3.tar.xz

2) Create directory for everything: 
```
mkdir ~/mesa
```
3) Enter directory: 
```
cd ~/messa
```
4) Unzip Mesa, 
```
tar xvf ~/Downloads/mesa-21.1.3.tar.xz
```
5) Make sure you have all of the dependencies, using home brew this is
```
brew install pkg-config
brew install bison
brew install llvm
brew install meson
brew uninstall cmake
```
cmake seems to mess things up, so make sure its not installed.

6) Enter brew shell: 
```
brew sh
```
7) Create build directory: 
```
mkdir ~/mesa/build
```
8) Change to source directory: 
```
cd ~/mesa/mesa-21.1.3
```
9) Configure Mesa for OSMesa
```
meson ~/mesa/build -Dgallium-drivers=swrast -Ddri-drivers= -Dvulkan-drivers= -Dprefix=~/mesa/build/install -Dlibunwind=disabled -Dosmesa=true -Degl=disabled -Dgles1=disabled -Dgles2=disabled -Dglx=disabled -Dvalgrind=disabled -Dc_std=c11 -Dshared-llvm=false -Dshared-glapi=enabled
```
10) Build Mesa
```
ninja -C ~/mesa/build
```
11) Copy libOSMesa to Boxedwine
```
cp ../build/src/gallium/targets/osmesa/libOSMesa.8.dylib ~/Boxedwine/lib/mac/precompiledMac  (or wherever the Boxedwine source is)
```
12) Copy libglapi to Boxedwine
``` 
cp ../build/src/mapi/shared-glapi/libglapi.0.dylib ~/Boxedwine/lib/mac/precompiledMac
```
13) Switch to Boxedwine lib directory: 
```
cd ~/Boxedwine/lib/mac/precompiledMac
```
14) change where libOSMesa.8.dylib looks for its dependency, libglapi.0.dylib
```
install_name_tool -change "@rpath/libglapi.0.dylib" "@executable_path/libglapi.0.dylib" libOSMesa.8.dylib
```
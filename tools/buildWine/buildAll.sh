#!/bin/bash
#To get OSS to compile, I downloaded a release from 
#http://www.opensound.com/developer/sources/stable/gpl/

#And copied include/soundcard.h to /usr/lib/oss/include/sys/soundcard.h

set -e
if [ ! -d wine-git ]
then
  git clone git://source.winehq.org/git/wine.git wine-git
fi

if [ -d tmp_install ]
then
  rm -rf tmp_install
fi

cd wine-git

do_build()
{
  VERSION=$1
  shift
  if [[ $1 == "wglext" ]]
  then
    WGLEXT="-DBOXED_NEED_WGLEXT"
    shift
  fi
  if [ ! -f ../wine-$VERSION.zip ]
    then
    git reset --hard
    git checkout wine-$VERSION
    while [[ $1 != "" ]]; do
      if [[ $1 == "patch" ]]
      then
        shift
        git apply ../patches/$1
        if [[ $PATCHES != "" ]]
        then
          PATCHES+=" "
        fi
        PATCHES+=$1
        shift
      elif [[ $1 == "revert" ]]
      then
        shift
        git revert -n $1
        if [[ $REVERTS != "" ]]
        then
          REVERTS+=" "
        fi
        REVERTS+="git revert -n $1"
        shift
      elif [[ $1 == "depends" ]]
      then
        shift
        ADD_DEPENDS=1
      else
        echo "unknown do_build option: $1"
        exit
      fi
    done
    rm -rf dlls/winex11.drv/*
    cp -r ../../wineboxed.drv/*.* dlls/winex11.drv/
    rm -rf dlls/winealsa.drv/*
    cp -r ../../wineboxedaudio.drv/*.* dlls/winealsa.drv/
    if [[ $ADD_DEPENDS == 1 ]]
    then
      echo "@MAKE_DLL_RULES@" >> dlls/winex11.drv/Makefile.in
      echo "@MAKE_DLL_RULES@" >> dlls/winealsa.drv/Makefile.in
    fi
#for some reason I don't understand the config process will fail when looking for dependencies because the header does not exist, even though I wrapped it in a #ifdef
    touch include/wine/wglext.h
    ./configure LDFLAGS="-s" CFLAGS="-O2 -msse2 -march=pentium4 -mfpmath=sse $WGLEXT" --without-cups --without-pulse --without-dbus --without-sane --without-hal --prefix=/opt/wine --disable-tests $EXTRA_ARGS
    make -j4
    #todo find another way to achieve what I want without using sudo
    sudo rm -rf /opt/wine
    sudo make install
    mkdir ../tmp_install
    mkdir ../tmp_install/opt
    cp -r /opt/wine ../tmp_install/opt/
    cd ../tmp_install
    rm opt/wine/lib/wine/winemenubuilder.exe.so
    rm opt/wine/lib/libwine.so 
    rm opt/wine/lib/libwine.so.1
    printf "libwine.so.1.0" > opt/wine/lib/libwine.so.link
    printf "libwine.so.1.0" > opt/wine/lib/libwine.so.1.link
    printf "Wine $VERSION" > wineVersion.txt
    printf "debian10.zip" > depends.txt
    cp ../changes.txt changes.txt
    echo "git checkout wine-$VERSION" > build.txt
    if [[ $REVERTS != "" ]]
    then
      echo $REVERTS >> build.txt
    fi
    if [[ $PATCHES != "" ]]
    then
      echo "Patched: $PATCHES" >> build.txt
    fi
    echo './configure CFLAGS="-O2 -msse2 -march=pentium4 -mfpmath=sse" --without-pulse --without-dbus --without-sane --without-hal --prefix=/opt/wine --disable-tests' >> build.txt
    echo "make -j4" >> build.txt
    mv opt/wine/lib/wine/wineoss.drv.so opt/wine/lib/wine/wineoss.drv.dsp.so
    zip -r ../wine-$VERSION.zip *
    cd ../wine-git
    make clean
    rm -rf ../tmp_install
  fi
}

VERSION=$(sed 's/\..*//' /etc/debian_version)
echo "Debian Version = $VERSION"
if [ $VERSION -gt 9 ]
then
# f2e5b8070776268912e1886d4516d7ddec6969fc
# kernel32: Use the Get/SetComputerName functions from kernelbase. 
# reverted because on slower systems it will fail to create a window, not sure why
    do_build 5.0 patch ddraw_waitvblank.patch patch wine5-lz.patch revert f2e5b8070776268912e1886d4516d7ddec6969fc
    do_build 4.0 patch ddraw_waitvblank.patch
    do_build 3.1 patch ddraw_waitvblank.patch
else
    do_build 2.0 wglext patch ddraw_waitvblank.patch

#patch wine18-19-gnutls.patch
#1.7.47 <= wine < 1.9.13"

#patch wine17-19-cups.patch
#1.7.12 <= wine < 1.9.14
#1.3.28"<= wine < 1.7.12 needs patch 2ac0c877f591be14815902b527f314a915eee147 but I couldn't find it so I disabled cups in the configuration

#patches yylex 1-3
#1.3.28 <= wine < 1.7.0
#patch yylex 4 - jscript
#1.1.10 <= wine < 1.7.0
#patch yylex 5 - vbscript
#1.3.28 <= wine < 1.7.0
#patch yylex 6
#1.5.7 <= wine < 1.7.0

    do_build 1.9.0 wglext patch ddraw_waitvblank.patch patch wine18-19-gnutls.patch patch wine17-19-cups.patch
    do_build 1.8 wglext patch ddraw_waitvblank.patch patch wine18-19-gnutls.patch patch wine17-19-cups.patch
    do_build 1.7.0 wglext depends patch ddraw_waitvblank_17.patch
    do_build 1.6 wglext depends patch ddraw_waitvblank_17.patch patch yylex.1.patch patch yylex.2.patch patch yylex.3.patch patch yylex.4.patch patch yylex.5.patch patch yylex.6.patch
fi

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
  if [[ $1 == "oldFont" ]]
  then
    OLDFONT="1";
    shift
  fi
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
      else
        echo "unknown do_build option: $1"
        exit
      fi
    done
    rm -rf dlls/winex11.drv/*
    cp -r ../../wineboxed.drv/*.* dlls/winex11.drv/
    #if [[ $OLDFONT -eq "1" ]]
    #then
      #EXTRA_ARGS="FREETYPE_LIBS=libfreetype.so.6.12.3"
    #fi
    ./configure CFLAGS="-O2 -march=pentium4 $WGLEXT" --without-pulse --without-alsa --without-dbus --without-sane --without-hal --prefix=/opt/wine --disable-tests $EXTRA_ARGS
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
    echo './configure CFLAGS="-O2 -march=pentium4" --without-pulse --without-alsa  --without-dbus --without-sane --without-hal --prefix=/opt/wine --disable-tests' >> build.txt
    echo "make -j4" >> build.txt
    zip -r ../wine-$VERSION.zip *
    cd ../wine-git
    make clean
    rm -rf ../tmp_install
  fi
}

do_build 5.0 patch wine5-lz.patch
do_build 4.0
do_build 3.1
#needs an older version of fontconfig, maybe this will have to run on a Deb 8 VM
#do_build 2.0 oldFont wglext
#do_build 1.9 oldFont wglext
#do_build 1.8 oldFont wglext
#do_build 1.7 oldFont wglext

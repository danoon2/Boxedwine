#!/bin/bash
#To get OSS to compile, I downloaded a release from 
#http://www.opensound.com/developer/sources/stable/gpl/

#And copied include/soundcard.h to /usr/lib/oss/include/sys/soundcard.h

set -e
ARG1=${1:-main}
if [ ! -d wine-git ]
then
  git clone https://github.com/wine-mirror/wine.git wine-git
fi

if [ ! -f TinyCore15WineBase.zip ]
then
  wget https://boxedwine.org/v2/2/TinyCore15WineBase.zip
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
  BVERSION=$1
  shift
  if [ ! -f ../Wine-$VERSION.zip ]
  then
    git reset --hard
    git clean -d -f
    git checkout wine-$VERSION
    mkdir ../tmp_install
    echo "git checkout wine-$VERSION" > ../tmp_install/build.txt
    if ((BVERSION >= 6230))
    then
     echo "git apply ../patches/fixSetupApiFromCrashingDuringDllDetach.patch" >> build.txt
     git apply ../patches/fixSetupApiFromCrashingDuringDllDetach.patch
    fi
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
    if ((BVERSION >= 5210))
    then
      if ((BVERSION <= 6180))
      then
        echo "git apply ../patches/wine6_font_race.patch" >> ../tmp_install/build.txt
        git apply ../patches/wine6_font_race.patch
      elif ((BVERSION <= 6210))
      then
        echo "git cherry-pick 39fea6cd1eb175cddf3cbccc1fd09fec48da881e --no-commit" >> ../tmp_install/build.txt
        git cherry-pick 39fea6cd1eb175cddf3cbccc1fd09fec48da881e --no-commit
      fi
    fi
  
    if ((BVERSION >= 9000))
    then
     echo todo auto refresh patch
    elif ((BVERSION >= 7190))
    then
      echo "skip"
      #git apply ../patches/auto_refresh/auto_refresh719.patch
    elif ((BVERSION >= 4000))
    then
      git apply ../patches/auto_refresh/auto_refresh6.patch
    else
      git apply ../patches/auto_refresh/auto_refresh3.patch
    fi
    if ((BVERSION >= 8000))
    then
      rm -rf dlls/winealsa.drv/*
      cp -r dlls/winecoreaudio.drv/* dlls/winealsa.drv/
      cp -r ../../coreaudio/* dlls/winealsa.drv/
      sed -i '/.*COREAUDIO_LIBS.*/ s/$/ -lm \$\(PTHREAD_LIBS\)/' dlls/winealsa.drv/Makefile.in
      sed -i 's/winecoreaudio/winealsa/g' dlls/winealsa.drv/Makefile.in
      echo "#include \"$(pwd)/dlls/winealsa.drv/CoreAudio/CoreAudio.c\"" > dlls/winealsa.drv/libkern/OSAtomic.h
      echo "#include \"$(pwd)/dlls/winealsa.drv/CoreAudio/CoreAudio.h\"" > dlls/winealsa.drv/CoreMIDI/CoreMIDI.h  
      mv dlls/winealsa.drv/winecoreaudio.drv.spec dlls/winealsa.drv/winealsa.drv.spec
    else
      rm -rf dlls/winealsa.drv/*
      cp -r ../../wineboxedaudio.drv/*.* dlls/winealsa.drv/
    fi

    set -x
    ./configure LDFLAGS="-s" CFLAGS="-O2 -msse2 -march=pentium4 -mfpmath=sse $EXTRA" --without-cups --without-pulse --without-dbus --without-sane --without-hal --without-udev --without-usb --without-xshape --without-xshm --without-ldap --prefix=/opt/wine --disable-tests $EXTRA_ARGS
    make -j$(getconf _NPROCESSORS_ONLN)
    #todo find another way to achieve what I want without using sudo
    sudo rm -rf /opt/wine
    sudo make install
    mkdir ../tmp_install/opt
    cp -r /opt/wine ../tmp_install/opt/
    cd ../tmp_install
    mkdir home
    mkdir home/username
    #../boxedwine -root . -zip ../TinyCore15WineBase.zip -env "WINEDLLOVERRIDES=mscoree,mshtml=" /bin/wine notfound || true
    rm -rf bin
    rm -rf dev
    rm -rf etc
    rm -rf lib
    rm -rf mnt
    rm -rf proc
    rm -rf tmp
    if [ -d opt/wine/lib/wine/i386-unix ]
    then
        if [ -f opt/wine/lib/wine/i386-unix/winemenubuilder.exe.so ]
        then
          rm opt/wine/lib/wine/i386-unix/winemenubuilder.exe.so
        fi
        if [ -f opt/wine/lib/wine/i386-unix/libwine.so.1 ]
        then
          rm opt/wine/lib/wine/i386-unix/libwine.so.1
          printf "libwine.so.1.0" > opt/wine/lib/wine/i386-unix/libwine.so.1.link
        fi
        #mv opt/wine/lib/wine/i386-unix/wineoss.drv.so opt/wine/lib/wine/i386-unix/wineoss.drv.dsp.so
    else
        rm opt/wine/lib/wine/winemenubuilder.exe.so
        rm -f opt/wine/lib/libwine.so 
        rm opt/wine/lib/libwine.so.1
        printf "libwine.so.1.0" > opt/wine/lib/libwine.so.link
        printf "libwine.so.1.0" > opt/wine/lib/libwine.so.1.link
        mv opt/wine/lib/wine/winealsa.drv.so opt/wine/lib/wine/winealsa.drv.dsp.so
    fi
    printf "$VERSION" > wineVersion.txt
    printf "Wine $VERSION" > name.txt
    #printf "debian10.zip" > depends.txt
    cp ../changes.txt changes.txt
    cp ../version.txt version.txt
    
    if [[ $REVERTS != "" ]]
    then
      echo $REVERTS >> build.txt
    fi
    if [[ $PATCHES != "" ]]
    then
      echo "Patched: $PATCHES" >> build.txt
    fi
    echo './configure LDFLAGS="-s" CFLAGS="-O2 -msse2 -march=pentium4 -mfpmath=sse $EXTRA" --without-cups --without-pulse --without-dbus --without-sane --without-hal --without-udev --without-usb --without-xshape --without-xshm --without-ldap --prefix=/opt/wine --disable-tests $EXTRA_ARGS' >> build.txt
    echo "make " >> build.txt
    cp ../TinyCore15WineBase.zip ../Wine-$VERSION.zip
    zip -ur ../Wine-$VERSION.zip *
    cd ../wine-git
    make clean
    rm -rf ../tmp_install
    if [[ -f include/wine/library.h ]]
    then
      rm include/wine/library.h
    fi
  fi
}

DVERSION=$(sed 's/\..*//' /etc/debian_version)
echo "Debian Version = $DVERSION"
if [ $DVERSION -gt 9 ]
then
# f2e5b8070776268912e1886d4516d7ddec6969fc
# kernel32: Use the Get/SetComputerName functions from kernelbase. 
# reverted because on slower systems it will fail to create a window, not sure why
  if [[ $ARG1 != "all" ]]
  then
     do_build 9.18 9180
     do_build 9.17 9170
     do_build 9.16 9160
     do_build 9.15 9150
     do_build 9.14 9140
     do_build 9.13 9130
     do_build 9.12 9120
     do_build 9.11 9110
     do_build 9.10 9100
     do_build 9.9 9090
     do_build 9.8 9080
     do_build 9.7 9070
     do_build 9.6 9060
     do_build 9.5 9050
     do_build 9.4 9040
     do_build 9.3 9030
     do_build 9.2 9020
     do_build 9.1 9010
     do_build 9.0 9000
#    do_build 9.2 9020 patch setupapidelay.patch
#    do_build 7.0 7000 patch ddraw_waitvblank.patch
#    do_build 6.2 6020 patch ddraw_waitvblank.patch
#    do_build 5.0 5000 patch ddraw_waitvblank.patch patch wine5-lz.patch revert f2e5b8070776268912e1886d4516d7ddec6969fc
#    do_build 4.1 4010 patch ddraw_waitvblank.patch
#    do_build 3.1 3010 patch ddraw_waitvblank.patch
  else
     do_build 9.0 9000 patch setupapidelay.patch
     do_build 8.0 8000
     do_build 7.0 7000 patch ddraw_waitvblank.patch
     do_build 6.0 6000 patch ddraw_waitvblank.patch
#     do_build 5.0 5000 patch ddraw_waitvblank.patch patch wine5-lz.patch revert f2e5b8070776268912e1886d4516d7ddec6969fc
#     do_build 4.1 4010 patch ddraw_waitvblank.patch
#     do_build 3.1 3010 patch ddraw_waitvblank.patch
  fi
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

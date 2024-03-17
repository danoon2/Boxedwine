#!/bin/bash
#To get OSS to compile, I downloaded a release from 
#http://www.opensound.com/developer/sources/stable/gpl/

#And copied include/soundcard.h to /usr/lib/oss/include/sys/soundcard.h

set -e
ARG1=${1:-main}
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
  BVERSION=$1
  EXTRA=" -DBOXED_WINE_VERSION=$BVERSION"
  shift
  if ((BVERSION > 6000))
  then
    EXTRA+=" -DBOXEDWINE_VULKAN"
  fi
  if [[ $1 == "wglext" ]]
  then
    EXTRA+=" -DBOXED_NEED_WGLEXT"
    shift
  fi
  if ((BVERSION <= 7110))
  then
    EXTRA+=' -DINCLUDE_UNICODE=\"wine/unicode.h\"'
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
    if ((BVERSION >= 9000))
    then
      sed -i 's/MAKE_DEP_UNIX/#pragma makedep unix/g' dlls/winex11.drv/*.c
      cp dlls/winex11.drv/Makefile.9000.in dlls/winex11.drv/Makefile.in
      cp dlls/winex11.drv/winex11.drv.9000.spec dlls/winex11.drv/winex11.drv.spec
    elif ((BVERSION == 7110))
    then
      cp dlls/winex11.drv/Makefile.7110.in dlls/winex11.drv/Makefile.in
    elif ((BVERSION > 7110))
    then
      sed -i 's/MAKE_DEP_UNIX/#pragma makedep unix/g' dlls/winex11.drv/*.c
      cp dlls/winex11.drv/Makefile.7120.in dlls/winex11.drv/Makefile.in
      cp dlls/winex11.drv/winex11.drv.7120.spec dlls/winex11.drv/winex11.drv.spec
    elif ((BVERSION >= 6170))
    then
      cp dlls/winex11.drv/Makefile.6170.in dlls/winex11.drv/Makefile.in
    fi
    if ((BVERSION >= 9000))
    then
     echo todo auto refresh patch
    elif ((BVERSION >= 7190))
    then
      git apply ../patches/auto_refresh/auto_refresh719.patch
    elif ((BVERSION >= 4000))
    then
      git apply ../patches/auto_refresh/auto_refresh6.patch
    else
      git apply ../patches/auto_refresh/auto_refresh3.patch
    fi
#    rm -rf dlls/winealsa.drv/*
#    cp -r ../../wineboxedaudio.drv/*.* dlls/winealsa.drv/
#    if ((BVERSION >= 9000))
#    then
#      cp ../../wineboxedaudio.drv/winealsa.drv.spec.9000 dlls/winealsa.drv/winealsa.drv.spec
#    fi
    if [[ $ADD_DEPENDS == 1 ]]
    then
      echo "@MAKE_DLL_RULES@" >> dlls/winex11.drv/Makefile.in
#      echo "@MAKE_DLL_RULES@" >> dlls/winealsa.drv/Makefile.in
    fi
#for some reason I don't understand the config process will fail when looking for dependencies because the header does not exist, even though I wrapped it in a #ifdef
    touch include/wine/wglext.h
    touch include/wine/library.h
    touch include/wine/vulkan.h
    touch include/wine/vulkan_driver.h
    if ((BVERSION >= 8020))
    then
      echo "#include \"imm.h\"" > dlls/winex11.drv/boxed_imm.h
      echo "#include \"immdev.h\"" >> dlls/winex11.drv/boxed_imm.h
    elif ((BVERSION >= 7130))
    then
      echo "#include \"ddk/imm.h\"" > dlls/winex11.drv/boxed_imm.h
    else
      echo "#include \"imm.h\"" > dlls/winex11.drv/boxed_imm.h
      echo "#include \"ddk/imm.h\"" >> dlls/winex11.drv/boxed_imm.h
    fi
    if ((BVERSION < 7000))
    then
      echo "" > dlls/winex11.drv/unixlib.h
    fi
    ./configure LDFLAGS="-s" CFLAGS="-O2 -msse2 -march=pentium4 -mfpmath=sse $EXTRA" --without-cups --without-pulse --without-dbus --without-sane --without-hal --prefix=/opt/wine --disable-tests $EXTRA_ARGS
    make -j$(getconf _NPROCESSORS_ONLN)
    #todo find another way to achieve what I want without using sudo
    sudo rm -rf /opt/wine
    sudo make install
    mkdir ../tmp_install
    mkdir ../tmp_install/opt
    cp -r /opt/wine ../tmp_install/opt/
    cd ../tmp_install
    mkdir home
    mkdir home/username
    ../boxedwine -root . -zip ../debian10.zip /bin/wine notfound || true
    if [ -d opt/wine/lib/wine/i386-unix ]
    then
        rm opt/wine/lib/wine/i386-unix/winemenubuilder.exe.so
        if [ -f opt/wine/lib/wine/i386-unix/libwine.so.1 ]
        then
          rm opt/wine/lib/wine/i386-unix/libwine.so.1
          printf "libwine.so.1.0" > opt/wine/lib/wine/i386-unix/libwine.so.1.link
        fi
        mv opt/wine/lib/wine/i386-unix/wineoss.drv.so opt/wine/lib/wine/i386-unix/wineoss.drv.dsp.so
    else
        rm opt/wine/lib/wine/winemenubuilder.exe.so
        rm -f opt/wine/lib/libwine.so 
        rm opt/wine/lib/libwine.so.1
        printf "libwine.so.1.0" > opt/wine/lib/libwine.so.link
        printf "libwine.so.1.0" > opt/wine/lib/libwine.so.1.link
        mv opt/wine/lib/wine/wineoss.drv.so opt/wine/lib/wine/wineoss.drv.dsp.so
    fi
    printf "Wine $VERSION" > wineVersion.txt
    printf "debian10.zip" > depends.txt
    cp ../changes.txt changes.txt
    cp ../version.txt version.txt
    echo "git checkout wine-$VERSION" > build.txt
    if [[ $REVERTS != "" ]]
    then
      echo $REVERTS >> build.txt
    fi
    if [[ $PATCHES != "" ]]
    then
      echo "Patched: $PATCHES" >> build.txt
    fi
    echo './configure CFLAGS="-O2 -msse2 -march=pentium4 -mfpmath=sse $EXTRA" --without-cups --without-pulse --without-dbus --without-sane --without-hal --prefix=/opt/wine --disable-tests $EXTRA_ARGS' >> build.txt
    echo "make " >> build.txt
    zip -r ../Wine-$VERSION.zip *
    cd ../wine-git
    make clean
    rm -rf ../tmp_install
    if [[ -f include/wine/library.h ]]
    then
      rm include/wine/library.h
    fi
  fi
}

VERSION=$(sed 's/\..*//' /etc/debian_version)
echo "Debian Version = $VERSION"
if [ $VERSION -gt 9 ]
then
# f2e5b8070776268912e1886d4516d7ddec6969fc
# kernel32: Use the Get/SetComputerName functions from kernelbase. 
# reverted because on slower systems it will fail to create a window, not sure why
  if [[ $ARG1 != "all" ]]
  then
    do_build 9.2 9020 patch setupapidelay.patch
    do_build 7.0 7000 patch ddraw_waitvblank.patch
    do_build 6.0 6000 patch ddraw_waitvblank.patch
    do_build 5.0 5000 patch ddraw_waitvblank.patch patch wine5-lz.patch revert f2e5b8070776268912e1886d4516d7ddec6969fc
    do_build 4.1 4010 patch ddraw_waitvblank.patch
    do_build 3.1 3010 patch ddraw_waitvblank.patch
  else
    do_build 8.2 8020
    do_build 8.1 8010
    do_build 8.0 8000
    do_build 7.22 7220
    do_build 7.21 7210
    do_build 7.20 7200
    do_build 7.19 7190
    do_build 7.18 7180
    do_build 7.17 7170
    do_build 7.16 7160
    do_build 7.15 7150
    do_build 7.14 7140
    do_build 7.13 7130
    do_build 7.12 7120
    do_build 7.11 7110
    do_build 7.10 7100
    do_build 7.9 7090
    do_build 7.8 7080
    do_build 7.7 7070
    do_build 7.6 7060
    do_build 7.5 7050
    do_build 7.4 7040
    do_build 7.3 7030
    do_build 7.2 7020
    do_build 7.1 7010
    do_build 7.0 7000 patch ddraw_waitvblank.patch
    do_build 6.23 6230 patch ddraw_waitvblank.patch
    do_build 6.22 6220 patch ddraw_waitvblank.patch
    do_build 6.21 6210 patch ddraw_waitvblank.patch
    do_build 6.20 6200 patch ddraw_waitvblank.patch
    do_build 6.19 6190 patch ddraw_waitvblank.patch
    do_build 6.18 6180 patch ddraw_waitvblank.patch
    do_build 6.17 6170 patch ddraw_waitvblank.patch
    do_build 6.16 6160 patch ddraw_waitvblank.patch
    do_build 6.15 6150 patch ddraw_waitvblank.patch
    do_build 6.14 6140 patch ddraw_waitvblank.patch
    do_build 6.13 6130 patch ddraw_waitvblank.patch
    do_build 6.12 6120 patch ddraw_waitvblank.patch
    do_build 6.11 6110 patch ddraw_waitvblank.patch
    do_build 6.10 6100 patch ddraw_waitvblank.patch
    do_build 6.9 6090 patch ddraw_waitvblank.patch
    do_build 6.8 6080 patch ddraw_waitvblank.patch
    do_build 6.7 6070 patch ddraw_waitvblank.patch
    do_build 6.6 6060 patch ddraw_waitvblank.patch
    do_build 6.5 6050 patch ddraw_waitvblank.patch
    do_build 6.4 6040 patch ddraw_waitvblank.patch
    do_build 6.3 6030 patch ddraw_waitvblank.patch
    do_build 6.2 6020 patch ddraw_waitvblank.patch
    do_build 6.1 6010 patch ddraw_waitvblank.patch
    do_build 6.0 6000 patch ddraw_waitvblank.patch
    do_build 5.0 5000 patch ddraw_waitvblank.patch patch wine5-lz.patch revert f2e5b8070776268912e1886d4516d7ddec6969fc
    do_build 4.1 4010 patch ddraw_waitvblank.patch
    do_build 3.1 3010 patch ddraw_waitvblank.patch
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

#/bin/sh
emcc -std=c++11 -Wall -Wno-unused-variable -Wno-parentheses-equality -Wno-unused-function -I../../include ../../source/sdl/*.cpp ../../platform/linux/*.cpp ../../source/emulation/cpu/*.cpp ../../source/emulation/cpu/common/*.cpp ../../source/emulation/cpu/normal/*.cpp ../../source/emulation/softmmu/*.cpp ../../source/io/*.cpp ../../source/kernel/*.cpp ../../source/kernel/devs/*.cpp ../../source/kernel/proc/*.cpp ../../source/kernel/loader/*.cpp ../../source/util/*.cpp ../../source/opengl/sdl/*.cpp ../../source/opengl/*.cpp -o boxedwine.html -O3 -DUNALIGNED_MEMORY=1 -DUSE_MMU -s USE_SDL=2 -DSDL2 -s TOTAL_MEMORY=536870912  --shell-file shellfs.html -s EXTRA_EXPORTED_RUNTIME_METHODS='["addRunDependency", "removeRunDependency"]'
if [ "$1" = "remote" ] ; then
    zip -r9qdgds 10m boxedwine.zip root -x root/lib/wine/**\*
    if [ -d "dlls" ]; then
        STR="{\"dlls\": {"
        for file in `find dlls -maxdepth 1 -type f -name "*.*"`; do
            STR="${STR}\"${file#*/}\": null  ,"
        done
        STR="${STR},,"

        SUB_STR="\"fakedlls\": {"
        for file in `find dlls/fakedlls -type f -name "*.*"`; do
            file="${file#*/}";
            file="${file#*/}";
            file="${file#*/}";
            SUB_STR="${SUB_STR}\"${file#*/}\": null  ,"
        done
        SUB_STR="${SUB_STR},,"
        SUB_STR="${SUB_STR/,,,/}}"

        if [ ${#SUB_STR} == 16 ]; then
            COMPLETE="${STR/,,,/}}}"
        else
            COMPLETE="${STR/,,,/},${SUB_STR}}}"
        fi
        echo ${COMPLETE} > dlls.json

    fi
else
	zip -r9qdgds 10m boxedwine.zip root
fi

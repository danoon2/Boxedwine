#/bin/sh
emcc -Wall -Wno-unused-variable -Wno-parentheses-equality -Wno-unused-function -I../../include ../../source/sdl/*.c ../../platform/linux/*.c ../../source/emulation/*.c ../../source/emulation/cpu/*.c ../../source/emulation/softmmu/*.c ../../source/io/*.c ../../source/kernel/*.c ../../source/kernel/devs/*.c ../../source/kernel/proc/*.c ../../source/kernel/loader/*.c ../../source/util/*.c ../../source/util/pbl/*.c ../../source/opengl/sdl/*.c ../../source/opengl/*.c -o boxedwine.html -O2 -DUNALIGNED_MEMORY=1 -DUSE_MMU -s WASM=1 -s USE_SDL=2 -DSDL2 -s TOTAL_MEMORY=536870912  --shell-file shellfs.html -s EXTRA_EXPORTED_RUNTIME_METHODS='["addRunDependency", "removeRunDependency"]'
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

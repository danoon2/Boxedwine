#!/bin/bash
# Source from the workspace root, including workspaces restored from a test stash.
# The Mac's SSH and Jenkins accounts share the installed tools, but not the cache.
boxedwine_em_cache="${EM_CACHE:-}"
case "$(uname -s)" in
    Darwin)
        boxedwine_emsdk_dir="${BOXEDWINE_EMSDK_ROOT:-/Users/Shared/BoxedwineCI/emsdk}"
        boxedwine_em_cache="${EM_CACHE:-$HOME/Library/Caches/Boxedwine/emscripten-5.0.7}"
        export BOXEDWINE_FIREFOX="${BOXEDWINE_FIREFOX:-/Applications/Firefox.app/Contents/MacOS/firefox}"
        ;;
    *)
        boxedwine_emsdk_dir="${BOXEDWINE_EMSDK_ROOT:-$HOME/emsdk}"
        export BOXEDWINE_FIREFOX="${BOXEDWINE_FIREFOX:-/usr/bin/firefox}"
        ;;
esac

if [ ! -f "$boxedwine_emsdk_dir/emsdk_env.sh" ]; then
    echo "Emscripten SDK not found at $boxedwine_emsdk_dir" >&2
    return 1
fi
source "$boxedwine_emsdk_dir/emsdk_env.sh"
if [ -n "$boxedwine_em_cache" ]; then
    # emsdk_env.sh clears EM_CACHE when it changes the active SDK environment.
    export EM_CACHE="$boxedwine_em_cache"
fi
if [ -n "${EMSDK_PYTHON:-}" ]; then
    # Make's build helpers also need the SDK Python, not Apple's older Python.
    export PATH="$(dirname "$EMSDK_PYTHON"):$PATH"
fi

boxedwine_emcc_version=$(emcc -dumpversion)
if [ "$boxedwine_emcc_version" != '5.0.7' ]; then
    echo "Emscripten 5.0.7 is required; found $boxedwine_emcc_version. Activate 5.0.7 in $boxedwine_emsdk_dir." >&2
    return 1
fi
if [ ! -x "$BOXEDWINE_FIREFOX" ]; then
    echo "Firefox not found at $BOXEDWINE_FIREFOX" >&2
    return 1
fi

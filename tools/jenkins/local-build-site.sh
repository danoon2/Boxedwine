#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
EMSCRIPTEN_DIR="$ROOT_DIR/project/emscripten"

SITE_DIR="${LOCAL_BUILD_SITE_DIR:-$ROOT_DIR/project/linux/Deploy/LocalBuildSite}"
MIRROR_URL="${BUILD_SITE_MIRROR_URL:-https://boxedwine.org/builds/}"
LOCAL_PUBLIC_URL="${BUILD_SITE_PUBLIC_URL:-}"
TITLE="${BUILD_SITE_TITLE:-Boxedwine Local Builds}"
EMSDK_DIR="${EMSDK_DIR:-/home/james/emsdk}"
BUILDFILES_DIR="${BUILDFILES_DIR:-/var/www/buildfiles}"
BOXEDWINE_ZIP_URL="http://boxedwine.org/v2/demos/boxedwine.1.zip"
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-8000}"
BUILD_NUMBER="${BUILD_NUMBER:-$(date -u +%Y%m%d%H%M%S)}"
BUILD_RESULT="${BUILD_RESULT:-LOCAL}"
KEEP="${BUILD_SITE_KEEP:-5}"
SKIP_SYNC="${LOCAL_BUILD_SITE_SKIP_SYNC:-0}"
SKIP_BUILD="${LOCAL_BUILD_SITE_SKIP_BUILD:-0}"
NO_SERVER="${LOCAL_BUILD_SITE_NO_SERVER:-0}"
DRY_RUN=0

BRANCH_NAME="${BRANCH_NAME:-$(git -C "$ROOT_DIR" rev-parse --abbrev-ref HEAD 2>/dev/null || echo local)}"
GIT_COMMIT="${GIT_COMMIT:-$(git -C "$ROOT_DIR" rev-parse HEAD 2>/dev/null || true)}"
DEMO_SOURCE="${BUILD_SITE_DEMOS_SOURCE:-$SITE_DIR/demos/apps}"
SINGLE_THREADED_DIR="${BUILD_SITE_SINGLE_THREADED_DIR:-$EMSCRIPTEN_DIR/Deploy/Web/SingleThreaded}"
MULTI_THREADED_DIR="${BUILD_SITE_MULTI_THREADED_DIR:-$EMSCRIPTEN_DIR/Deploy/Web/MultiThreaded}"
SINGLE_THREADED_JIT_DIR="${BUILD_SITE_SINGLE_THREADED_JIT_DIR:-$EMSCRIPTEN_DIR/Deploy/Web/SingleThreadedJit}"
MULTI_THREADED_JIT_DIR="${BUILD_SITE_MULTI_THREADED_JIT_DIR:-$EMSCRIPTEN_DIR/Deploy/Web/MultiThreadedJit}"

usage() {
    cat <<EOF
Usage: tools/jenkins/local-build-site.sh [options]

Build the local Boxedwine demo website without publishing it.

Options:
  --site-dir DIR        Local website directory (default: $SITE_DIR)
  --mirror-url URL      Public site to mirror when BUILD_SITE_REMOTE is unset
                        (default: $MIRROR_URL)
  --branch NAME         Branch name recorded in the generated site
  --build-number NUM    Build number recorded in the generated site
  --emsdk DIR           Emscripten SDK directory (default: $EMSDK_DIR)
  --buildfiles-dir DIR  Extra web build files copied like Jenkins
                        (default: $BUILDFILES_DIR)
  --host HOST           Local server host (default: $HOST)
  --port PORT           Local server port (default: $PORT)
  --skip-sync           Reuse the existing local website directory
  --skip-build          Reuse existing Emscripten Deploy/Web outputs
  --no-server           Generate the site but do not start a serve
  --dry-run             Print the workflow without changing files
  -h, --help            Show this help

If BUILD_SITE_REMOTE is set, the script uses rsync from that remote. Otherwise
it falls back to wget mirroring $MIRROR_URL.
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --site-dir)
            SITE_DIR="$2"
            DEMO_SOURCE="${BUILD_SITE_DEMOS_SOURCE:-$SITE_DIR/demos/apps}"
            shift 2
            ;;
        --mirror-url)
            MIRROR_URL="$2"
            shift 2
            ;;
        --branch)
            BRANCH_NAME="$2"
            shift 2
            ;;
        --build-number)
            BUILD_NUMBER="$2"
            shift 2
            ;;
        --emsdk)
            EMSDK_DIR="$2"
            shift 2
            ;;
        --buildfiles-dir)
            BUILDFILES_DIR="$2"
            shift 2
            ;;
        --host)
            HOST="$2"
            shift 2
            ;;
        --port)
            PORT="$2"
            shift 2
            ;;
        --skip-sync)
            SKIP_SYNC=1
            shift
            ;;
        --skip-build)
            SKIP_BUILD=1
            shift
            ;;
        --no-server)
            NO_SERVER=1
            shift
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "$1 is required but was not found in PATH." >&2
        exit 1
    fi
}

run() {
    echo "+ $*"
    if [ "$DRY_RUN" != "1" ]; then
        "$@"
    fi
}

mirror_site() {
    if [ "$SKIP_SYNC" = "1" ]; then
        echo "Skipping site sync; using $SITE_DIR"
        return
    fi

    run mkdir -p "$SITE_DIR"

    if [ -n "${BUILD_SITE_REMOTE:-}" ]; then
        require_command rsync
        RSYNC_ARGS=()
        if [ -n "${BUILD_SITE_SSH_KEY:-}" ]; then
            RSYNC_ARGS=(-e "ssh -i ${BUILD_SITE_SSH_KEY}")
        fi
        echo "Syncing existing build site from $BUILD_SITE_REMOTE"
        if [ "$DRY_RUN" = "1" ]; then
            echo "+ rsync -az ${RSYNC_ARGS[*]} ${BUILD_SITE_REMOTE}/ $SITE_DIR/"
        else
            rsync -az "${RSYNC_ARGS[@]}" "${BUILD_SITE_REMOTE}/" "$SITE_DIR/" || true
        fi
        return
    fi

    require_command wget
    echo "Mirroring existing build site from $MIRROR_URL"
    mirror_with_wget "$MIRROR_URL"
    mirror_with_wget "${MIRROR_URL%/}/demos/"
    hydrate_demo_apps_from_mirror
}

mirror_with_wget() {
    local url="$1"
    local wget_args=(
        --mirror
        --page-requisites
        --convert-links
        --adjust-extension
        --no-parent
        --no-host-directories
        --cut-dirs=1
        --directory-prefix "$SITE_DIR"
        "$url"
    )

    echo "Fetching $url"
    echo "+ wget ${wget_args[*]}"
    if [ "$DRY_RUN" = "1" ]; then
        return
    fi
    wget "${wget_args[@]}" || echo "Warning: wget reported missing URLs while mirroring $url; continuing with downloaded files."
}

hydrate_demo_apps_from_mirror() {
    local demos_index="$SITE_DIR/demos/index.html"
    local apps_dir="$SITE_DIR/demos/apps"
    local apps_url="${MIRROR_URL%/}/demos/apps"

    if [ ! -f "$demos_index" ]; then
        return
    fi

    mkdir -p "$apps_dir"

    echo "Hydrating demo app zips from $apps_url"
    if [ "$DRY_RUN" = "1" ]; then
        echo "+ parse $demos_index for root/app/overlay zip parameters"
        echo "+ wget -N changed demo zips and demos.json into $apps_dir"
        return
    fi

    python3 - "$demos_index" <<'PY' | while IFS= read -r zip_name; do
import html
import re
import sys
from pathlib import Path
from urllib.parse import parse_qsl, unquote

text = Path(sys.argv[1]).read_text(encoding="utf-8", errors="ignore")
zips = set()
for href in re.findall(r'href="([^"]+)"', text):
    decoded = html.unescape(unquote(href))
    if "?" not in decoded:
        continue
    query = decoded.split("?", 1)[1]
    if query.endswith(".html"):
        query = query[:-5]
    for key, value in parse_qsl(query, keep_blank_values=True):
        if key in {"root", "app", "overlay"} and value.endswith(".zip"):
            zips.add(value)
for zip_name in sorted(zips):
    print(zip_name)
PY
        if [ "$zip_name" = "boxedwine.zip" ]; then
            continue
        fi
        wget -N -P "$apps_dir" "$apps_url/$zip_name"
    done

    wget -N -P "$apps_dir" "$apps_url/demos.json" || true
}

ensure_boxedwine_zip() {
    local apps_dir="$1"

    echo "Downloading boxedwine.zip from $BOXEDWINE_ZIP_URL"
    if [ "$DRY_RUN" = "1" ]; then
        echo "+ mkdir -p $apps_dir"
        echo "+ wget -O $apps_dir/boxedwine.zip $BOXEDWINE_ZIP_URL"
        return
    fi

    mkdir -p "$apps_dir"
    wget -O "$apps_dir/boxedwine.zip" "$BOXEDWINE_ZIP_URL"
}

copy_web_build() {
    local build_dir="$1"
    local destination_dir="$2"

    run rm -rf "$destination_dir"
    run mkdir -p "$destination_dir"

    run cp "$build_dir/boxedwine.html" "$destination_dir/"
    run cp "$EMSCRIPTEN_DIR/boxedwine.css" "$destination_dir/"
    run cp "$EMSCRIPTEN_DIR/boxedwine-shell.js" "$destination_dir/"
    run cp "$build_dir/boxedwine.js" "$destination_dir/"
    run cp "$build_dir/boxedwine.wasm" "$destination_dir/"

    if [ -d "$BUILDFILES_DIR" ] && compgen -G "$BUILDFILES_DIR/*" >/dev/null; then
        run cp "$BUILDFILES_DIR"/* "$destination_dir/"
    else
        echo "Warning: $BUILDFILES_DIR is missing or empty; skipping Jenkins buildfiles copy."
    fi
}

build_emscripten() {
    if [ "$SKIP_BUILD" = "1" ]; then
        echo "Skipping Emscripten build; using existing Emscripten Deploy/Web outputs"
        return
    fi

    require_command make
    if [ ! -f "$EMSDK_DIR/emsdk_env.sh" ]; then
        echo "Could not find $EMSDK_DIR/emsdk_env.sh" >&2
        exit 1
    fi

    if [ "$DRY_RUN" = "1" ]; then
        echo "+ source $EMSDK_DIR/emsdk_env.sh"
        echo "+ cd $EMSCRIPTEN_DIR"
        echo "+ rm -rf Deploy/Web"
        echo "+ make clean"
        echo "+ make release"
        echo "+ copy Build/Release to $SINGLE_THREADED_DIR"
        echo "+ make multiThreaded"
        echo "+ copy Build/MultiThreaded to $MULTI_THREADED_DIR"
        echo "+ make jit"
        echo "+ copy Build/Jit to $SINGLE_THREADED_JIT_DIR"
        echo "+ make multiThreadedJit"
        echo "+ copy Build/MultiThreadedJit to $MULTI_THREADED_JIT_DIR"
        return
    fi

    # shellcheck source=/dev/null
    source "$EMSDK_DIR/emsdk_env.sh"
    cd "$EMSCRIPTEN_DIR"
    rm -rf Deploy/Web
    make clean
    make release
    if [ ! -f "Build/Release/boxedwine.wasm" ]; then
        echo "Build/Release/boxedwine.wasm was not created." >&2
        exit 1
    fi
    copy_web_build "$EMSCRIPTEN_DIR/Build/Release" "$SINGLE_THREADED_DIR"
    make multiThreaded
    if [ ! -f "Build/MultiThreaded/boxedwine.wasm" ]; then
        echo "Build/MultiThreaded/boxedwine.wasm was not created." >&2
        exit 1
    fi
    copy_web_build "$EMSCRIPTEN_DIR/Build/MultiThreaded" "$MULTI_THREADED_DIR"
    make jit
    if [ ! -f "Build/Jit/boxedwine.wasm" ]; then
        echo "Build/Jit/boxedwine.wasm was not created." >&2
        exit 1
    fi
    copy_web_build "$EMSCRIPTEN_DIR/Build/Jit" "$SINGLE_THREADED_JIT_DIR"
    make multiThreadedJit
    if [ ! -f "Build/MultiThreadedJit/boxedwine.wasm" ]; then
        echo "Build/MultiThreadedJit/boxedwine.wasm was not created." >&2
        exit 1
    fi
    copy_web_build "$EMSCRIPTEN_DIR/Build/MultiThreadedJit" "$MULTI_THREADED_JIT_DIR"
}

generate_site() {
    ensure_boxedwine_zip "$DEMO_SOURCE"

    if [ "$DRY_RUN" != "1" ]; then
        if [ ! -d "$DEMO_SOURCE" ]; then
            echo "Demo source not found: $DEMO_SOURCE" >&2
            echo "Run without --skip-sync, set BUILD_SITE_REMOTE, or set BUILD_SITE_DEMOS_SOURCE." >&2
            exit 1
        fi
        if [ ! -d "$SINGLE_THREADED_DIR" ] ||
            [ ! -d "$MULTI_THREADED_DIR" ] ||
            [ ! -d "$SINGLE_THREADED_JIT_DIR" ] ||
            [ ! -d "$MULTI_THREADED_JIT_DIR" ]; then
            echo "Emscripten web outputs are missing." >&2
            echo "Expected: $SINGLE_THREADED_DIR, $MULTI_THREADED_DIR, $SINGLE_THREADED_JIT_DIR, and $MULTI_THREADED_JIT_DIR" >&2
            exit 1
        fi
    fi

    require_command python3
    run "$ROOT_DIR/tools/jenkins/build_site.py" \
        --site-dir "$SITE_DIR" \
        --title "$TITLE" \
        --branch "$BRANCH_NAME" \
        --build-number "$BUILD_NUMBER" \
        --result "$BUILD_RESULT" \
        --commit "$GIT_COMMIT" \
        --commit-url "${GIT_URL:-}" \
        --build-url "${BUILD_URL:-}" \
        --public-url "$LOCAL_PUBLIC_URL" \
        --demo-source "$DEMO_SOURCE" \
        --single-threaded-dir "$SINGLE_THREADED_DIR" \
        --multi-threaded-dir "$MULTI_THREADED_DIR" \
        --single-threaded-jit-dir "$SINGLE_THREADED_JIT_DIR" \
        --multi-threaded-jit-dir "$MULTI_THREADED_JIT_DIR" \
        --keep "$KEEP" \
        --skip-prune-removed-demo-branches
}

start_server() {
    if [ "$NO_SERVER" = "1" ]; then
        echo "Local build site generated at $SITE_DIR"
        return
    fi

    require_command node
    echo "Serving $SITE_DIR at http://$HOST:$PORT/"
    echo "COOP/COEP headers are provided by project/emscripten/server.mjs."
    run node "$EMSCRIPTEN_DIR/server.mjs" --root "$SITE_DIR" --port "$PORT" --host "$HOST"
}

mirror_site
build_emscripten
generate_site
start_server

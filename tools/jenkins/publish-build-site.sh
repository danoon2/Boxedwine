#!/usr/bin/env bash
set -euo pipefail

if [ -z "${BUILD_SITE_REMOTE:-}" ]; then
    echo "BUILD_SITE_REMOTE is not set; skipping static build site publish."
    exit 0
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SITE_DIR="$(mktemp -d)"
BOXEDWINE_ZIP_URL="${BOXEDWINE_ZIP_URL:-https://boxedwine.org/v2/15/TinyCore15Wine11.0-web.zip}"
DEMO_ROOT_CONFIG="${BUILD_SITE_DEMO_ROOT_CONFIG:-$ROOT_DIR/tools/buildWine/webgl_filesystems_v15.json}"
REMOTE_LOCK_ACQUIRED=0
REMOTE_HOST="${BUILD_SITE_REMOTE%%:*}"
REMOTE_PATH="${BUILD_SITE_REMOTE#*:}"

remote_ssh() {
    if [ -n "${BUILD_SITE_SSH_KEY:-}" ]; then
        ssh -i "$BUILD_SITE_SSH_KEY" "$REMOTE_HOST" "$@"
    else
        ssh "$REMOTE_HOST" "$@"
    fi
}

cleanup() {
    local status=$?
    if [ "$REMOTE_LOCK_ACQUIRED" = "1" ]; then
        for attempt in 1 2 3; do
            if remote_ssh "if [ -d '$REMOTE_PATH/.publish-lock' ]; then rmdir '$REMOTE_PATH/.publish-lock'; fi"; then
                REMOTE_LOCK_ACQUIRED=0
                break
            fi
            if [ "$attempt" != "3" ]; then
                sleep 2
            fi
        done
        if [ "$REMOTE_LOCK_ACQUIRED" = "1" ]; then
            echo "Could not release publish lock on $REMOTE_HOST: $REMOTE_PATH/.publish-lock" >&2
            echo "After confirming no publish is running, remove that empty directory with rmdir before retrying." >&2
            if [ "$status" = "0" ]; then
                status=1
            fi
        fi
    fi
    rm -rf "$SITE_DIR" || true
    exit "$status"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

SSH_ARGS=()
if [ -n "${BUILD_SITE_SSH_KEY:-}" ]; then
    SSH_ARGS=(-e "ssh -i ${BUILD_SITE_SSH_KEY}")
fi

if [ "$REMOTE_HOST" != "$BUILD_SITE_REMOTE" ]; then
    remote_ssh "mkdir -p '$REMOTE_PATH'"

    for attempt in $(seq 1 60); do
        if remote_ssh "if mkdir '$REMOTE_PATH/.publish-lock' 2>/dev/null; then
            exit 0
        elif [ -d '$REMOTE_PATH/.publish-lock' ]; then
            exit 75
        else
            echo 'Could not create remote publish lock directory.' >&2
            exit 1
        fi"; then
            REMOTE_LOCK_ACQUIRED=1
            break
        else
            status=$?
            if [ "$status" != "75" ]; then
                echo "Failed to contact or create publish lock on $REMOTE_HOST (exit $status)." >&2
                exit "$status"
            fi
        fi
        sleep 2
    done

    if [ "$REMOTE_LOCK_ACQUIRED" != "1" ]; then
        echo "Could not acquire static site publish lock: $REMOTE_HOST:$REMOTE_PATH/.publish-lock" >&2
        echo "Another publish may be running, or an interrupted publish left the lock behind." >&2
        echo "After confirming no publish is running, remove that empty directory with rmdir before retrying." >&2
        exit 1
    fi
fi

# Keep coordination state out of the mirrored site and preserve it during --delete.
# A failed download must stop publication: uploading a partial mirror could delete
# existing builds from the server.
rsync -az --exclude=/.publish-lock "${SSH_ARGS[@]}" "${BUILD_SITE_REMOTE}/" "$SITE_DIR/"

ARTIFACT_ARGS=()
if [ -n "${BUILD_SITE_ARTIFACT:-}" ] && [ -f "$BUILD_SITE_ARTIFACT" ]; then
    ARTIFACT_ARGS+=(--artifact "$BUILD_SITE_ARTIFACT")
fi

LOG_ARGS=()
if [ -n "${BUILD_SITE_LOG:-}" ] && [ -f "$BUILD_SITE_LOG" ]; then
    LOG_ARGS+=(--log "$BUILD_SITE_LOG")
fi

DEMO_ARGS=()
DEMO_SOURCE="${BUILD_SITE_DEMOS_SOURCE:-$SITE_DIR/demos/apps}"
# Jenkins unstashes the web build into project/linux/Deploy/Web before this
# script runs. The Emscripten build stage populates these dirs from the
# release, multiThreaded, jit, and multiThreadedJit targets.
SINGLE_THREADED_DIR="${BUILD_SITE_SINGLE_THREADED_DIR:-$ROOT_DIR/project/linux/Deploy/Web/SingleThreaded}"
MULTI_THREADED_DIR="${BUILD_SITE_MULTI_THREADED_DIR:-$ROOT_DIR/project/linux/Deploy/Web/MultiThreaded}"
SINGLE_THREADED_JIT_DIR="${BUILD_SITE_SINGLE_THREADED_JIT_DIR:-$ROOT_DIR/project/linux/Deploy/Web/SingleThreadedJit}"
MULTI_THREADED_JIT_DIR="${BUILD_SITE_MULTI_THREADED_JIT_DIR:-$ROOT_DIR/project/linux/Deploy/Web/MultiThreadedJit}"
if [ -d "$SINGLE_THREADED_DIR" ] &&
    [ -d "$MULTI_THREADED_DIR" ] &&
    [ -d "$SINGLE_THREADED_JIT_DIR" ] &&
    [ -d "$MULTI_THREADED_JIT_DIR" ]; then
    mkdir -p "$DEMO_SOURCE"
    wget -O "$DEMO_SOURCE/TinyCore15Wine11.0-web.zip" "$BOXEDWINE_ZIP_URL"
    DEMO_ARGS+=(
        --demo-source "$DEMO_SOURCE"
        --demo-root-config "$DEMO_ROOT_CONFIG"
        --single-threaded-dir "$SINGLE_THREADED_DIR"
        --multi-threaded-dir "$MULTI_THREADED_DIR"
        --single-threaded-jit-dir "$SINGLE_THREADED_JIT_DIR"
        --multi-threaded-jit-dir "$MULTI_THREADED_JIT_DIR"
    )
else
    echo "Demo site inputs are not complete; skipping demo page update."
    echo "Expected: $SINGLE_THREADED_DIR, $MULTI_THREADED_DIR, $SINGLE_THREADED_JIT_DIR, and $MULTI_THREADED_JIT_DIR"
fi

"$ROOT_DIR/tools/jenkins/build_site.py" \
    --site-dir "$SITE_DIR" \
    --title "${BUILD_SITE_TITLE:-Boxedwine Builds}" \
    --branch "${BRANCH_NAME:-unknown}" \
    --build-number "${BUILD_NUMBER:-0}" \
    --result "${BUILD_RESULT:-SUCCESS}" \
    --commit "${GIT_COMMIT:-}" \
    --commit-url "${GIT_URL:-}" \
    --build-url "${BUILD_URL:-}" \
    "${ARTIFACT_ARGS[@]}" \
    "${LOG_ARGS[@]}" \
    "${DEMO_ARGS[@]}"

rsync -az --delete --exclude=/.publish-lock "${SSH_ARGS[@]}" "$SITE_DIR/" "$BUILD_SITE_REMOTE/"

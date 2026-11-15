#!/usr/bin/env bash
set -euo pipefail

if [ -z "${BUILD_SITE_REMOTE:-}" ]; then
    echo "BUILD_SITE_REMOTE is not set; skipping static build site publish."
    exit 0
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_RESULT="$(printf '%s' "${BUILD_RESULT:-SUCCESS}" | tr '[:lower:]' '[:upper:]')"
WORK_DIR="$(mktemp -d)"
SITE_DIR="$WORK_DIR/site"
STATE_FILE="$WORK_DIR/site-state.json"
STATE_TOOL="$ROOT_DIR/tools/jenkins/publish_site_state.py"
mkdir -p "$SITE_DIR"
DEMO_ROOT_CONFIG="${BUILD_SITE_DEMO_ROOT_CONFIG:-$ROOT_DIR/tools/buildWine/webgl_filesystems.json}"
REMOTE_LOCK_ACQUIRED=0
REMOTE_HOST="${BUILD_SITE_REMOTE%%:*}"
REMOTE_PATH="${BUILD_SITE_REMOTE#*:}"
# Quote a path as one POSIX-shell argument, including embedded apostrophes.
REMOTE_SITE_QUOTED="'${REMOTE_PATH//\'/\'\\\'\'}'"

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
            if remote_ssh "if [ -d $REMOTE_SITE_QUOTED/.publish-lock ]; then rmdir $REMOTE_SITE_QUOTED/.publish-lock; fi"; then
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
    rm -rf "$WORK_DIR" || true
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
    remote_ssh "mkdir -p $REMOTE_SITE_QUOTED"

    for attempt in $(seq 1 60); do
        if remote_ssh "if mkdir $REMOTE_SITE_QUOTED/.publish-lock 2>/dev/null; then
            exit 0
        elif [ -d $REMOTE_SITE_QUOTED/.publish-lock ]; then
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

ARTIFACT_ARGS=()
if [ "$BUILD_RESULT" = SUCCESS ] && [ -n "${BUILD_SITE_ARTIFACT:-}" ] && [ -f "$BUILD_SITE_ARTIFACT" ]; then
    ARTIFACT_ARGS+=(--artifact "$BUILD_SITE_ARTIFACT")
fi

LOG_ARGS=()
if [ -n "${BUILD_SITE_LOG:-}" ] && [ -f "$BUILD_SITE_LOG" ]; then
    LOG_ARGS+=(--log "$BUILD_SITE_LOG")
fi
if [ "$BUILD_RESULT" != SUCCESS ] && [ "${#LOG_ARGS[@]}" = 0 ]; then
    echo "BUILD_SITE_LOG must name an existing log for a $BUILD_RESULT build." >&2
    exit 1
fi

DEMO_ARGS=()
SNAPSHOT_ARGS=()
# Jenkins unstashes the web build into project/linux/Deploy/Web before this
# script runs. The Emscripten build stage populates these dirs from the
# release, multiThreaded, jit, and multiThreadedJit targets.
SINGLE_THREADED_DIR="${BUILD_SITE_SINGLE_THREADED_DIR:-$ROOT_DIR/project/linux/Deploy/Web/SingleThreaded}"
MULTI_THREADED_DIR="${BUILD_SITE_MULTI_THREADED_DIR:-$ROOT_DIR/project/linux/Deploy/Web/MultiThreaded}"
SINGLE_THREADED_JIT_DIR="${BUILD_SITE_SINGLE_THREADED_JIT_DIR:-$ROOT_DIR/project/linux/Deploy/Web/SingleThreadedJit}"
MULTI_THREADED_JIT_DIR="${BUILD_SITE_MULTI_THREADED_JIT_DIR:-$ROOT_DIR/project/linux/Deploy/Web/MultiThreadedJit}"
if [ "$BUILD_RESULT" != SUCCESS ]; then
    # Old or partial outputs must not trigger downloads/validation or replace
    # the last successful demos when publishing a failed build's log.
    echo "Build result is $BUILD_RESULT; publishing the log without artifacts or demos."
elif [ -d "$SINGLE_THREADED_DIR" ] &&
    [ -d "$MULTI_THREADED_DIR" ] &&
    [ -d "$SINGLE_THREADED_JIT_DIR" ] &&
    [ -d "$MULTI_THREADED_JIT_DIR" ]; then
    SNAPSHOT_ARGS+=(--demos)
    DEMO_ARGS+=(
        --published-demo-state "$STATE_FILE"
        --demo-root-config "$DEMO_ROOT_CONFIG"
        --single-threaded-dir "$SINGLE_THREADED_DIR"
        --multi-threaded-dir "$MULTI_THREADED_DIR"
        --single-threaded-jit-dir "$SINGLE_THREADED_JIT_DIR"
        --multi-threaded-jit-dir "$MULTI_THREADED_JIT_DIR"
    )
    if [ -n "${BOXEDWINE_ZIP_URL:-}" ]; then
        DEMO_ARGS+=(--demo-root-url "$BOXEDWINE_ZIP_URL")
    fi
else
    echo "Demo site inputs are not complete; skipping demo page update."
    echo "Expected: $SINGLE_THREADED_DIR, $MULTI_THREADED_DIR, $SINGLE_THREADED_JIT_DIR, and $MULTI_THREADED_JIT_DIR"
fi

# Read only branch/build JSON and demo inventory. ZIP payloads, old runtimes,
# artifacts and screenshots stay on the server. The helper runs from stdin.
if [ "$REMOTE_HOST" != "$BUILD_SITE_REMOTE" ]; then
    remote_ssh "python3 - snapshot --site $REMOTE_SITE_QUOTED ${SNAPSHOT_ARGS[*]}" < "$STATE_TOOL" > "$STATE_FILE"
else
    python3 "$STATE_TOOL" snapshot --site "$BUILD_SITE_REMOTE" "${SNAPSHOT_ARGS[@]}" > "$STATE_FILE"
fi
python3 "$STATE_TOOL" prepare --state "$STATE_FILE" --site "$SITE_DIR"

"$ROOT_DIR/tools/jenkins/build_site.py" \
    --site-dir "$SITE_DIR" \
    --title "${BUILD_SITE_TITLE:-Boxedwine Builds}" \
    --branch "${BRANCH_NAME:-unknown}" \
    --build-number "${BUILD_NUMBER:-0}" \
    --result "$BUILD_RESULT" \
    --commit "${GIT_COMMIT:-}" \
    --commit-url "${GIT_URL:-}" \
    --build-url "${BUILD_URL:-}" \
    --keep "${BUILD_SITE_KEEP:-5}" \
    "${ARTIFACT_ARGS[@]}" \
    "${LOG_ARGS[@]}" \
    "${DEMO_ARGS[@]}"

# This is a partial site, so never use rsync --delete. Only directories explicitly
# retired by the generator are removed, after a successful upload, under the lock.
RETIRED="$(python3 "$STATE_TOOL" plan --state "$STATE_FILE" --site "$SITE_DIR")"
rsync -az "${SSH_ARGS[@]}" "$SITE_DIR/" "$BUILD_SITE_REMOTE/"
if [ "$REMOTE_HOST" != "$BUILD_SITE_REMOTE" ]; then
    remote_ssh "python3 - prune --site $REMOTE_SITE_QUOTED --paths-base64 '$RETIRED'" < "$STATE_TOOL"
else
    python3 "$STATE_TOOL" prune --site "$BUILD_SITE_REMOTE" --paths-base64 "$RETIRED"
fi

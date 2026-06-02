#!/usr/bin/env bash
set -euo pipefail

if [ -z "${BUILD_SITE_REMOTE:-}" ]; then
    echo "BUILD_SITE_REMOTE is not set; skipping static build site publish."
    exit 0
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SITE_DIR="$(mktemp -d)"
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
    if [ "$REMOTE_LOCK_ACQUIRED" = "1" ]; then
        remote_ssh "rmdir '$REMOTE_PATH/.publish-lock'"
    fi
    rm -rf "$SITE_DIR"
}
trap cleanup EXIT

SSH_ARGS=()
if [ -n "${BUILD_SITE_SSH_KEY:-}" ]; then
    SSH_ARGS=(-e "ssh -i ${BUILD_SITE_SSH_KEY}")
fi

if [ "$REMOTE_HOST" != "$BUILD_SITE_REMOTE" ]; then
    remote_ssh "mkdir -p '$REMOTE_PATH'"

    for attempt in $(seq 1 60); do
        if remote_ssh "mkdir '$REMOTE_PATH/.publish-lock'" 2>/dev/null; then
            REMOTE_LOCK_ACQUIRED=1
            break
        fi
        sleep 2
    done

    if [ "$REMOTE_LOCK_ACQUIRED" != "1" ]; then
        echo "Could not acquire static site publish lock."
        exit 1
    fi
fi

rsync -az "${SSH_ARGS[@]}" "${BUILD_SITE_REMOTE}/" "$SITE_DIR/" || true

ARTIFACT_ARGS=()
if [ -n "${BUILD_SITE_ARTIFACT:-}" ] && [ -f "$BUILD_SITE_ARTIFACT" ]; then
    ARTIFACT_ARGS+=(--artifact "$BUILD_SITE_ARTIFACT")
fi

DEMO_ARGS=()
DEMO_SOURCE="${BUILD_SITE_DEMOS_SOURCE:-$SITE_DIR/demos/apps}"
SINGLE_THREADED_DIR="${BUILD_SITE_SINGLE_THREADED_DIR:-$ROOT_DIR/project/linux/Deploy/Web/SingleThreaded}"
MULTI_THREADED_DIR="${BUILD_SITE_MULTI_THREADED_DIR:-$ROOT_DIR/project/linux/Deploy/Web/MultiThreaded}"
if [ -d "$DEMO_SOURCE" ] && [ -d "$SINGLE_THREADED_DIR" ] && [ -d "$MULTI_THREADED_DIR" ]; then
    DEMO_ARGS+=(
        --demo-source "$DEMO_SOURCE"
        --single-threaded-dir "$SINGLE_THREADED_DIR"
        --multi-threaded-dir "$MULTI_THREADED_DIR"
    )
else
    echo "Demo site inputs are not complete; skipping demo page update."
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
    "${DEMO_ARGS[@]}"

rsync -az --delete "${SSH_ARGS[@]}" "$SITE_DIR/" "$BUILD_SITE_REMOTE/"

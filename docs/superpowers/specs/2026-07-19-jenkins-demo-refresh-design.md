# Jenkins Local Demo Refresh Design

## Goal

Make the local Jenkins demo-site workflow refresh demo application ZIPs and `demos.json` when the public copies are newer, while avoiding unnecessary downloads of unchanged files.

## Scope

Change `tools/jenkins/local-build-site.sh` and its documentation in `tools/jenkins/instructions.md`. Do not change `tools/jenkins/publish-build-site.sh`: it creates a fresh temporary site directory and synchronizes the remote site with `rsync -az`, which already transfers files whose size or modification time differs.

## Download Behavior

In `hydrate_demo_apps_from_mirror`:

- Continue deriving demo ZIP names from the mirrored demos page.
- Continue skipping `boxedwine.zip` in this loop because `ensure_boxedwine_zip` downloads it unconditionally from its dedicated URL later.
- Remove the early return that skips a demo ZIP merely because it exists locally.
- Replace `wget -nc` with `wget -N` for each parsed demo ZIP.
- Replace `wget -nc` with `wget -N` for `demos.json`.

The server provides `Last-Modified` headers. `wget -N` will retain an unchanged local file and download a public file when its timestamp indicates that it is newer. ZIP download failures will remain fatal under `set -e`; the existing best-effort `|| true` behavior for `demos.json` will remain unchanged.

## User-Facing Output and Documentation

Update dry-run output so it says the workflow refreshes changed demo ZIPs and `demos.json`, rather than downloading only missing ZIPs. Update `tools/jenkins/instructions.md` to document timestamp-aware refresh behavior and clarify that `--skip-sync` deliberately bypasses it.

## Verification

Verification will cover:

1. A failing pre-change assertion that detects the current existence short-circuit and `wget -nc` commands.
2. Shell syntax with `bash -n tools/jenkins/local-build-site.sh`.
3. Static assertions that the existence short-circuit is gone and both demo downloads use `wget -N`.
4. A temporary WSL download check showing that `wget -N` downloads an absent file, leaves an unchanged file alone, and refreshes a deliberately older local copy.
5. Documentation assertions for the refresh and `--skip-sync` behavior.

No full Emscripten rebuild is required because the change affects only demo-site asset synchronization.

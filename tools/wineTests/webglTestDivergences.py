#!/usr/bin/env python3
"""Validate the classified Wine test adaptations and split WebGL patch series."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from collections import Counter
from pathlib import Path


ALLOWED_CATEGORIES = frozenset(
    {
        "intentional_webgl_limit",
        "unimplemented_emulation",
        "defect_to_fix",
    }
)
TEST_PATH_RE = re.compile(r"^diff --git a/(dlls/.*/tests/[^ ]+) b/")
PATCH_PATH_RE = re.compile(r"^diff --git a/([^ ]+) b/")
WINE_TEST_PATH_RE = re.compile(r"^dlls/.*/tests/")


class DivergenceError(RuntimeError):
    """The divergence manifest or its relationship to the patch is invalid."""


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _patch_lines(patch_path: Path) -> list[str]:
    try:
        return Path(patch_path).read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise DivergenceError(f"could not read Wine patch {patch_path}: {error}") from error


def modified_patch_files(patch_path: Path) -> set[str]:
    """Return every file modified by a git-format patch."""
    return {
        match.group(1)
        for line in _patch_lines(patch_path)
        if (match := PATCH_PATH_RE.match(line))
    }


def added_test_policy_lines(patch_path: Path) -> tuple[set[str], Counter, Counter]:
    """Return modified test files and added skip/todo policy lines."""
    current_test_file = None
    modified_test_files = set()
    skip_lines = Counter()
    todo_lines = Counter()
    for line in _patch_lines(patch_path):
        if match := TEST_PATH_RE.match(line):
            current_test_file = match.group(1)
            modified_test_files.add(current_test_file)
            continue
        if line.startswith("diff --git "):
            current_test_file = None
            continue
        if (
            current_test_file is None
            or not line.startswith("+")
            or line.startswith("+++")
        ):
            continue
        added = line[1:].strip()
        if "skip(" in added:
            skip_lines[added] += 1
        if "todo_wine_if" in added:
            todo_lines[added] += 1
    return modified_test_files, skip_lines, todo_lines


def _classified_rules(manifest: dict, policy: str) -> dict[str, tuple[int, str]]:
    classified = {}
    for group in manifest.get("classified_rules", []):
        if not isinstance(group, dict):
            raise DivergenceError("classified_rules entries must be objects")
        category = group.get("category")
        rationale = group.get("rationale")
        if category not in ALLOWED_CATEGORIES:
            raise DivergenceError(f"unknown divergence category: {category!r}")
        if not isinstance(rationale, str) or not rationale.strip():
            raise DivergenceError(
                f"divergence category {category!r} has no rationale"
            )
        rules = group.get(policy, [])
        if not isinstance(rules, list):
            raise DivergenceError(f"{category} {policy} must be an array")
        for rule in rules:
            if not isinstance(rule, dict):
                raise DivergenceError(f"{category} {policy} rule must be an object")
            marker = rule.get("marker")
            occurrences = rule.get("occurrences")
            if not isinstance(marker, str) or not marker:
                raise DivergenceError(f"{category} {policy} rule has no marker")
            if (
                not isinstance(occurrences, int)
                or isinstance(occurrences, bool)
                or occurrences <= 0
            ):
                raise DivergenceError(
                    f"{category} {policy} marker has invalid occurrences: {marker}"
                )
            if marker in classified:
                raise DivergenceError(
                    f"{policy} marker is classified more than once: {marker}"
                )
            classified[marker] = (occurrences, category)
    return classified


def _validate_patch_hash(expected_hash: object, path: Path, label: str) -> None:
    actual_hash = sha256(path)
    if not isinstance(expected_hash, str) or (
        actual_hash.lower() != expected_hash.lower()
    ):
        raise DivergenceError(
            f"Wine WebGL {label} patch SHA-256 {actual_hash} does not match "
            f"divergence manifest {expected_hash}"
        )


def load_and_validate(
    manifest_path: Path,
    production_patch_paths: tuple[Path, ...] | list[Path],
    test_patch_path: Path,
) -> dict:
    """Load a v2 manifest and require an exact ordered patch-series boundary."""
    manifest_path = Path(manifest_path)
    production_patch_paths = tuple(Path(path) for path in production_patch_paths)
    test_patch_path = Path(test_patch_path)
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except FileNotFoundError as error:
        raise DivergenceError(
            f"WebGL divergence manifest does not exist: {manifest_path}"
        ) from error
    except (OSError, json.JSONDecodeError) as error:
        raise DivergenceError(
            f"could not read WebGL divergence manifest {manifest_path}: {error}"
        ) from error
    if not isinstance(manifest, dict) or manifest.get("schema_version") != 2:
        raise DivergenceError(
            f"unsupported WebGL divergence manifest schema: {manifest_path}"
        )

    production_entries = manifest.get("production_patches")
    if not isinstance(production_entries, list) or not production_entries:
        raise DivergenceError("production_patches must be a nonempty array")
    if len(production_entries) != len(production_patch_paths):
        raise DivergenceError(
            "production patch count does not match divergence manifest: "
            f"expected {len(production_entries)}, got {len(production_patch_paths)}"
        )

    production_files = set()
    validated_production_patches = []
    patch_ids = set()
    for index, (entry, patch_path) in enumerate(
        zip(production_entries, production_patch_paths), start=1
    ):
        if not isinstance(entry, dict):
            raise DivergenceError("production_patches entries must be objects")
        patch_id = entry.get("patch_id")
        category = entry.get("category")
        declared_path = entry.get("path")
        if not isinstance(patch_id, str) or not patch_id:
            raise DivergenceError(f"production patch {index} has no patch_id")
        if patch_id in patch_ids:
            raise DivergenceError(f"duplicate production patch_id: {patch_id}")
        patch_ids.add(patch_id)
        if not isinstance(category, str) or not category:
            raise DivergenceError(
                f"production patch {patch_id!r} has no category"
            )
        if (
            not isinstance(declared_path, str)
            or Path(declared_path).name != patch_path.name
        ):
            raise DivergenceError(
                f"production patch {patch_id!r} path does not match "
                f"{patch_path.name}"
            )
        _validate_patch_hash(
            entry.get("sha256"), patch_path, f"production {patch_id!r}"
        )
        patch_files = modified_patch_files(patch_path)
        production_test_files = {
            path for path in patch_files if WINE_TEST_PATH_RE.match(path)
        }
        if production_test_files:
            raise DivergenceError(
                f"production Wine WebGL patch {patch_id!r} contains test files: "
                f"{sorted(production_test_files)}"
            )
        production_files.update(patch_files)
        validated_production_patches.append(
            {
                "patch_id": patch_id,
                "category": category,
                "path": str(patch_path.resolve()),
                "sha256": sha256(patch_path),
                "files": len(patch_files),
            }
        )

    test_entry = manifest.get("test_patch")
    if not isinstance(test_entry, dict):
        raise DivergenceError("test_patch must be an object")
    declared_test_path = test_entry.get("path")
    if (
        not isinstance(declared_test_path, str)
        or Path(declared_test_path).name != test_patch_path.name
    ):
        raise DivergenceError(
            f"test patch path does not match {test_patch_path.name}"
        )
    _validate_patch_hash(test_entry.get("sha256"), test_patch_path, "test")

    test_patch_files = modified_patch_files(test_patch_path)

    modified_files, actual_skips, actual_todos = added_test_policy_lines(
        test_patch_path
    )
    expected_files = manifest.get("modified_test_files")
    if not isinstance(expected_files, list) or not all(
        isinstance(path, str) for path in expected_files
    ):
        raise DivergenceError("modified_test_files must be an array of paths")
    if modified_files != set(expected_files):
        raise DivergenceError(
            "modified Wine test files do not match divergence manifest: "
            f"expected {sorted(expected_files)}, got {sorted(modified_files)}"
        )
    if test_patch_files != set(expected_files):
        raise DivergenceError(
            "test-only Wine WebGL patch contains files outside its manifest: "
            f"expected {sorted(expected_files)}, got {sorted(test_patch_files)}"
        )
    overlap = production_files & test_patch_files
    if overlap:
        raise DivergenceError(
            f"production and test Wine WebGL patches overlap: {sorted(overlap)}"
        )

    classified_skips = _classified_rules(manifest, "skip_rules")
    classified_todos = _classified_rules(manifest, "todo_rules")
    for label, actual, classified in (
        ("skip", actual_skips, classified_skips),
        ("todo", actual_todos, classified_todos),
    ):
        if set(actual) != set(classified):
            raise DivergenceError(
                f"classified {label} markers do not match patch: "
                f"missing {sorted(set(actual) - set(classified))}, "
                f"stale {sorted(set(classified) - set(actual))}"
            )
        mismatches = [
            f"{marker!r}: expected {classified[marker][0]}, got {count}"
            for marker, count in actual.items()
            if classified[marker][0] != count
        ]
        if mismatches:
            raise DivergenceError(
                f"classified {label} occurrence mismatch: " + "; ".join(mismatches)
            )

    manifest["_source_path"] = str(manifest_path.resolve())
    manifest["_sha256"] = sha256(manifest_path)
    manifest["_validated_production_patches"] = validated_production_patches
    manifest["_test_patch_path"] = str(test_patch_path.resolve())
    manifest["_series_counts"] = {
        "production_patches": len(validated_production_patches),
        "production_patch_files": [
            {
                "patch_id": entry["patch_id"],
                "files": entry["files"],
            }
            for entry in validated_production_patches
        ],
        "production_unique_files": len(production_files),
        "test_files": len(test_patch_files),
    }
    manifest["_policy_counts"] = {
        "modified_test_files": len(modified_files),
        "skip_calls": sum(actual_skips.values()),
        "todo_calls": sum(actual_todos.values()),
        "unique_skip_rules": len(actual_skips),
        "unique_todo_rules": len(actual_todos),
    }
    return manifest


def main(argv: list[str] | None = None) -> int:
    repo_root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(
        description="Validate the Wine WebGL production/test patch split."
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        default=Path(__file__).with_name("webgl-test-divergences-v2.json"),
    )
    parser.add_argument(
        "--production-patch",
        type=Path,
        action="append",
        dest="production_patches",
        help="ordered production patch; repeat for every production patch",
    )
    parser.add_argument(
        "--test-patch",
        type=Path,
        default=repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-tests-against-wine-11.0.patch",
    )
    arguments = parser.parse_args(argv)
    production_patches = arguments.production_patches or [
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-build-config-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-adapter-context-caps-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-shader-generation-glsl-es-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-texture-formats-transfers-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-blitter-batching-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-directdraw-runtime-presentation-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-d3dx9-assets-compatibility-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-d3dxof-parser-hardening-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-wined3d-draw-state-query-against-wine-11.0.patch",
        repo_root
        / "tools"
        / "d3dToWebGL"
        / "webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch",
    ]
    try:
        manifest = load_and_validate(
            arguments.manifest,
            production_patches,
            arguments.test_patch,
        )
    except DivergenceError as error:
        print(f"error: {error}")
        return 1
    counts = manifest["_policy_counts"]
    print(
        f"{manifest.get('manifest_id', 'WebGL divergence manifest')}: "
        f"{manifest['_series_counts']['production_patches']} production patches, "
        f"{counts['modified_test_files']} files, "
        f"{counts['skip_calls']} skip calls, {counts['todo_calls']} todo calls"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

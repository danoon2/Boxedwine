import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest


MODULE_PATH = Path(__file__).resolve().parents[1] / "webglTestDivergences.py"
REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_MANIFEST = (
    REPO_ROOT / "tools" / "wineTests" / "webgl-test-divergences-v2.json"
)
DEFAULT_PRODUCTION_PATCHES = (
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-build-config-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-adapter-context-caps-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-shader-generation-glsl-es-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-texture-formats-transfers-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-blitter-batching-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-directdraw-runtime-presentation-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-d3dx9-assets-compatibility-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-d3dxof-parser-hardening-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-wined3d-draw-state-query-against-wine-11.0.patch",
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch",
)
DEFAULT_TEST_PATCH = (
    REPO_ROOT
    / "tools"
    / "d3dToWebGL"
    / "webgl-tests-against-wine-11.0.patch"
)


def load_module():
    spec = importlib.util.spec_from_file_location(
        "webglTestDivergencesTest", MODULE_PATH
    )
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


class WebGLTestDivergenceTests(unittest.TestCase):
    def setUp(self):
        self.module = load_module()

    def manifest_for(
        self,
        production_patches,
        test_patch,
        *,
        modified_test_files,
        classified_rules,
    ):
        return {
            "schema_version": 2,
            "production_patches": [
                {
                    "patch_id": f"production-{index}",
                    "category": "test",
                    "path": patch.name,
                    "sha256": self.module.sha256(patch),
                }
                for index, patch in enumerate(production_patches, start=1)
            ],
            "test_patch": {
                "patch_id": "tests",
                "path": test_patch.name,
                "sha256": self.module.sha256(test_patch),
            },
            "modified_test_files": modified_test_files,
            "classified_rules": classified_rules,
        }

    def test_default_manifest_classifies_every_test_policy_change(self):
        manifest = self.module.load_and_validate(
            DEFAULT_MANIFEST,
            DEFAULT_PRODUCTION_PATCHES,
            DEFAULT_TEST_PATCH,
        )

        self.assertEqual(
            {
                "modified_test_files": 9,
                "skip_calls": 63,
                "todo_calls": 44,
                "unique_skip_rules": 41,
                "unique_todo_rules": 14,
            },
            manifest["_policy_counts"],
        )
        self.assertEqual(
            {
                "production_patches": 10,
                "production_patch_files": [
                    {"patch_id": "build-config", "files": 4},
                    {"patch_id": "adapter-context-caps", "files": 4},
                    {"patch_id": "shader-generation-glsl-es", "files": 11},
                    {"patch_id": "texture-formats-transfers", "files": 9},
                    {"patch_id": "blitter-batching", "files": 7},
                    {"patch_id": "directdraw-runtime-presentation", "files": 9},
                    {"patch_id": "d3dx9-assets-compatibility", "files": 28},
                    {"patch_id": "d3dxof-parser-hardening", "files": 3},
                    {"patch_id": "wined3d-draw-state-query", "files": 12},
                    {
                        "patch_id": "d3d8-d3d9-compatibility-diagnostics",
                        "files": 3,
                    },
                ],
                "production_unique_files": 77,
                "test_files": 9,
            },
            manifest["_series_counts"],
        )
        self.assertEqual(
            self.module.ALLOWED_CATEGORIES,
            frozenset(
                group["category"] for group in manifest["classified_rules"]
            ),
        )

    def test_unclassified_added_skip_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            production_patch = temp / "production.patch"
            production_patch.write_text("", encoding="utf-8")
            test_patch = temp / "tests.patch"
            test_patch.write_text(
                "diff --git a/dlls/d3d9/tests/visual.c "
                "b/dlls/d3d9/tests/visual.c\n"
                "--- a/dlls/d3d9/tests/visual.c\n"
                "+++ b/dlls/d3d9/tests/visual.c\n"
                "@@ -1 +1,2 @@\n"
                "+skip(\"New unclassified behavior.\\\\n\");\n",
                encoding="utf-8",
            )
            manifest = self.manifest_for(
                [production_patch],
                test_patch,
                modified_test_files=["dlls/d3d9/tests/visual.c"],
                classified_rules=[],
            )
            manifest_path = temp / "manifest.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            with self.assertRaisesRegex(
                self.module.DivergenceError,
                "classified skip markers do not match patch",
            ):
                self.module.load_and_validate(
                    manifest_path, [production_patch], test_patch
                )

    def test_unknown_category_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            production_patch = temp / "production.patch"
            production_patch.write_text("", encoding="utf-8")
            test_patch = temp / "tests.patch"
            test_patch.write_text(
                "diff --git a/dlls/d3d9/tests/visual.c "
                "b/dlls/d3d9/tests/visual.c\n"
                "--- a/dlls/d3d9/tests/visual.c\n"
                "+++ b/dlls/d3d9/tests/visual.c\n"
                "@@ -1 +1,2 @@\n"
                "+skip(\"Known behavior.\\\\n\");\n",
                encoding="utf-8",
            )
            manifest = self.manifest_for(
                [production_patch],
                test_patch,
                modified_test_files=["dlls/d3d9/tests/visual.c"],
                classified_rules=[
                    {
                        "category": "ignore_forever",
                        "rationale": "Invalid category for the test.",
                        "skip_rules": [
                            {
                                "marker": 'skip("Known behavior.\\\\n");',
                                "occurrences": 1,
                            }
                        ],
                        "todo_rules": [],
                    }
                ],
            )
            manifest_path = temp / "manifest.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            with self.assertRaisesRegex(
                self.module.DivergenceError, "unknown divergence category"
            ):
                self.module.load_and_validate(
                    manifest_path, [production_patch], test_patch
                )

    def test_production_patch_rejects_wine_test_files(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            production_patch = temp / "production.patch"
            production_patch.write_text(
                "diff --git a/dlls/d3d9/tests/visual.c "
                "b/dlls/d3d9/tests/visual.c\n",
                encoding="utf-8",
            )
            test_patch = temp / "tests.patch"
            test_patch.write_text("", encoding="utf-8")
            manifest = self.manifest_for(
                [production_patch],
                test_patch,
                modified_test_files=[],
                classified_rules=[],
            )
            manifest_path = temp / "manifest.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            with self.assertRaisesRegex(
                self.module.DivergenceError,
                "production Wine WebGL patch .* contains test files",
            ):
                self.module.load_and_validate(
                    manifest_path, [production_patch], test_patch
                )

    def test_test_patch_rejects_production_files(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            production_patch = temp / "production.patch"
            production_patch.write_text("", encoding="utf-8")
            test_patch = temp / "tests.patch"
            test_patch.write_text(
                "diff --git a/dlls/d3d9/device.c b/dlls/d3d9/device.c\n",
                encoding="utf-8",
            )
            manifest = self.manifest_for(
                [production_patch],
                test_patch,
                modified_test_files=[],
                classified_rules=[],
            )
            manifest_path = temp / "manifest.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            with self.assertRaisesRegex(
                self.module.DivergenceError,
                "test-only Wine WebGL patch contains files outside its manifest",
            ):
                self.module.load_and_validate(
                    manifest_path, [production_patch], test_patch
                )


if __name__ == "__main__":
    unittest.main()

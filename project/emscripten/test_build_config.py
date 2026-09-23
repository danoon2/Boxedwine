"""Build dependency checks using a fake compiler; no emulator or GPU is run."""

import importlib.util
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock


HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location("build_config", HERE / "update-build-config.py")
config = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(config)


class ConfigurationTests(unittest.TestCase):
    def test_unchanged_settings_preserve_dependency_timestamp(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "config.json"
            self.assertTrue(config.update(path, {"flags": "-O2"}))
            previous = path.stat().st_mtime_ns
            self.assertFalse(config.update(path, {"flags": "-O2"}))
            self.assertEqual(previous, path.stat().st_mtime_ns)
            self.assertTrue(config.update(path, {"flags": "-O3"}))
            self.assertEqual({"flags": "-O3"}, json.loads(path.read_bytes()))

    def test_literal_flags_and_unrelated_environment(self):
        environment = {"BW_BUILD_CC": "cc", "BW_BUILD_CXX": "c++",
            "BW_BUILD_CPPFLAGS": "-D'GLH=<GL/gl.h>'\nquote: ' \" $HOME $(echo literal)",
            "UNRELATED_SECRET": "must not be recorded"}
        with mock.patch.object(config, "compiler_identity", return_value={"version": "1"}):
            value = config.configuration("compile", environment)
        self.assertEqual(environment["BW_BUILD_CPPFLAGS"], value["settings"]["CPPFLAGS"])
        self.assertNotIn("must not be recorded", json.dumps(value))

    def test_compiler_failure_does_not_replace_stamp(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "config.json"
            path.write_bytes(b"previous")
            with mock.patch.object(sys, "argv", ["helper", "compile", str(path)]), \
                    mock.patch.object(config, "compiler_identity", side_effect=ValueError("missing")):
                with self.assertRaises(SystemExit) as error:
                    config.main()
            self.assertEqual(1, error.exception.code)
            self.assertEqual(b"previous", path.read_bytes())

    def test_compiler_environment_is_part_of_both_phase_identities(self):
        environment = {"BW_BUILD_CC": "cc", "BW_BUILD_CXX": "c++"}
        with mock.patch.object(config, "compiler_identity", return_value={"version": "1"}):
            for kind in ("compile", "link"):
                original = config.configuration(kind, environment)
                for key, value in (("EMCC_CFLAGS", "-DFIXTURE=1"), ("SOURCE_DATE_EPOCH", "1768319767")):
                    with self.subTest(kind=kind, key=key):
                        changed = config.configuration(kind, dict(environment, **{key: value}))
                        self.assertNotEqual(original, changed)
                        self.assertEqual(value, changed["environment"][key])
                self.assertEqual(original,
                    config.configuration(kind, dict(environment, UNRELATED_SECRET="not recorded")))


FAKE_COMPILER = r'''
import hashlib
import json
import os
from pathlib import Path
import sys

args = sys.argv[1:]
if args == ['--version']:
    print('fixture compiler ' + os.environ.get('FIXTURE_COMPILER_VERSION', '1'))
    raise SystemExit(0)
output_argument = args[args.index('-o') + 1]
output = Path(output_argument)
output.parent.mkdir(parents=True, exist_ok=True)
kind = 'compile' if '-c' in args else 'link'
if kind == 'compile':
    source = Path(args[args.index('-c') + 1])
    assert source.suffix in ('.cpp', '.c', '.s'), source
    data = source.read_bytes()
    if '-MMD' in args:
        dependencies = [source]
        header = source.with_suffix('.h')
        if header.exists():
            dependencies.append(header)
            data += header.read_bytes()
        output.with_suffix('.d').write_text(output_argument + ': ' + ' '.join(map(str, dependencies)) + '\n')
else:
    data = b''.join(Path(arg).read_bytes() for arg in args if arg.endswith('.o'))
data = hashlib.sha256(data + json.dumps(args).encode()).hexdigest()
output.write_text(data)
if kind == 'link':
    for suffix in ('.js', '.wasm'):
        output.with_suffix(suffix).write_text(data)
record = (json.dumps({'kind': kind, 'args': args}) + '\n').encode()
fd = os.open(os.environ['FIXTURE_CALL_LOG'], os.O_WRONLY | os.O_CREAT | os.O_APPEND, 0o600)
try:
    os.write(fd, record)
finally:
    os.close(fd)
'''


@unittest.skipUnless(os.name == "posix" and shutil.which("make"), "requires POSIX make")
class MakeDependencyTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.project = self.root / "project/emscripten"
        self.project.mkdir(parents=True)
        for name in ("makefile", "update-build-config.py", "boxedwine-asan-pthread.cpp", "webgl-counters.mjs"):
            shutil.copyfile(HERE / name, self.project / name)
        for name in ("shell.html", "boxedwine.css", "boxedwine-shell.js",
                "boxedwine-wasm-jit-module-broker.js", "boxedwine-multithreaded-audio.js",
                "boxedwine-audio-worklet.js"):
            (self.project / name).write_text("fixture\n")
        self.cpp = self.root / "source/probe.cpp"
        self.scheduler = self.root / "source/kernel/kscheduler.cpp"
        self.c = self.root / "lib/helper.c"
        for path in (self.cpp, self.scheduler, self.c):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("fixture source\n")
        self.c.with_suffix(".h").write_text("fixture header\n")
        self.compiler = self.root / "compiler.py"
        self.compiler.write_text(FAKE_COMPILER)
        self.log = self.root / "calls.jsonl"
        self.environment = dict(os.environ, FIXTURE_CALL_LOG=str(self.log))
        self.sources = [self.cpp, self.scheduler]
        (self.root / "source/emulation").mkdir()

    def run_make(self, *settings, mode="release"):
        command = ["make", mode, "make_jobs=3", "PYTHON=" + sys.executable,
            "CC=" + sys.executable + " " + str(self.compiler),
            "CXX=" + sys.executable + " " + str(self.compiler),
            "SRCS=" + " ".join(map(str, self.sources)), "SOFT_SOURCES=" + str(self.c),
            *settings]
        completed = subprocess.run(command, cwd=self.project, env=self.environment,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        self.assertEqual(0, completed.returncode, completed.stdout)
        calls = [json.loads(line) for line in self.log.read_text().splitlines()]
        self.log.write_text("")
        return [call["kind"] for call in calls], completed.stdout

    def initial_build(self, *settings):
        calls, _ = self.run_make(*settings)
        self.assertEqual(3, calls.count("compile"))
        self.assertEqual(1, calls.count("link"))

    def change_source(self, path, target=None):
        # Linux filesystem mtimes can share one clock tick even when writes
        # happen sequentially. Test a strictly newer input, as make requires.
        time.sleep(0.02)
        path.write_text("changed input\n")
        target = target or self.project / "Build/Release/boxedwine.html"
        self.assertGreater(path.stat().st_mtime_ns, target.stat().st_mtime_ns)

    def test_noop_reuses_objects_and_linked_outputs(self):
        self.initial_build()
        self.assertEqual([], self.run_make()[0])

    def test_compiler_flags_recompile_and_link(self):
        self.initial_build()
        calls, _ = self.run_make("GCC_EXTRA_FLAGS=-DFIXTURE=1")
        self.assertEqual(3, calls.count("compile"))
        self.assertEqual(1, calls.count("link"))
        self.assertEqual([], self.run_make("GCC_EXTRA_FLAGS=-DFIXTURE=1")[0])

    def test_link_flags_only_relink(self):
        self.initial_build()
        self.assertEqual(["link"], self.run_make("EXTRA_LD_FLAGS=-sASSERTIONS=1")[0])

    def test_compiler_environment_changes_and_removal_rebuild(self):
        self.initial_build()
        for key, value in (("EMCC_CFLAGS", "-DFIXTURE=1"), ("SOURCE_DATE_EPOCH", "1768319767")):
            with self.subTest(key=key):
                self.environment[key] = value
                calls, _ = self.run_make()
                self.assertEqual(3, calls.count("compile"))
                self.assertEqual(1, calls.count("link"))
                self.assertEqual([], self.run_make()[0])
                del self.environment[key]
                calls, _ = self.run_make()
                self.assertEqual(3, calls.count("compile"))
                self.assertEqual(1, calls.count("link"))
                self.assertEqual([], self.run_make()[0])

    def test_compiler_version_change_invalidates_objects(self):
        self.initial_build()
        self.environment["FIXTURE_COMPILER_VERSION"] = "2"
        calls, _ = self.run_make()
        self.assertEqual(3, calls.count("compile"))
        self.assertEqual(1, calls.count("link"))

    def test_c_header_change_recompiles_affected_object(self):
        self.initial_build()
        self.change_source(self.c.with_suffix(".h"))
        calls, _ = self.run_make()
        self.assertEqual(["compile", "link"], calls)

    def test_removed_source_relinks_without_compiling(self):
        self.initial_build()
        self.sources.remove(self.cpp)
        self.assertEqual(["link"], self.run_make()[0])

    def test_shell_and_javascript_changes_relink(self):
        flags = ("EXTRA_LD_FLAGS=--js-library=boxedwine-multithreaded-audio.js "
            "--pre-js=boxedwine-wasm-jit-module-broker.js")
        self.initial_build(flags)
        for name in ("shell.html", "boxedwine-multithreaded-audio.js", "boxedwine-wasm-jit-module-broker.js",
                "boxedwine-audio-worklet.js"):
            self.change_source(self.project / name)
            self.assertEqual(["link"], self.run_make(flags)[0])

    def test_missing_secondary_outputs_relink(self):
        self.initial_build()
        for suffix in (".wasm", ".js"):
            (self.project / "Build/Release/boxedwine").with_suffix(suffix).unlink()
            self.assertEqual(["link"], self.run_make()[0])
            self.assertEqual([], self.run_make()[0])

    def test_profile_flag_rebuilds_objects(self):
        self.initial_build()
        calls, _ = self.run_make("WASM_PROFILING=1")
        self.assertEqual(3, calls.count("compile"))
        self.assertEqual(1, calls.count("link"))

    def test_gl_counter_toggle_rebuilds_each_mode_and_preserves_caller_flags(self):
        def runtime_methods(build):
            flags = json.loads((build / ".link-config.json").read_text())["settings"]["LDFLAGS"]
            # Emscripten uses the final assignment, including a mode's override.
            values = [arg.partition("=")[2] for arg in shlex.split(flags)
                if arg.removeprefix("-s").startswith("EXPORTED_RUNTIME_METHODS=")]
            value = values[-1]
            return set(json.loads(value) if value.startswith("[") else value.split(","))

        for mode, folder in (("release", "Release"), ("jit", "Jit"),
                ("multiThreaded", "MultiThreaded"), ("multiThreadedJit", "MultiThreadedJit")):
            with self.subTest(mode=mode):
                build = self.project / "Build" / folder
                settings = ("GCC_EXTRA_FLAGS=-DFIXTURE=1",)
                calls, _ = self.run_make(*settings, mode=mode)
                self.assertEqual(3, calls.count("compile"))
                self.assertEqual(1, calls.count("link"))
                self.assertFalse((build / "webgl-counters.mjs").exists())
                original = json.loads((build / ".compile-config.json").read_text())
                original_methods = runtime_methods(build)
                calls, _ = self.run_make(*settings, "WEBGL_COUNTERS=1", mode=mode)
                self.assertEqual(3, calls.count("compile"))
                self.assertEqual(1, calls.count("link"))
                enabled = json.loads((build / ".compile-config.json").read_text())
                self.assertEqual(original_methods | {"ccall"}, runtime_methods(build))
                for key in ("CPPFLAGS", "WASM_EXCEPTION_CPPFLAGS"):
                    self.assertEqual(1, enabled["settings"][key].split().count("-DBOXEDWINE_WEBGL_COUNTERS"))
                    self.assertIn("-DFIXTURE=1", enabled["settings"][key].split())
                self.assertEqual((self.project / "webgl-counters.mjs").read_bytes(),
                    (build / "webgl-counters.mjs").read_bytes())
                self.assertEqual([], self.run_make(*settings, "WEBGL_COUNTERS=1", mode=mode)[0])
                calls, _ = self.run_make(*settings, mode=mode)
                self.assertEqual(3, calls.count("compile"))
                self.assertEqual(1, calls.count("link"))
                self.assertEqual(original, json.loads((build / ".compile-config.json").read_text()))
                self.assertEqual(original_methods, runtime_methods(build))
                self.assertEqual([], self.run_make(*settings, mode=mode)[0])

    def test_gl_report_asset_change_does_not_recompile_or_relink(self):
        self.initial_build("WEBGL_COUNTERS=1")
        helper = self.project / "webgl-counters.mjs"
        copied = self.project / "Build/Release/webgl-counters.mjs"
        self.change_source(helper, copied)
        self.assertEqual([], self.run_make("WEBGL_COUNTERS=1")[0])
        self.assertEqual(helper.read_bytes(), copied.read_bytes())

    def test_mode_caches_are_independent(self):
        self.initial_build()
        calls, _ = self.run_make(mode="jit")
        self.assertEqual(3, calls.count("compile"))
        self.assertEqual(1, calls.count("link"))
        self.assertEqual([], self.run_make()[0])
        self.assertEqual([], self.run_make(mode="jit")[0])

    def test_separate_compile_phase_does_not_link(self):
        calls, _ = self.run_make("BUILD_PHASE=compile")
        self.assertEqual(["compile"] * 3, calls)
        self.assertFalse((self.project / "Build/Release/boxedwine.html").exists())
        self.assertEqual(["link"], self.run_make()[0])
        self.assertEqual([], self.run_make("BUILD_PHASE=compile")[0])
        self.assertEqual([], self.run_make()[0])

    def test_asan_shim_dependencies_are_included(self):
        header = self.project / "boxedwine-asan-pthread.h"
        header.write_text("fixture header\n")
        flags = "OPTIMIZATION_FLAGS=-O2 -fsanitize=address"
        calls, _ = self.run_make(flags, mode="multiThreadedJit")
        self.assertEqual(4, calls.count("compile"))
        self.assertEqual(1, calls.count("link"))
        target = self.project / "Build/MultiThreadedJit/boxedwine.html"
        self.change_source(header, target)
        self.assertEqual(["compile", "link"], self.run_make(flags, mode="multiThreadedJit")[0])


if __name__ == "__main__":
    unittest.main()

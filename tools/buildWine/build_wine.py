#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
from dataclasses import dataclass
import json
import os
import platform
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from typing import Any, Callable, Dict, List, Mapping, Optional, Sequence, Tuple
import urllib.request
from urllib.request import urlretrieve
import zipfile


class BuildError(RuntimeError):
    pass


@dataclass(frozen=True)
class WineTag:
    tag: str
    version: str
    components: Tuple[int, ...]


@dataclass(frozen=True)
class Operation:
    kind: str
    value: str
    reason: str


@dataclass(frozen=True)
class RequiredConfigCheck:
    checking: str
    result: Optional[str]
    result_regex: Optional[str]
    apt: Tuple[str, ...]
    reason: str


@dataclass(frozen=True)
class BaseFilesystemConfig:
    url: str
    filename: str
    size: Optional[int]
    sha256: Optional[str]


def parse_version(version: str) -> Tuple[int, ...]:
    if not re.fullmatch(r"\d+(?:\.\d+)*", version):
        raise BuildError(f"Invalid Wine version: {version}")
    return tuple(int(part) for part in version.split("."))


def parse_wine_tag(tag: str) -> WineTag:
    match = re.fullmatch(r"wine-(\d+(?:\.\d+)*)", tag)
    if not match:
        raise BuildError(f"Expected a Wine tag like wine-11.0, got: {tag}")
    version = match.group(1)
    return WineTag(tag=tag, version=version, components=parse_version(version))


def positive_int(value: str) -> int:
    try:
        parsed = int(value)
    except ValueError:
        raise argparse.ArgumentTypeError("jobs must be a positive integer")
    if parsed <= 0:
        raise argparse.ArgumentTypeError("jobs must be a positive integer")
    return parsed


def _compare_versions(left: Tuple[int, ...], right: Tuple[int, ...]) -> int:
    length = max(len(left), len(right))
    padded_left = left + (0,) * (length - len(left))
    padded_right = right + (0,) * (length - len(right))
    if padded_left < padded_right:
        return -1
    if padded_left > padded_right:
        return 1
    return 0


def version_in_range(version: Tuple[int, ...], bounds: Mapping[str, Any]) -> bool:
    _validate_when_bounds(bounds)
    if "min" in bounds and _compare_versions(version, parse_version(bounds["min"])) < 0:
        return False
    if "max" in bounds and _compare_versions(version, parse_version(bounds["max"])) > 0:
        return False
    return True


SUPPORTED_OPERATIONS = ("apply_patch", "cherry_pick", "revert")
OPERATION_ALLOWED_KEYS = frozenset(SUPPORTED_OPERATIONS + ("when", "reason"))
WHEN_ALLOWED_KEYS = frozenset(("min", "max"))
BUILD_ALLOWED_KEYS = frozenset(("prefix", "configure_args", "cc", "cxx", "cflags", "ld", "targetflags", "ldflags"))
BASE_FILESYSTEM_ALLOWED_KEYS = frozenset(("url", "filename", "size", "sha256"))
CONFIG_CHECK_ALLOWED_KEYS = frozenset(("checking", "result", "result_regex", "apt", "reason"))


def load_config(path: Path) -> Dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as config_file:
            return json.load(config_file)
    except FileNotFoundError as error:
        raise BuildError(f"Could not read config {path}: {error.strerror}") from error
    except OSError as error:
        raise BuildError(f"Could not read config {path}: {error}") from error
    except json.JSONDecodeError as error:
        raise BuildError(f"Could not parse config {path}: {error}") from error


def _unsupported_keys(item: Mapping[str, Any], allowed_keys: frozenset) -> List[str]:
    return sorted(str(key) for key in item if key not in allowed_keys)


def _safe_relative_path(value: str, label: str) -> str:
    if value in {"", ".", ".."} or value.startswith("/") or "\\" in value:
        raise BuildError(f"{label} must be a safe relative path: {value}")
    if any(part in {"", ".", ".."} for part in value.split("/")):
        raise BuildError(f"{label} must be a safe relative path: {value}")
    return value


def _validate_when_bounds(bounds: Mapping[str, Any]) -> None:
    unsupported = _unsupported_keys(bounds, WHEN_ALLOWED_KEYS)
    if unsupported:
        raise BuildError(f"Unsupported when keys: {', '.join(unsupported)}")
    for name in ("min", "max"):
        if name in bounds and not isinstance(bounds[name], str):
            raise BuildError(f"when {name} must be a string: {bounds}")
    if "min" in bounds and "max" in bounds:
        minimum = parse_version(bounds["min"])
        maximum = parse_version(bounds["max"])
        if _compare_versions(minimum, maximum) > 0:
            raise BuildError(f"when min must be <= max: {bounds}")


def operation_from_config(item: Mapping[str, Any]) -> Operation:
    if not isinstance(item, Mapping):
        raise BuildError(f"operation entries must be objects: {item}")
    unsupported = _unsupported_keys(item, OPERATION_ALLOWED_KEYS)
    if unsupported:
        raise BuildError(f"Unsupported operation keys: {', '.join(unsupported)}")
    present = [name for name in SUPPORTED_OPERATIONS if name in item]
    if len(present) != 1:
        raise BuildError(f"Each operation must contain exactly one operation key from {SUPPORTED_OPERATIONS}: {item}")
    kind = present[0]
    value = item[kind]
    if not isinstance(value, str) or not value:
        raise BuildError(f"Operation {kind} must be a non-empty string: {item}")
    if kind == "apply_patch":
        value = _safe_relative_path(value, "patch path")
    reason = item.get("reason", "")
    if reason is not None and not isinstance(reason, str):
        raise BuildError(f"Operation reason must be a string: {item}")
    return Operation(kind=kind, value=value, reason=reason or "")


def select_operations(config: Mapping[str, Any], version: Tuple[int, ...]) -> List[Operation]:
    if not isinstance(config, Mapping):
        raise BuildError(f"Config must be an object: {config}")
    operations = config.get("operations", [])
    if not isinstance(operations, list):
        raise BuildError(f"operations must be a list: {operations}")
    selected: List[Operation] = []
    for item in operations:
        if not isinstance(item, Mapping):
            raise BuildError(f"operation entries must be objects: {item}")
        operation = operation_from_config(item)
        bounds = item.get("when", {})
        if not isinstance(bounds, Mapping):
            raise BuildError(f"Operation when value must be an object: {item}")
        if version_in_range(version, bounds):
            selected.append(operation)
    return selected


def required_config_check_from_config(item: Mapping[str, Any]) -> RequiredConfigCheck:
    if not isinstance(item, Mapping):
        raise BuildError(f"required_config_checks entries must be objects: {item}")
    unsupported = _unsupported_keys(item, CONFIG_CHECK_ALLOWED_KEYS)
    if unsupported:
        raise BuildError(f"Unsupported required_config_checks keys: {', '.join(unsupported)}")
    checking = item.get("checking")
    if not isinstance(checking, str) or not checking.startswith("checking "):
        raise BuildError(f"required config check must contain a checking string: {item}")
    result = item.get("result")
    result_regex = item.get("result_regex")
    if result is not None and not isinstance(result, str):
        raise BuildError(f"required config check result must be a string: {item}")
    if result_regex is not None and not isinstance(result_regex, str):
        raise BuildError(f"required config check result_regex must be a string: {item}")
    if (result is None) == (result_regex is None):
        raise BuildError(f"required config check must contain exactly one of result or result_regex: {item}")
    apt_value = item.get("apt", [])
    if not isinstance(apt_value, list) or not all(isinstance(package, str) and package for package in apt_value):
        raise BuildError(f"required config check apt must be a list of package names: {item}")
    reason = item.get("reason", "")
    if reason is not None and not isinstance(reason, str):
        raise BuildError(f"required config check reason must be a string: {item}")
    if result_regex is not None:
        try:
            re.compile(result_regex)
        except re.error as error:
            raise BuildError(f"required config check result_regex is invalid: {error}") from error
    return RequiredConfigCheck(
        checking=checking,
        result=result,
        result_regex=result_regex,
        apt=tuple(apt_value),
        reason=reason or "",
    )


def required_config_checks(config: Mapping[str, Any]) -> List[RequiredConfigCheck]:
    checks = config.get("required_config_checks", [])
    if not isinstance(checks, list):
        raise BuildError(f"required_config_checks must be a list: {checks}")
    return [required_config_check_from_config(item) for item in checks]


def parse_config_log_results(config_log: Path) -> Dict[str, List[str]]:
    results: Dict[str, List[str]] = {}
    pending: Optional[str] = None
    try:
        lines = config_log.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError as error:
        raise BuildError(f"Could not read Wine config.log {config_log}: {error}") from error
    for line in lines:
        checking = re.match(r"configure:\d+: (checking .*)", line)
        if checking:
            pending = checking.group(1)
            continue
        result = re.match(r"configure:\d+: result: (.*)", line)
        if result and pending is not None:
            results.setdefault(pending, []).append(result.group(1))
            pending = None
    return results


def _config_check_matches(check: RequiredConfigCheck, actual: str) -> bool:
    if check.result is not None:
        return actual == check.result
    assert check.result_regex is not None
    return re.fullmatch(check.result_regex, actual) is not None


def validate_config_log(config: Mapping[str, Any], config_log: Path) -> None:
    checks = required_config_checks(config)
    if not checks:
        return
    results = parse_config_log_results(config_log)
    problems: List[str] = []
    packages: List[str] = []
    for check in checks:
        actual_results = results.get(check.checking, [])
        expected = check.result if check.result is not None else f"/{check.result_regex}/"
        if not actual_results:
            problem = f"- {check.checking}: expected {expected}, but configure did not record this check"
        elif any(_config_check_matches(check, actual) for actual in actual_results):
            continue
        else:
            got = ", ".join(repr(actual) for actual in actual_results)
            problem = f"- {check.checking}: expected {expected}, got {got}"
        if check.apt:
            problem = f"{problem} (apt: {' '.join(check.apt)})"
            packages.extend(check.apt)
        if check.reason:
            problem = f"{problem} [{check.reason}]"
        problems.append(problem)
    if problems:
        message = ["Wine configure did not enable required Boxedwine build functionality:", *problems]
        if packages:
            deduped_packages = sorted(set(packages))
            message.append(f"Install missing dependencies with: sudo apt install {' '.join(deduped_packages)}")
            message.append("If apt cannot find :i386 packages, run: sudo dpkg --add-architecture i386 && sudo apt update")
        raise BuildError("\n".join(message))


MINGW_STRIP = "i686-w64-mingw32-strip"
BASE_REQUIRED_COMMANDS = (
    "git",
    "make",
    "gcc",
    "g++",
    "pkg-config",
    "flex",
    "bison",
    "fontforge",
    "gettext",
    "sdl2-config",
)
MINGW_REQUIRED_COMMANDS = (
    MINGW_STRIP,
)
REQUIRED_COMMANDS = BASE_REQUIRED_COMMANDS + MINGW_REQUIRED_COMMANDS
BASE_APT_PACKAGES = (
    "build-essential",
    "git",
    "gcc-multilib",
    "g++-multilib",
    "libc6-dev-i386",
    "libx11-dev:i386",
    "libxext-dev:i386",
    "libxrender-dev:i386",
    "libxrandr-dev:i386",
    "libxi-dev:i386",
    "libxcursor-dev:i386",
    "libxinerama-dev:i386",
    "libxxf86vm-dev:i386",
    "libfreetype-dev:i386",
    "libgl-dev:i386",
    "libegl-dev:i386",
    "libvulkan-dev:i386",
    "pkg-config",
    "flex",
    "bison",
    "fontforge",
    "gettext",
    "libsdl2-dev",
    "libssl-dev",
    "libminizip-dev",
    "libcurl4-openssl-dev",
    "zlib1g-dev",
    "libgl-dev",
)
MINGW_APT_PACKAGES = (
    "binutils-mingw-w64-i686",
)
APT_PACKAGES = BASE_APT_PACKAGES + MINGW_APT_PACKAGES
DEFAULT_I386_PKG_CONFIG_LIBDIR = "/usr/lib/i386-linux-gnu/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig"


def build_uses_mingw(config: Mapping[str, Any]) -> bool:
    return "--without-mingw" not in configure_command(config)


def required_commands(mingw_required: bool) -> Tuple[str, ...]:
    if mingw_required:
        return REQUIRED_COMMANDS
    return BASE_REQUIRED_COMMANDS


def apt_packages(mingw_required: bool) -> Tuple[str, ...]:
    if mingw_required:
        return APT_PACKAGES
    return BASE_APT_PACKAGES


def read_os_release(path: Path = Path("/etc/os-release")) -> Dict[str, str]:
    values: Dict[str, str] = {}
    if not path.exists():
        return values
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" not in line or line.startswith("#"):
            continue
        key, value = line.split("=", 1)
        values[key] = value.strip().strip("\"'")
    return values


def detect_wsl() -> bool:
    for path in (Path("/proc/sys/kernel/osrelease"), Path("/proc/version")):
        try:
            text = path.read_text(encoding="utf-8", errors="ignore").lower()
        except OSError:
            continue
        if "microsoft" in text or "wsl" in text:
            return True
    return False


def _gcc_m32_compile_link(source_name: str, source_text: str, extra_args: Sequence[str]) -> bool:
    with tempfile.TemporaryDirectory() as temp_dir:
        source = Path(temp_dir) / source_name
        output = Path(temp_dir) / "test"
        source.write_text(source_text, encoding="utf-8")
        try:
            result = subprocess.run(
                ["gcc", "-m32", str(source), *extra_args, "-o", str(output)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
        except OSError:
            return False
        return result.returncode == 0


def default_gcc_m32_works() -> bool:
    return _gcc_m32_compile_link("test.c", "int main(void) { return 0; }\n", [])


def default_gxx_m32_works() -> bool:
    with tempfile.TemporaryDirectory() as temp_dir:
        source = Path(temp_dir) / "test.cc"
        output = Path(temp_dir) / "test"
        source.write_text("int main() { return 0; }\n", encoding="utf-8")
        try:
            result = subprocess.run(
                ["g++", "-m32", str(source), "-o", str(output)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
        except OSError:
            return False
        return result.returncode == 0


def default_x11_m32_works() -> bool:
    return _gcc_m32_compile_link(
        "test-x11.c",
        "#include <X11/Xlib.h>\nint main(void) { Display *display = 0; (void)display; return 0; }\n",
        ["-lX11"],
    )


def default_freetype_m32_works() -> bool:
    env = dict(os.environ)
    env.setdefault("PKG_CONFIG_LIBDIR", DEFAULT_I386_PKG_CONFIG_LIBDIR)
    try:
        result = subprocess.run(
            ["pkg-config", "--cflags", "--libs", "freetype2"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
            env=env,
        )
    except OSError:
        return False
    if result.returncode != 0:
        return False
    return _gcc_m32_compile_link(
        "test-freetype.c",
        "#include <ft2build.h>\n#include FT_FREETYPE_H\nint main(void) { FT_Library library; return FT_Init_FreeType(&library); }\n",
        result.stdout.split(),
    )


def default_opengl_m32_works() -> bool:
    return _gcc_m32_compile_link(
        "test-opengl.c",
        "#include <GL/gl.h>\nint main(void) { const GLubyte *name = glGetString(GL_RENDERER); return name == 0; }\n",
        ["-lGL"],
    )


def default_egl_m32_works() -> bool:
    return _gcc_m32_compile_link(
        "test-egl.c",
        "#include <EGL/egl.h>\nint main(void) { EGLDisplay display = EGL_NO_DISPLAY; return display != EGL_NO_DISPLAY; }\n",
        ["-lEGL"],
    )


def default_vulkan_m32_works() -> bool:
    return _gcc_m32_compile_link(
        "test-vulkan.c",
        "#include <vulkan/vulkan.h>\nint main(void) { uint32_t count = 0; return vkEnumerateInstanceExtensionProperties(0, &count, 0); }\n",
        ["-lvulkan"],
    )


def wsl_windows_mount_checkout(path: Path) -> Optional[Path]:
    resolved = path.resolve(strict=False)
    parts = resolved.parts
    if len(parts) < 3 or parts[0] != "/" or parts[1] != "mnt" or not re.fullmatch(r"[A-Za-z]", parts[2]):
        return None
    if len(parts) >= 4:
        return Path("/", "mnt", parts[2], parts[3])
    return Path("/", "mnt", parts[2])


class EnvironmentChecker:
    def __init__(
        self,
        os_release: Optional[Mapping[str, str]] = None,
        is_linux: Optional[bool] = None,
        is_wsl: Optional[bool] = None,
        work_dir: Optional[Path] = None,
        command_exists: Optional[Callable[[str], bool]] = None,
        gcc_m32_works: Optional[Callable[[], bool]] = None,
        gxx_m32_works: Optional[Callable[[], bool]] = None,
        x11_m32_works: Optional[Callable[[], bool]] = None,
        freetype_m32_works: Optional[Callable[[], bool]] = None,
        opengl_m32_works: Optional[Callable[[], bool]] = None,
        egl_m32_works: Optional[Callable[[], bool]] = None,
        vulkan_m32_works: Optional[Callable[[], bool]] = None,
        mingw_required: bool = True,
    ) -> None:
        self.os_release = dict(os_release) if os_release is not None else read_os_release()
        self.is_linux = platform.system() == "Linux" if is_linux is None else is_linux
        self.is_wsl = detect_wsl() if is_wsl is None else is_wsl
        self.work_dir = Path(__file__).resolve().parent if work_dir is None else Path(work_dir)
        self.command_exists = command_exists or (lambda name: shutil.which(name) is not None)
        self.gcc_m32_works = gcc_m32_works or default_gcc_m32_works
        self.gxx_m32_works = gxx_m32_works or default_gxx_m32_works
        self.x11_m32_works = x11_m32_works or default_x11_m32_works
        self.freetype_m32_works = freetype_m32_works or default_freetype_m32_works
        self.opengl_m32_works = opengl_m32_works or default_opengl_m32_works
        self.egl_m32_works = egl_m32_works or default_egl_m32_works
        self.vulkan_m32_works = vulkan_m32_works or default_vulkan_m32_works
        self.mingw_required = mingw_required

    def check(self) -> None:
        problems: List[str] = []
        distro_id = self.os_release.get("ID", "").lower()
        id_like = self.os_release.get("ID_LIKE", "").lower().split()
        debian_family = distro_id in {"debian", "ubuntu"} or any(name in {"debian", "ubuntu"} for name in id_like)
        support_message = "This build tool currently supports Debian or Ubuntu Linux/WSL only."

        if not self.is_linux or not debian_family:
            raise BuildError(support_message)

        if self.is_wsl:
            windows_mount = wsl_windows_mount_checkout(self.work_dir)
            if windows_mount is not None:
                raise BuildError(
                    "This checkout is on a Windows-mounted WSL path, which can break 32-bit Wine font tooling. "
                    f"Use a WSL-native Linux filesystem checkout instead, such as /home/$USER/Boxedwine, rather than {windows_mount}."
                )

        missing = sorted(name for name in required_commands(self.mingw_required) if not self.command_exists(name))
        if missing:
            problems.append(f"Missing required commands: {', '.join(missing)}")

        gcc_m32_ok = "gcc" not in missing and self.gcc_m32_works()
        if "gcc" not in missing and not gcc_m32_ok:
            problems.append("gcc -m32 could not compile and link a tiny program.")

        if "g++" not in missing and not self.gxx_m32_works():
            problems.append("g++ -m32 could not compile and link a tiny program.")

        if gcc_m32_ok and not self.x11_m32_works():
            problems.append("gcc -m32 could not compile and link a tiny X11 program.")

        if gcc_m32_ok and not self.freetype_m32_works():
            problems.append("gcc -m32 could not compile and link a tiny FreeType program.")

        if gcc_m32_ok and not self.opengl_m32_works():
            problems.append("gcc -m32 could not compile and link a tiny OpenGL program.")

        if gcc_m32_ok and not self.egl_m32_works():
            problems.append("gcc -m32 could not compile and link a tiny EGL program.")

        if gcc_m32_ok and not self.vulkan_m32_works():
            problems.append("gcc -m32 could not compile and link a tiny Vulkan program.")

        if problems:
            packages = " ".join(apt_packages(self.mingw_required))
            problems.append(f"Install build dependencies with: sudo apt install {packages}")
            problems.append("If apt cannot find :i386 packages, run: sudo dpkg --add-architecture i386 && sudo apt update")
            raise BuildError("\n".join(problems))


def command_text(args: Sequence[object]) -> str:
    return shlex.join(str(arg) for arg in args)


class CommandRunner:
    def __init__(self, dry_run: bool = False) -> None:
        self.dry_run = dry_run

    def run(self, args: Sequence[object], cwd: Optional[Path] = None, env: Optional[Mapping[str, str]] = None) -> None:
        print(f"+ {command_text(args)}")
        if self.dry_run:
            return
        subprocess.run([str(arg) for arg in args], cwd=cwd, env=dict(env) if env is not None else None, check=True)


def _build_config(config: Mapping[str, Any]) -> Mapping[str, Any]:
    build = config.get("build", {})
    if not isinstance(build, Mapping):
        raise BuildError(f"build config must be an object: {build}")
    unsupported = _unsupported_keys(build, BUILD_ALLOWED_KEYS)
    if unsupported:
        raise BuildError(f"Unsupported build keys: {', '.join(unsupported)}")
    return build


def _mapping_config(config: Mapping[str, Any], key: str) -> Mapping[str, Any]:
    if not isinstance(config, Mapping):
        raise BuildError(f"Config must be an object: {config}")
    value = config.get(key)
    if not isinstance(value, Mapping):
        raise BuildError(f"{key} config must be an object")
    return value


def _required_string(config: Mapping[str, Any], key: str, label: str) -> str:
    value = config.get(key)
    if not isinstance(value, str) or not value:
        raise BuildError(f"{label} must be a non-empty string")
    return value


def _optional_positive_int(config: Mapping[str, Any], key: str, label: str) -> Optional[int]:
    value = config.get(key)
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
        raise BuildError(f"{label} must be a positive integer")
    return value


def _optional_sha256(config: Mapping[str, Any], key: str, label: str) -> Optional[str]:
    value = config.get(key)
    if value is None:
        return None
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-fA-F]{64}", value):
        raise BuildError(f"{label} must be a 64-character SHA-256 hex digest")
    return value.lower()


def _safe_relative_name(value: str, label: str, name_kind: str) -> str:
    path = Path(value)
    if value in {".", ".."} or path.is_absolute() or path.name != value or "/" in value or "\\" in value:
        raise BuildError(f"{label} must be a safe relative {name_kind}")
    return value


def _base_filesystem_values(config: Mapping[str, Any]) -> BaseFilesystemConfig:
    base = _mapping_config(config, "base_filesystem")
    unsupported = _unsupported_keys(base, BASE_FILESYSTEM_ALLOWED_KEYS)
    if unsupported:
        raise BuildError(f"Unsupported base_filesystem keys: {', '.join(unsupported)}")
    url = _required_string(base, "url", "base url")
    filename = _safe_relative_name(
        _required_string(base, "filename", "base filename"),
        "base filename",
        "filename",
    )
    size = _optional_positive_int(base, "size", "base size")
    sha256 = _optional_sha256(base, "sha256", "base sha256")
    return BaseFilesystemConfig(url=url, filename=filename, size=size, sha256=sha256)


def _wine_values(config: Mapping[str, Any]) -> Tuple[str, str]:
    wine_config = _mapping_config(config, "wine")
    repo = _required_string(wine_config, "repo", "wine repo")
    source_dir = _safe_relative_name(
        _required_string(wine_config, "source_dir", "wine source_dir"),
        "wine source_dir",
        "directory name",
    )
    return repo, source_dir


def _validate_destructive_path_config(config: Mapping[str, Any]) -> None:
    _base_filesystem_values(config)
    _wine_values(config)


def validate_config(config: Mapping[str, Any]) -> None:
    if not isinstance(config, Mapping):
        raise BuildError(f"Config must be an object: {config}")
    _base_filesystem_values(config)
    _wine_values(config)
    configure_command(config)
    configure_environment(config, {})
    required_config_checks(config)
    operations = config.get("operations", [])
    if not isinstance(operations, list):
        raise BuildError(f"operations must be a list: {operations}")
    for item in operations:
        if not isinstance(item, Mapping):
            raise BuildError(f"operation entries must be objects: {item}")
        operation_from_config(item)
        bounds = item.get("when", {})
        if not isinstance(bounds, Mapping):
            raise BuildError(f"Operation when value must be an object: {item}")
        _validate_when_bounds(bounds)


def validate_operation_files(operations: Sequence[Operation], patches_dir: Path) -> None:
    for operation in operations:
        if operation.kind != "apply_patch":
            continue
        patch_file = patches_dir / operation.value
        if not patch_file.is_file():
            raise BuildError(f"Missing patch file: {patch_file}")


def build_requires_oss_headers(config: Mapping[str, Any]) -> bool:
    build = _build_config(config)
    configure_args = build.get("configure_args", [])
    if not isinstance(configure_args, list):
        return False
    return "--with-oss" in configure_args


def validate_oss_header(config: Mapping[str, Any], work_dir: Path) -> None:
    if not build_requires_oss_headers(config):
        return
    source_header = work_dir / "oss" / "soundcard.h"
    if not source_header.is_file():
        raise BuildError(f"Missing OSS soundcard.h: {source_header}")


def clean_patch_root(patch_root: Path) -> None:
    resolved_root = patch_root.resolve(strict=False)
    if resolved_root.name != "tmp_patches":
        raise BuildError(f"Refusing to clean patch path: {resolved_root}")
    if patch_root.exists():
        shutil.rmtree(patch_root)


def clean_oss_header_root(oss_root: Path) -> None:
    resolved_root = oss_root.resolve(strict=False)
    if resolved_root.name != "tmp_oss":
        raise BuildError(f"Refusing to clean OSS header path: {resolved_root}")
    if oss_root.exists():
        shutil.rmtree(oss_root)


def prepare_oss_headers(source_header: Path, oss_root: Path) -> Path:
    clean_oss_header_root(oss_root)
    if not source_header.is_file():
        raise BuildError(f"Missing OSS soundcard.h: {source_header}")
    target = oss_root / "include" / "sys" / "soundcard.h"
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source_header, target)
    return oss_root


def _normalize_patch_line_endings(data: bytes) -> bytes:
    return data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")


def prepare_patch_files(operations: Sequence[Operation], patches_dir: Path, patch_root: Path) -> Dict[str, Path]:
    normalized: Dict[str, Path] = {}
    for operation in operations:
        if operation.kind != "apply_patch":
            continue
        patch_file = patches_dir / operation.value
        data = patch_file.read_bytes()
        if b"\r" not in data:
            continue
        target = patch_root / operation.value
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(_normalize_patch_line_endings(data))
        normalized[operation.value] = target
        print(f"Normalized CRLF patch line endings: {patch_file} -> {target}")
    return normalized


def configure_environment(config: Mapping[str, Any], base_env: Optional[Mapping[str, str]] = None) -> Dict[str, str]:
    env = dict(os.environ if base_env is None else base_env)
    env.setdefault("PKG_CONFIG_LIBDIR", DEFAULT_I386_PKG_CONFIG_LIBDIR)
    build = _build_config(config)
    if "cc" in build:
        cc = build["cc"]
        if not isinstance(cc, str):
            raise BuildError(f"cc must be a string: {cc}")
        env["CC"] = cc
    if "cxx" in build:
        cxx = build["cxx"]
        if not isinstance(cxx, str):
            raise BuildError(f"cxx must be a string: {cxx}")
        env["CXX"] = cxx
    if "cflags" in build:
        cflags = build["cflags"]
        if not isinstance(cflags, str):
            raise BuildError(f"cflags must be a string: {cflags}")
        env["CFLAGS"] = cflags
    if "ld" in build:
        ld = build["ld"]
        if not isinstance(ld, str):
            raise BuildError(f"ld must be a string: {ld}")
        env["LD"] = ld
    if "targetflags" in build:
        targetflags = build["targetflags"]
        if not isinstance(targetflags, str):
            raise BuildError(f"targetflags must be a string: {targetflags}")
        env["TARGETFLAGS"] = targetflags
    if "ldflags" in build:
        ldflags = build["ldflags"]
        if not isinstance(ldflags, str):
            raise BuildError(f"ldflags must be a string: {ldflags}")
        env["LDFLAGS"] = ldflags
    return env


def configure_command(config: Mapping[str, Any]) -> List[str]:
    build = _build_config(config)
    configure_args = build.get("configure_args", [])
    if not isinstance(configure_args, list) or not all(isinstance(arg, str) for arg in configure_args):
        raise BuildError(f"configure_args must be a list of strings: {configure_args}")
    prefix = build.get("prefix")
    if prefix is not None:
        if not isinstance(prefix, str) or not prefix:
            raise BuildError(f"build prefix must be a non-empty string: {prefix}")
        expected_prefix_arg = f"--prefix={prefix}"
        prefix_args = [arg for arg in configure_args if arg.startswith("--prefix=")]
        if prefix_args != [expected_prefix_arg]:
            raise BuildError("build prefix must match configure_args")
    return ["./configure", *configure_args]


def operation_command(
    operation: Operation,
    patches_dir: Path,
    patch_overrides: Optional[Mapping[str, Path]] = None,
) -> List[str]:
    if operation.kind == "apply_patch":
        patch_file = (patch_overrides or {}).get(operation.value, patches_dir / operation.value)
        return ["git", "apply", str(patch_file)]
    if operation.kind == "cherry_pick":
        return ["git", "cherry-pick", operation.value, "--no-commit"]
    if operation.kind == "revert":
        return ["git", "revert", "-n", operation.value]
    raise BuildError(f"Unsupported operation kind: {operation.kind}")


def prepare_staging_root(stage: Path) -> None:
    resolved_stage = stage.resolve(strict=False)
    if resolved_stage.name != "tmp_install":
        raise BuildError(f"Refusing to clean staging path: {resolved_stage}")
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)


def prepare_wineboot_root(root: Path) -> None:
    resolved_root = root.resolve(strict=False)
    if resolved_root.name != "tmp_root":
        raise BuildError(f"Refusing to clean wineboot root path: {resolved_root}")
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)


def _require_file(path: Path) -> None:
    if not path.is_file():
        raise BuildError(f"Missing required Wine file: {path}")


def write_metadata(stage: Path, tag: WineTag, build_lines: Sequence[str]) -> None:
    (stage / "home" / "username").mkdir(parents=True, exist_ok=True)
    (stage / "wineVersion.txt").write_text(tag.version, encoding="utf-8")
    (stage / "name.txt").write_text(f"Wine {tag.version}", encoding="utf-8")
    build_text = "".join(line if line.endswith("\n") else f"{line}\n" for line in build_lines)
    (stage / "build.txt").write_text(build_text, encoding="utf-8")


def prune_packaged_wine(stage: Path) -> None:
    modern_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-unix"
    old_lib_dir = stage / "opt" / "wine" / "lib"
    old_wine_dir = old_lib_dir / "wine"

    if modern_dir.is_dir():
        _require_file(modern_dir / "wine")
        libwine = modern_dir / "libwine.so.1"
        (modern_dir / "winemenubuilder.exe.so").unlink(missing_ok=True)
        if libwine.exists() or libwine.is_symlink():
            libwine.unlink()
            (modern_dir / "libwine.so.1.link").write_text("libwine.so.1.0", encoding="utf-8")
        return

    if not old_wine_dir.is_dir():
        raise BuildError(f"Missing required Wine file: {old_wine_dir}")

    winemenubuilder = old_wine_dir / "winemenubuilder.exe.so"
    libwine = old_lib_dir / "libwine.so.1"
    _require_file(winemenubuilder)
    _require_file(libwine)

    winemenubuilder.unlink()
    (old_lib_dir / "libwine.so").unlink(missing_ok=True)
    libwine.unlink()
    (old_lib_dir / "libwine.so.link").write_text("libwine.so.1.0", encoding="utf-8")
    (old_lib_dir / "libwine.so.1.link").write_text("libwine.so.1.0", encoding="utf-8")


def strip_packaged_pe_binaries(stage: Path, runner: CommandRunner) -> List[List[str]]:
    pe_dir = stage / "opt" / "wine" / "lib" / "wine" / "i386-windows"
    if not pe_dir.is_dir():
        return []

    binaries = [
        path.relative_to(stage).as_posix()
        for path in sorted(pe_dir.iterdir())
        if path.is_file() and path.suffix.lower() in {".dll", ".exe"}
    ]
    if not binaries:
        return []

    command = [MINGW_STRIP, "-s", *binaries]
    runner.run(command, cwd=stage)
    return [command]


def iter_stage_entries(stage: Path):
    for path in sorted(stage.rglob("*")):
        relative = path.relative_to(stage).as_posix()
        if path.is_dir():
            yield path, f"{relative}/"
        else:
            yield path, relative


def _path_is_within(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
    except ValueError:
        return False
    return True


REPLACEABLE_BASE_ENTRIES = {"name.txt", "wineVersion.txt", "build.txt"}
PRESERVED_BASE_ENTRIES = {"changes.txt", "version.txt"}


def remove_preserved_base_entries_from_stage(stage: Path) -> None:
    for name in PRESERVED_BASE_ENTRIES:
        path = stage / name
        if path.is_dir() and not path.is_symlink():
            shutil.rmtree(path)
        else:
            path.unlink(missing_ok=True)


def package_zip(base_zip: Path, stage: Path, output_zip: Path) -> None:
    resolved_base = base_zip.resolve(strict=False)
    resolved_stage = stage.resolve(strict=False)
    resolved_output = output_zip.resolve(strict=False)

    if resolved_base == resolved_output:
        raise BuildError("base and output zip must be different")
    if _path_is_within(resolved_output, resolved_stage):
        raise BuildError("output zip must not be inside staging")

    temp_zip: Optional[Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            prefix=f".{output_zip.name}.",
            suffix=".tmp",
            dir=output_zip.parent,
            delete=False,
        ) as temp_file:
            temp_zip = Path(temp_file.name)
        remove_preserved_base_entries_from_stage(stage)
        stage_entries = list(iter_stage_entries(stage))
        replace_names = {arcname for path, arcname in stage_entries if not path.is_dir() and arcname in REPLACEABLE_BASE_ENTRIES}
        with zipfile.ZipFile(base_zip, "r") as base, zipfile.ZipFile(temp_zip, "w", compression=zipfile.ZIP_DEFLATED) as zf:
            existing_names = set()
            for info in base.infolist():
                if info.filename in replace_names:
                    continue
                zf.writestr(info, base.read(info))
                existing_names.add(info.filename)
            for path, arcname in stage_entries:
                if arcname in existing_names:
                    if path.is_dir():
                        continue
                    raise BuildError(f"Duplicate zip entry: {arcname}")
                if path.is_dir():
                    info = zipfile.ZipInfo(arcname)
                    info.compress_type = zipfile.ZIP_DEFLATED
                    zf.writestr(info, b"")
                else:
                    zf.write(path, arcname)
                existing_names.add(arcname)
        temp_zip.replace(output_zip)
    except Exception:
        if temp_zip is not None and temp_zip.exists():
            temp_zip.unlink()
        raise


def find_boxedwine_linux_dir(work_dir: Path) -> Path:
    candidates = [work_dir / "project" / "linux"]
    parents = list(work_dir.parents)
    if len(parents) >= 2:
        candidates.append(parents[1] / "project" / "linux")

    for candidate in candidates:
        if candidate.is_dir():
            return candidate
    raise BuildError(f"Missing Boxedwine Linux project directory: {candidates[-1]}")


def merge_directory_into_zip(zip_path: Path, source_dir: Path, archive_root: str) -> None:
    if not source_dir.is_dir():
        raise BuildError(f"Missing directory to merge into zip: {source_dir}")

    archive_root = archive_root.strip("/")
    if not archive_root:
        raise BuildError("archive root must not be empty")
    archive_prefix = f"{archive_root}/"
    entries = []
    for path in sorted(source_dir.rglob("*")):
        relative = path.relative_to(source_dir).as_posix()
        arcname = f"{archive_prefix}{relative}"
        if path.is_dir():
            arcname = f"{arcname}/"
        entries.append((path, arcname))

    temp_zip: Optional[Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            prefix=f".{zip_path.name}.",
            suffix=".tmp",
            dir=zip_path.parent,
            delete=False,
        ) as temp_file:
            temp_zip = Path(temp_file.name)
        with zipfile.ZipFile(zip_path, "r") as existing, zipfile.ZipFile(temp_zip, "w", compression=zipfile.ZIP_DEFLATED) as zf:
            existing_names = set()
            for info in existing.infolist():
                if info.filename == archive_root or info.filename.startswith(archive_prefix):
                    continue
                zf.writestr(info, existing.read(info))
                existing_names.add(info.filename)

            root_info = zipfile.ZipInfo(archive_prefix)
            root_info.compress_type = zipfile.ZIP_DEFLATED
            zf.writestr(root_info, b"")
            existing_names.add(archive_prefix)
            for path, arcname in entries:
                if arcname in existing_names:
                    continue
                if path.is_dir():
                    info = zipfile.ZipInfo(arcname)
                    info.compress_type = zipfile.ZIP_DEFLATED
                    zf.writestr(info, b"")
                else:
                    zf.write(path, arcname)
                existing_names.add(arcname)
        temp_zip.replace(zip_path)
    except Exception:
        if temp_zip is not None and temp_zip.exists():
            temp_zip.unlink()
        raise


def remove_zip_prefix(zip_path: Path, archive_root: str) -> None:
    archive_root = archive_root.strip("/")
    archive_prefix = f"{archive_root}/"
    temp_zip: Optional[Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            prefix=f".{zip_path.name}.",
            suffix=".tmp",
            dir=zip_path.parent,
            delete=False,
        ) as temp_file:
            temp_zip = Path(temp_file.name)
        with zipfile.ZipFile(zip_path, "r") as existing, zipfile.ZipFile(temp_zip, "w", compression=zipfile.ZIP_DEFLATED) as zf:
            removed = False
            for info in existing.infolist():
                if info.filename == archive_root or info.filename.startswith(archive_prefix):
                    removed = True
                    continue
                zf.writestr(info, existing.read(info))
        if removed:
            temp_zip.replace(zip_path)
        else:
            temp_zip.unlink()
    except Exception:
        if temp_zip is not None and temp_zip.exists():
            temp_zip.unlink()
        raise


WINDOW_MANAGER_REGISTRY_GUEST_PATH = "/home/username/boxedwine-window-manager.reg"
WINDOW_MANAGER_REGISTRY_TEXT = (
    "REGEDIT4\r\n"
    "\r\n"
    "[HKEY_CURRENT_USER\\Software\\Wine\\X11 Driver]\r\n"
    '"Decorated"="N"\r\n'
    '"Managed"="N"\r\n'
)


def write_window_manager_registry_file(tmp_root: Path) -> Path:
    reg_file = tmp_root / WINDOW_MANAGER_REGISTRY_GUEST_PATH.lstrip("/")
    reg_file.parent.mkdir(parents=True, exist_ok=True)
    reg_file.write_text(WINDOW_MANAGER_REGISTRY_TEXT, encoding="ascii")
    return reg_file


def verify_window_manager_registry(wine_home: Path) -> None:
    user_reg = wine_home / "user.reg"
    if not user_reg.is_file():
        raise BuildError(f"wineboot did not create Wine user registry: {user_reg}")
    text = user_reg.read_text(encoding="utf-8", errors="replace")
    if '"Decorated"="N"' not in text or '"Managed"="N"' not in text:
        raise BuildError("Wine X11 window manager decoration/control registry values were not disabled")


def apply_window_manager_registry(wine_home: Path) -> None:
    user_reg = wine_home / "user.reg"
    if not user_reg.is_file():
        raise BuildError(f"wineboot did not create Wine user registry: {user_reg}")

    section_pattern = re.compile(r"^\[Software\\\\Wine\\\\X11 Driver\](?:\s|$)")
    section_header = "[Software\\\\Wine\\\\X11 Driver]"
    desired_values = {
        '"Decorated"': '"Decorated"="N"',
        '"Managed"': '"Managed"="N"',
    }
    lines = user_reg.read_text(encoding="utf-8", errors="replace").splitlines()
    start = next((index for index, line in enumerate(lines) if section_pattern.match(line)), None)

    if start is None:
        if lines and lines[-1] != "":
            lines.append("")
        lines.extend([section_header, desired_values['"Decorated"'], desired_values['"Managed"']])
        user_reg.write_text("\n".join(lines) + "\n", encoding="utf-8")
        verify_window_manager_registry(wine_home)
        return

    end = len(lines)
    for index in range(start + 1, len(lines)):
        if lines[index].startswith("["):
            end = index
            break

    seen = set()
    for index in range(start + 1, end):
        for value_name, replacement in desired_values.items():
            if lines[index].startswith(f"{value_name}="):
                lines[index] = replacement
                seen.add(value_name)

    insert_at = start + 1
    while insert_at < end and lines[insert_at].startswith("#"):
        insert_at += 1
    missing = [replacement for value_name, replacement in desired_values.items() if value_name not in seen]
    if missing:
        lines[insert_at:insert_at] = missing

    user_reg.write_text("\n".join(lines) + "\n", encoding="utf-8")
    verify_window_manager_registry(wine_home)


def initialize_wine_home(work_dir: Path, output_zip: Path, runner: CommandRunner) -> List[List[str]]:
    tmp_root = work_dir / "tmp_root"
    prepare_wineboot_root(tmp_root)
    linux_dir = find_boxedwine_linux_dir(work_dir)
    remove_zip_prefix(output_zip, "home/username/.wine")

    build_command = ["make", "release"]
    runner.run(build_command, cwd=linux_dir)

    boxedwine = linux_dir / "Build" / "Release" / "boxedwine"
    if not boxedwine.is_file():
        raise BuildError(f"Boxedwine build did not create: {boxedwine}")

    wineboot_command = [
        str(boxedwine),
        "-root",
        str(tmp_root),
        "-zip",
        str(output_zip),
        "-env",
        "WINEDLLOVERRIDES=mscoree,mshtml=",
        "/opt/wine/bin/wineboot",
        "-u",
    ]
    wine_home = tmp_root / "home" / "username" / ".wine"
    try:
        runner.run(wineboot_command)
    except subprocess.CalledProcessError:
        if not (wine_home / "system.reg").is_file():
            raise
    if not (wine_home / "system.reg").is_file():
        raise BuildError(f"wineboot did not create Wine home: {wine_home}")
    write_window_manager_registry_file(tmp_root)
    apply_window_manager_registry(wine_home)
    merge_directory_into_zip(output_zip, wine_home, "home/username/.wine")
    return [build_command, wineboot_command]


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def remote_content_length(url: str) -> Optional[int]:
    try:
        request = urllib.request.Request(url, method="HEAD")
        with urllib.request.urlopen(request) as response:
            value = response.headers.get("Content-Length")
    except Exception:
        return None
    if value is None:
        return None
    try:
        return int(value)
    except ValueError:
        return None


def base_filesystem_mismatch_reason(base: BaseFilesystemConfig, path: Path, remote_size: Optional[int] = None) -> Optional[str]:
    actual_size = path.stat().st_size
    if base.size is not None and actual_size != base.size:
        return f"expected {base.size} bytes, found {actual_size} bytes"
    if remote_size is not None and actual_size != remote_size:
        return f"remote is {remote_size} bytes, local is {actual_size} bytes"
    if base.sha256 is not None:
        actual_sha256 = file_sha256(path)
        if actual_sha256 != base.sha256:
            return f"expected sha256 {base.sha256}, found {actual_sha256}"
    return None


def ensure_base_filesystem(config: Mapping[str, Any], work_dir: Path) -> Path:
    base = _base_filesystem_values(config)
    path = work_dir / base.filename
    remote_size: Optional[int] = None
    if path.exists():
        if not zipfile.is_zipfile(path):
            raise BuildError(f"Base filesystem is not a valid zip: {path}")
        reason = base_filesystem_mismatch_reason(base, path)
        if reason is None and base.size is None and base.sha256 is None:
            remote_size = remote_content_length(base.url)
            reason = base_filesystem_mismatch_reason(base, path, remote_size)
        if reason is None:
            return path
        print(f"Cached base filesystem is stale ({reason}); redownloading {path}")

    temp_path: Optional[Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            prefix=f".{base.filename}.",
            suffix=".tmp",
            dir=work_dir,
            delete=False,
        ) as temp_file:
            temp_path = Path(temp_file.name)
        print(f"Downloading {base.url} -> {path}")
        urlretrieve(base.url, temp_path)
        if not zipfile.is_zipfile(temp_path):
            raise BuildError(f"Downloaded base filesystem is not a valid zip: {path}")
        reason = base_filesystem_mismatch_reason(base, temp_path, remote_size)
        if reason is not None:
            raise BuildError(f"Downloaded base filesystem does not match expected metadata: {reason}")
        temp_path.replace(path)
    except Exception:
        if temp_path is not None and temp_path.exists():
            temp_path.unlink()
        raise
    return path


def ensure_wine_source(config: Mapping[str, Any], work_dir: Path, runner: CommandRunner) -> Path:
    repo, source_dir_name = _wine_values(config)
    source_dir = work_dir / source_dir_name
    if source_dir.exists():
        runner.run(["git", "fetch", "--tags", "--force"], cwd=source_dir)
        runner.run(["git", "reset", "--hard"], cwd=source_dir)
        runner.run(["git", "clean", "-d", "-f", "-x"], cwd=source_dir)
    else:
        runner.run(["git", "clone", repo, str(source_dir)], cwd=work_dir)
    return source_dir


def regenerate_wine_fonts(wine_dir: Path, runner: CommandRunner) -> List[List[str]]:
    fonts_dir = wine_dir / "fonts"
    fontforge_script = fonts_dir / "genttf.ff"
    if not fontforge_script.is_file():
        return []

    commands: List[List[str]] = []
    for source in sorted(fonts_dir.glob("*.sfd")):
        command = ["fontforge", "-script", "genttf.ff", source.name, source.with_suffix(".ttf").name]
        runner.run(command, cwd=fonts_dir)
        commands.append(command)
    return commands


def build_wine(
    tag_text: str,
    work_dir: Path,
    config: Mapping[str, Any],
    runner: CommandRunner,
    jobs: int,
    patches_dir: Path,
    check_environment: bool = True,
    initialize_home: bool = True,
) -> Path:
    tag = parse_wine_tag(tag_text)
    validate_config(config)
    operations = select_operations(config, tag.components)
    validate_operation_files(operations, patches_dir)
    work_dir = work_dir.resolve()
    validate_oss_header(config, work_dir)
    stage = work_dir / "tmp_install"
    patch_root = work_dir / "tmp_patches"
    oss_root = work_dir / "tmp_oss"
    output_zip = work_dir / f"Wine-{tag.version}.zip"

    if check_environment:
        EnvironmentChecker(work_dir=work_dir, mingw_required=build_uses_mingw(config)).check()

    clean_patch_root(patch_root)
    patch_overrides = prepare_patch_files(operations, patches_dir, patch_root)
    prepared_oss_root: Optional[Path] = None
    if build_requires_oss_headers(config):
        prepared_oss_root = prepare_oss_headers(work_dir / "oss" / "soundcard.h", oss_root)
    else:
        clean_oss_header_root(oss_root)
    prepare_staging_root(stage)

    base_zip = ensure_base_filesystem(config, work_dir)
    wine_dir = ensure_wine_source(config, work_dir, runner)

    build_lines: List[str] = [
        f"Requested tag: {tag.tag}",
        f"Wine version: {tag.version}",
        f"Base filesystem: {base_zip.name}",
        f"Output zip: {output_zip.name}",
    ]
    checkout = ["git", "checkout", tag.tag]
    runner.run(checkout, cwd=wine_dir)
    build_lines.append(command_text(checkout))

    for operation in operations:
        if operation.reason:
            build_lines.append(f"# {operation.reason}")
        command = operation_command(operation, patches_dir, patch_overrides)
        runner.run(command, cwd=wine_dir)
        build_lines.append(command_text(command))

    font_commands = regenerate_wine_fonts(wine_dir, runner)
    if font_commands:
        build_lines.append("# Regenerate Wine bitmap TTF fonts")
        build_lines.extend(command_text(command) for command in font_commands)

    configure = configure_command(config)
    configure_env = configure_environment(config)
    if prepared_oss_root is not None:
        configure_env["OSSLIBDIR"] = str(prepared_oss_root)
    runner.run(configure, cwd=wine_dir, env=configure_env)
    env_text = " ".join(
        f"{name}={shlex.quote(configure_env[name])}"
        for name in ("CC", "CXX", "CFLAGS", "LD", "TARGETFLAGS", "LDFLAGS", "OSSLIBDIR")
        if configure_env.get(name)
    )
    build_lines.append(f"{env_text} {command_text(configure)}".strip())
    validate_config_log(config, wine_dir / "config.log")

    make = ["make", f"-j{jobs}"]
    runner.run(make, cwd=wine_dir)
    build_lines.append(command_text(make))

    install = ["make", "install", f"DESTDIR={stage}"]
    runner.run(install, cwd=wine_dir)
    build_lines.append(command_text(install))

    prune_packaged_wine(stage)
    strip_commands = strip_packaged_pe_binaries(stage, runner) if build_uses_mingw(config) else []
    if strip_commands:
        build_lines.append("# Strip PE debug and symbol data from MinGW-built Wine DLLs")
        build_lines.extend(command_text(command) for command in strip_commands)
    write_metadata(stage, tag, build_lines)
    package_zip(base_zip, stage, output_zip)
    if initialize_home:
        initialize_wine_home(work_dir, output_zip, runner)
    return output_zip


def create_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Build one Wine tag for Boxedwine.")
    parser.add_argument("tag", help="Wine tag to build, such as wine-11.0")
    parser.add_argument("--config", type=Path, default=Path(__file__).resolve().with_name("wine_builds.json"))
    parser.add_argument("--jobs", type=positive_int, default=os.cpu_count() or 1)
    parser.add_argument("--skip-env-check", action="store_true", help="Skip Debian/WSL dependency checks.")
    parser.add_argument("--skip-wineboot", action="store_true", help="Do not initialize and package /home/username/.wine.")
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = create_arg_parser()
    args = parser.parse_args(argv)
    script_dir = Path(__file__).resolve().parent

    try:
        parse_wine_tag(args.tag)
        config = load_config(args.config)
        output = build_wine(
            args.tag,
            script_dir,
            config,
            CommandRunner(),
            jobs=args.jobs,
            patches_dir=script_dir / "patches",
            check_environment=not args.skip_env_check,
            initialize_home=not args.skip_wineboot,
        )
    except BuildError as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
    except subprocess.CalledProcessError as error:
        failed_command = error.cmd if isinstance(error.cmd, (list, tuple)) else [error.cmd]
        print(f"Command failed with exit code {error.returncode}: {command_text(failed_command)}", file=sys.stderr)
        raise SystemExit(error.returncode)

    print(f"Created {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

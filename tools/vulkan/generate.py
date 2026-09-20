#!/usr/bin/env python3
"""Regenerate the Vulkan bridge in isolation; check by default, --write to update."""

import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

OUTPUTS = ("tools/vulkan/vk.c", "tools/vulkan/vkdef.h", "source/vulkan/vkdef.h",
           "source/vulkan/vkfuncs.h", "source/vulkan/vk_host.cpp", "source/vulkan/vk_host.h",
           "source/vulkan/vk_host_marshal.cpp", "source/vulkan/vk_host_marshal.h")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix="boxedwine-vulkan-") as temp:
        work = Path(temp)
        for name in ("classes", "tools/vulkan", "source/vulkan", "lib/mesa/vkRegistry"):
            (work / name).mkdir(parents=True)
        for name in ("lib/mesa/vkRegistry/vk.xml", "tools/vulkan/vkdef.h"):
            shutil.copyfile(repo / name, work / name)
        sources = sorted((repo / "tools/vulkan/gen/src").rglob("*.java"))
        subprocess.run(["javac", "-encoding", "UTF-8", "-d", str(work / "classes"),
                        *map(str, sources)], check=True)
        result = subprocess.run(["java", "-cp", str(work / "classes"), "boxedwine.org.Main"],
                                cwd=work, text=True, capture_output=True)
        if result.returncode:
            print(result.stdout + result.stderr)
            return result.returncode
        for line in result.stdout.splitlines():
            if "incomplete" in line:
                print(line)
        # Read every output before replacing anything; generator failures must
        # not leave a mixture of old and new host/guest source in the checkout.
        generated = {name: (work / name).read_text(encoding="utf-8") for name in OUTPUTS}
        changed = [name for name, value in generated.items()
                   if (repo / name).read_text(encoding="utf-8") != value]
        if args.write:
            for name in changed:
                (repo / name).write_text(generated[name], encoding="utf-8", newline="\n")
        print(f"{'Updated' if args.write else 'Different'}: {len(changed)} generated files")
        for name in changed:
            print(name)
        return 0 if args.write or not changed else 1


if __name__ == "__main__":
    raise SystemExit(main())

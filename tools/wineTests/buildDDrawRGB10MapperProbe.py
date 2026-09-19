"""Build the mapper probe from an already patched Wine tree using MinGW.

The public DirectDraw texture enumeration does not list RGB10 formats. Compile
the two real conversion functions into a device-independent probe to check the
forward mapping as well as the public surface-creation path's reverse mapping.
"""
import argparse
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine-source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="New build directory")
    parser.add_argument("--compiler", default="i686-w64-mingw32-gcc")
    args = parser.parse_args()
    source = args.wine_source.resolve()
    output = args.output.resolve()
    text = (source / "dlls/ddraw/utils.c").read_text(encoding="utf-8")
    header = (source / "include/wine/wined3d.h").read_text(encoding="utf-8")
    formats = re.findall(r"^enum wined3d_format_id\n\{.*?^\};", header, re.MULTILINE | re.DOTALL)
    if len(formats) != 1:
        parser.error("Expected exactly one Wine format enumeration")
    functions = []
    for name in ("ddrawformat_from_wined3dformat", "wined3dformat_from_ddrawformat"):
        matches = re.findall(r"^(?:void|enum wined3d_format_id) " + name
                + r"\([^;]*?\n\{.*?^\}\n", text, re.MULTILINE | re.DOTALL)
        if len(matches) != 1:
            parser.error(f"Expected exactly one complete definition of {name}")
        functions.append(matches[0])
    output.mkdir(parents=True, exist_ok=False)
    # Retain Wine's source license alongside the unmodified function bodies.
    license_text = text[:text.index("*/") + 2]
    (output / "ddraw_format_mappers.inc").write_text(license_text + "\n" + formats[0]
            + "\n" + "\n".join(functions), encoding="utf-8")
    executable = output / "DDrawRGB10MapperProbe.exe"
    subprocess.run([args.compiler, "-O2", "-Wall", "-Wextra", "-Werror",
        "-I", str(output), str(Path(__file__).with_name("tests") / "ddraw_rgb10_mapper_probe.c"),
        "-o", str(executable)], check=True)
    print(executable)


if __name__ == "__main__":
    main()

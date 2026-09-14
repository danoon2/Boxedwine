#!/usr/bin/env python3
"""Build the inspected psVoodoo revision for isolated Wine experiments only."""
import argparse
import concurrent.futures
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess

REVISION = "8ebfc3c45f8f067af852720f9857d4641bf64252"


def run(command, **kwargs):
    return subprocess.run(command, check=True, text=True, capture_output=True, **kwargs).stdout


def replace_once(path, before, after):
    text = path.read_text()
    if text.count(before) != 1:
        raise RuntimeError(f"Unexpected source in {path.name}: {before!r}")
    path.write_text(text.replace(before, after))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path, help="Clean official Git checkout")
    parser.add_argument("--toolchain", required=True, type=Path, help="LLVM-MinGW bin directory")
    parser.add_argument("--output", required=True, type=Path, help="New scratch build directory")
    parser.add_argument("--gamma-logexp", action="store_true",
                        help="Experimental: lower gamma shader pow to log/mul/exp")
    parser.add_argument("--d3d9-only", action="store_true",
                        help="Experimental: use the existing D3D9 fallback instead of D3D9Ex")
    parser.add_argument("--trace-present", action="store_true",
                        help="Diagnostic: log the first presentation calls to psVoodoo.log")
    args = parser.parse_args()
    upstream = args.source.resolve()
    toolchain = args.toolchain.resolve()
    output = args.output.resolve()
    revision = run(["git", "-C", str(upstream), "rev-parse", "HEAD"]).strip()
    if revision != REVISION:
        raise RuntimeError(f"Expected {REVISION}, found {revision}; review new sources first")
    run(["git", "-C", str(upstream), "diff", "--exit-code", "HEAD", "--"])
    output.mkdir(parents=True, exist_ok=False)
    source = output / "source"
    objects = output / "objects"
    objects.mkdir()
    files = run(["git", "-C", str(upstream), "ls-files", "-z"]).split("\0")
    for name in filter(None, files):
        dest = source / name
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(upstream / name, dest)

    # Match MSVC's acceptance of binding this temporary to a read-only parameter.
    for name in ("Palette.cpp", "Palette.h"):
        replace_once(source / name, "SetChromaKey(CColor &key)", "SetChromaKey(const CColor &key)")
    replace_once(source / "entry.cpp", "#include <math.h>", "#include <math.h>\n#include <stdio.h>")
    # Descent 3 sizes its texture cache from this query. Upstream's zero-returning
    # stub makes it allocate an empty cache and later dereference 0xDEADBEEF.
    # m_freemem is the next allocation offset, despite its name.
    replace_once(source / "UTMU.h", "    void         guTexMemReset(void);",
                 "    FxU32        guTexMemQueryAvail(GrChipID_t tmu) const;\n"
                 "    void         guTexMemReset(void);")
    replace_once(source / "UTMU.cpp", "void CUTMU::guTexMemReset(void)",
                 "FxU32 CUTMU::guTexMemQueryAvail(GrChipID_t tmu) const\n"
                 "{\n"
                 "    // psVoodoo exposes one TMU with TEXMEMSIZE bytes.\n"
                 "    return tmu == GR_TMU0 && m_freemem < TEXMEMSIZE\n"
                 "        ? TEXMEMSIZE - m_freemem : 0;\n"
                 "}\n\n"
                 "void CUTMU::guTexMemReset(void)")
    replace_once(source / "entry.cpp",
                 "    Log::WarnOnce(ENOSUP_guTexMemQueryAvail);\n    return 0;",
                 "    return glide ? glide->guTexMemQueryAvail(tmu) : 0;")
    # The wrapper's resource uses Windows version constants, not MFC controls.
    (source / "afxres.h").write_text("#include <winres.h>\n")

    # Local include spellings were written for Windows' case-insensitive filesystem.
    local_headers = {p.name.lower(): p.name for p in source.glob("*.h")}
    for path in list(source.glob("*.cpp")) + list(source.glob("*.h")):
        contents = path.read_text()
        contents = re.sub(r'(#include\s+")([^"/]+)(")',
                          lambda m: m[1] + local_headers.get(m[2].lower(), m[2].lower()) + m[3],
                          contents)
        path.write_text(contents)

    if args.gamma_logexp:
        # Diagnostic only. Preserve gamma's exponent, then validate its edge cases
        # and image output before considering this as a shipping compatibility fix.
        for channel in "rgb":
            replace_once(source / "RenderBuffer2D.cpp",
                         f'"pow r1, r0.{channel}, c0.{channel}\\n"',
                         f'"log r1, r0.{channel}\\n"\n'
                         f'               "mul r1, r1, c0.{channel}\\n"\n'
                         '               "exp r1, r1.x\\n"')

    if args.d3d9_only:
        replace_once(source / "Device.cpp",
                     'if(createEx && SUCCEEDED(createEx(D3D_SDK_VERSION, &d3dEx)))',
                     'if(false && createEx && SUCCEEDED(createEx(D3D_SDK_VERSION, &d3dEx)))')

    if args.trace_present:
        path = source / "RenderBuffer2D.cpp"
        text = path.read_text()
        for signature in ("void CRenderBuffer2D::doPresentationRender(int i, LPDIRECT3DSURFACE9 flip)",
                          "bool CRenderBuffer2D::Present(int i, bool wait)"):
            marker = signature + "\n{"
            assert text.count(marker) == 1
            text = text.replace(marker, marker + "\n    static unsigned traceCount = 0;\n"
                                "    bool tracePresent = traceCount++ < 5;\n")
        calls = ["m_rend->PushStateForRenderTexture();",
                 "m_rend->m_device->SetRenderTarget(0, flip);",
                 "m_rend->m_device->SetTexture(0, m_presentation_tex[i]);",
                 "m_rend->m_device->SetPixelShader(GetPresentationShader());",
                 "m_rend->m_device->SetPixelShaderConstantF(0, shaderC0, 1);",
                 "m_rend->ResetTarget();", "m_rend->PopState();",
                 "doPresentationRender(i, flip);", "flip->Release();",
                 "res = swap_chain->Present(NULL, NULL, NULL, NULL, (wait?0:D3DPRESENT_DONOTWAIT));",
                 "swap_chain->Release();"]
        for i, call in enumerate(calls):
            assert text.count(call) == 1, call
            text = text.replace(call,
                f'if (tracePresent) Log::Printf("TRACE {i} begin\\n");\n    {call}\n'
                f'    if (tracePresent) Log::Printf("TRACE {i} end\\n");')
        path.write_text(text)
        (output / "trace-calls.json").write_text(json.dumps(calls, indent=2) + "\n")
        replace_once(source / "Device.cpp", "    m_pushed_state->Apply();",
                     '    Log::Printf("STATE Apply begin\\n");\n'
                     '    m_pushed_state->Apply();\n'
                     '    Log::Printf("STATE Apply end\\n");')
        replace_once(source / "Device.cpp", "    m_pushed_state->Release();\n    m_pushed_state = NULL;",
                     '    Log::Printf("STATE Release begin\\n");\n'
                     '    m_pushed_state->Release();\n'
                     '    Log::Printf("STATE Release end\\n");\n    m_pushed_state = NULL;')

    # Keep all RGB565 values available for LFB writes, including F-16's cyan HUD.
    lfb_patch = Path(__file__).resolve().with_name("lfb-colors.patch")
    run(["git", "apply", "--unsafe-paths", "--directory=" + str(source), str(lfb_patch)])
    shutil.copy2(lfb_patch, output / lfb_patch.name)

    # Queued triangles must retain the clipping rectangle active at submission.
    clip_patch = Path(__file__).resolve().with_name("clip-window.patch")
    run(["git", "apply", "--unsafe-paths", "--directory=" + str(source), str(clip_patch)])
    shutil.copy2(clip_patch, output / clip_patch.name)

    # F-16 uses an inverted, distance-dependent texture alpha combine mode.
    detail_patch = Path(__file__).resolve().with_name("texture-detail.patch")
    run(["git", "apply", "--unsafe-paths", "--directory=" + str(source), str(detail_patch)])
    shutil.copy2(detail_patch, output / detail_patch.name)

    compiler = str(toolchain / "i686-w64-mingw32-clang++")
    flags = ["-O2", "-std=c++11", "-DNDEBUG", "-DWIN32", "-D_WINDOWS", "-D_USRDLL",
             "-DPGVOODOO_EXPORTS", "-DCPPDLL", "-D_CRT_SECURE_NO_WARNINGS",
             "-D__IDirect3D9Ex_INTERFACE_DEFINED__", "-D__IDirect3DDevice9Ex_INTERFACE_DEFINED__",
             "-I" + str(source), "-Wno-ignored-pragmas", "-Wno-constant-conversion",
             "-Wno-deprecated-declarations", "-Wno-c++11-narrowing"]
    commands = [[compiler, *flags, "-c", str(path), "-o", str(objects / (path.stem + ".o"))]
                for path in sorted(source.glob("*.cpp"))]

    def compile_one(command):
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (objects / (Path(command[-1]).stem + ".log")).write_text(result.stdout)
        return result

    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(compile_one, commands))
    (output / "build.log").write_text("\n".join(result.stdout for result in results))
    if any(result.returncode for result in results):
        raise RuntimeError(f"Compilation failed; see {output / 'build.log'}")

    resource_command = [str(toolchain / "i686-w64-mingw32-windres"), "-I" + str(source),
                        str(source / "psVoodoo.rc"), str(objects / "resource.o")]
    run(resource_command)
    symbols = run([str(toolchain / "llvm-nm"), "-g", "--defined-only", str(objects / "entry.o")])
    exports = re.findall(r" T _([A-Za-z]\w*)@(\d+)$", symbols, re.MULTILINE)
    if not exports:
        raise RuntimeError("No stdcall exports found")
    # Games use Microsoft's leading underscore; the diagnostic uses bare names.
    definitions = ["EXPORTS"]
    for name, size in exports:
        definitions += [f'"_{name}@{size}" = "{name}@{size}"', f'"{name}" = "{name}@{size}"']
    export_file = output / "glide2x.def"
    export_file.write_text("\n".join(definitions) + "\n")
    dll = output / "glide2x.dll"
    link_command = [compiler, "-shared", "-static", "-o", str(dll),
                    *[str(p) for p in sorted(objects.glob("*.o"))], str(export_file),
                    "-ld3d9", "-ld3dx9_43", "-lwinmm", "-lgdi32", "-ladvapi32", "-luser32"]
    run(link_command)
    inspection = run([str(toolchain / "llvm-readobj"), "--coff-imports", "--coff-exports", str(dll)])
    (output / "pe-inspection.txt").write_text(inspection)
    metadata = {"revision": revision, "gamma_logexp": args.gamma_logexp,
                "texture_memory_query": True,
                "lfb_color_mask": True,
                "clip_window_flush": True,
                "texture_detail_alpha": True,
                "detail_patch_sha256": hashlib.sha256(detail_patch.read_bytes()).hexdigest(),
                "clip_patch_sha256": hashlib.sha256(clip_patch.read_bytes()).hexdigest(),
                "lfb_patch_sha256": hashlib.sha256(lfb_patch.read_bytes()).hexdigest(),
                "d3d9_only": args.d3d9_only,
                "trace_present": args.trace_present,
                "compiler": run([compiler, "--version"]),
                "commands": [*commands, resource_command, link_command],
                "dll_bytes": dll.stat().st_size,
                "dll_sha256": hashlib.sha256(dll.read_bytes()).hexdigest(),
                "shipping_approved": False}
    (output / "build.json").write_text(json.dumps(metadata, indent=2) + "\n")
    print(dll)
    print("Experimental build only; see GLIDE_SUPPORT.md for compatibility and source-notice findings.")


if __name__ == "__main__":
    main()

"""Run a graphics reproducer or focused Wine test with the browser harness.

This is a diagnostic probe, not an accepted-result baseline or a Wine test skip.
Compile the selected source in tests/ as PE32/i386 and pass --executable.
For point-size capability and Wine rendering probes, use buildPointSizeProbes.py.
"""
import argparse
from dataclasses import replace
import json
from pathlib import Path

import wineGraphicsBrowser as graphics


def main(default_probe=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", choices=("clip-planes", "colorkey", "clip-state", "viewport-depth", "gl-depth-range", "depth-bias", "depth-copy", "depth-readback", "stencil-clear", "point-caps8", "point-caps9", "wine-points8", "wine-points9", "context-isolation", "context-concurrency", "late-context", "thread-binding", "present-lifecycle", "self-blit", "alpha-test", "multisample-copy", "framebuffer-yield", "drawable-yield", "readback-yield", "copy-read-buffer", "read-buffer-ownership", "readback-pack", "format-samples", "d3d9-format-samples", "rgb10-transfer", "depth-samples", "float-caps", "xfile-object-limit", "float-fog", "float-texture-transform", "normal-texgen", "strict-fog", "d3dx-tangent", "d3dx-states", "d3dx-sphere", "texgen", "ffp-failure", "mapped-buffers", "glsl-failure", "blitter-failure"), default=default_probe, required=default_probe is None)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--filesystem", type=Path, required=True)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="New artifact directory")
    parser.add_argument("--headless", action="store_true")
    parser.add_argument("--keep-browser-profile", action="store_true",
        help="Retain the isolated Chrome profile for diagnostic follow-up")
    parser.add_argument("--mode", default="single-threaded-non-jit", choices=(
        "single-threaded-non-jit", "single-threaded-jit",
        "multi-threaded-non-jit", "multi-threaded-jit"))
    parser.add_argument("--chrome", type=Path)
    parser.add_argument("--timeout", type=int, default=120)
    parser.add_argument("--repeat", type=int, default=1,
        help="Repeat launches in one browser session; retain and validate every cycle")
    parser.add_argument("--cleanup-wait-seconds", type=int,
        help="Require Wine cleanup, then observe this many seconds for late browser errors")
    parser.add_argument("--ffp-injection", choices=("none", "compile", "early", "late"), default="none",
        help="FFP probe only: require test-only injected Wine failures; production DLLs ignore this setting")
    parser.add_argument("--glsl-injection", choices=("none", "compile", "link"), default="none",
        help="GLSL probe only: require real driver faults from the separate test-only Wine injection")
    parser.add_argument("--blitter-injection", choices=("none", "compile", "link"), default="none",
        help="Blitter probe only: require real driver faults from the separate test-only Wine injection")
    args = parser.parse_args()
    if args.ffp_injection != "none" and args.probe != "ffp-failure":
        parser.error("--ffp-injection requires --probe ffp-failure")
    if args.glsl_injection != "none" and args.probe != "glsl-failure":
        parser.error("--glsl-injection requires --probe glsl-failure")
    if args.blitter_injection != "none" and args.probe != "blitter-failure":
        parser.error("--blitter-injection requires --probe blitter-failure")
    suite = {
        "blitter-failure": graphics.GraphicsSuite("blitter-failure", "BlitterFailureProbe.exe", ("blitter_failure",),
            group_arguments=("--fault",) if args.blitter_injection != "none" else (),
            environment=("BW_TEST_BLITTER_FAULT=" + args.blitter_injection, "WINEDEBUG=-all,err+all"),
            redirect_output=True, cleanup_wait_seconds=15),
        "glsl-failure": graphics.GraphicsSuite("glsl-failure", "GLSLFailureProbe.exe", ("glslfailure",),
            group_arguments=("--" + args.glsl_injection + "-fault",) if args.glsl_injection != "none" else (),
            environment=("BW_TEST_GLSL_FAILURE=" + args.glsl_injection, "WINEDEBUG=-all,err+d3d_shader"),
            redirect_output=True, cleanup_wait_seconds=15),
        "mapped-buffers": graphics.GraphicsSuite("mapped-buffers", "MappedBufferProbe.exe", ("mappedbuffers",),
            redirect_output=True, cleanup_wait_seconds=15),
        "ffp-failure": graphics.GraphicsSuite("ffp-failure", "FFPFailureProbe.exe", ("ffpfailure",),
            group_arguments=("--fault",) if args.ffp_injection != "none" else (),
            environment=("BW_TEST_FFP_FAILURE=" + args.ffp_injection,), redirect_output=True, cleanup_wait_seconds=15),
        "texgen": graphics.GraphicsSuite("d3d9-texgen", "D3D9TexgenProbe.exe", ("texgen",), redirect_output=True, cleanup_wait_seconds=15),
        "d3dx-sphere": graphics.GraphicsSuite("d3dx-sphere", "D3DXSphereProbe.exe", ("d3dxsphere",), redirect_output=True, cleanup_wait_seconds=15),
        "d3dx-states": graphics.GraphicsSuite("d3dx-states", "D3DXStatesProbe.exe", ("d3dxstates",), redirect_output=True, cleanup_wait_seconds=15),
        "d3dx-tangent": graphics.GraphicsSuite("d3dx-tangent", "D3DXTangentProbe.exe", ("d3dxtangent",), redirect_output=True, cleanup_wait_seconds=15),
        "float-fog": graphics.GraphicsSuite("float-fog", "D3D9FloatFogProbe.exe", ("visual",), redirect_output=True),
        "float-texture-transform": graphics.GraphicsSuite("float-texture-transform", "D3D9FloatTextureTransformProbe.exe", ("visual",), redirect_output=True),
        "normal-texgen": graphics.GraphicsSuite("normal-texgen", "D3D9NormalTexgenProbe.exe", ("visual",), redirect_output=True),
        "strict-fog": graphics.GraphicsSuite("strict-fog", "D3D9StrictFogProbe.exe", ("visual",), redirect_output=True),
        "xfile-object-limit": graphics.GraphicsSuite("xfile-object-limit", "XFileObjectLimitProbe.exe", ("objects",), result_style="marshal", redirect_output=True),
        "format-samples": graphics.GraphicsSuite("format-samples", "FormatSamplesProbe.exe", ("samples",), result_style="marshal", redirect_output=True),
        "d3d9-format-samples": graphics.GraphicsSuite("d3d9-format-samples", "D3D9FormatSamplesProbe.exe", ("formats",), redirect_output=True),
        "rgb10-transfer": graphics.GraphicsSuite("rgb10-transfer", "D3D9RGB10TransferProbe.exe", ("rgb10",), redirect_output=True),
        "float-caps": graphics.GraphicsSuite("float-caps", "D3D9FloatCapsProbe.exe", ("floatcaps",), redirect_output=True),
        "depth-samples": graphics.GraphicsSuite("depth-samples", "D3D9DepthSamplesProbe.exe", ("depthsamples",), redirect_output=True),
        "thread-binding": graphics.GraphicsSuite("wgl-thread-binding", "WGLThreadBindingProbe.exe", ("binding",), redirect_output=True),
        "context-concurrency": graphics.GraphicsSuite("d3d9-concurrent-contexts", "D3D9ConcurrentContextProbe.exe", ("concurrent",), redirect_output=True),
        "late-context": graphics.GraphicsSuite("d3d9-late-context", "D3D9LateContextProbe.exe", ("latecontext",), redirect_output=True),
        "framebuffer-yield": graphics.GraphicsSuite("framebuffer-yield", "FramebufferYieldProbe.exe", ("yield",), result_style="marshal", redirect_output=True),
        "drawable-yield": graphics.GraphicsSuite("drawable-yield", "DrawableYieldProbe.exe", ("yield",), result_style="marshal", redirect_output=True),
        "readback-yield": graphics.GraphicsSuite("readback-yield", "ReadbackYieldProbe.exe", ("yield",), result_style="marshal", redirect_output=True),
        "copy-read-buffer": graphics.GraphicsSuite("copy-read-buffer", "CopyReadBufferProbe.exe", ("copy",), result_style="marshal", redirect_output=True),
        "read-buffer-ownership": graphics.GraphicsSuite("read-buffer-ownership", "ReadBufferOwnershipProbe.exe", ("ownership",), result_style="marshal", redirect_output=True),
        "readback-pack": graphics.GraphicsSuite("readback-pack", "ReadbackPackProbe.exe", ("pack",), result_style="marshal", redirect_output=True),
        "multisample-copy": graphics.GraphicsSuite("d3d9-multisample-copy", "D3D9MultisampleCopyProbe.exe", ("mscopy",), redirect_output=True),
        "alpha-test": graphics.GraphicsSuite("d3d9-alpha", "D3D9AlphaProbe.exe", ("alpha",), redirect_output=True),
        "self-blit": graphics.GraphicsSuite("ddraw-self-blit", "DDrawSelfBlitProbe.exe", ("selfblit",), redirect_output=True),
        "present-lifecycle": graphics.GraphicsSuite("d3d9-present-lifecycle", "D3D9PresentLifecycleProbe.exe", ("present",), redirect_output=True),
        "context-isolation": graphics.GraphicsSuite("d3d9-context-isolation", "D3D9ContextTextureProbe.exe", ("contexts",), redirect_output=True),
        "point-caps8": graphics.GraphicsSuite("d3d8-point-caps", "D3D8PointSizeCapsProbe.exe", ("pointsize",), redirect_output=True),
        "point-caps9": graphics.GraphicsSuite("d3d9-point-caps", "D3D9PointSizeCapsProbe.exe", ("pointsize",), redirect_output=True),
        "wine-points8": graphics.GraphicsSuite("d3d8-point-rendering", "D3D8WinePointSizeProbe.exe", ("visual",), redirect_output=True),
        "wine-points9": graphics.GraphicsSuite("d3d9-point-rendering", "D3D9WinePointSizeProbe.exe", ("visual",), redirect_output=True),
        "stencil-clear": graphics.GraphicsSuite("ddraw-stencil-clear", "DDrawStencilClearProbe.exe", ("readback",), redirect_output=True),
        "depth-readback": graphics.GraphicsSuite("ddraw-depth-readback", "DDrawDepthReadbackCapsProbe.exe", ("readback",), redirect_output=True),
        "depth-copy": graphics.GraphicsSuite("d3d9-depth-copy", "D3D9DepthCopyProbe.exe", ("copy",), redirect_output=True),
        "depth-bias": graphics.GraphicsSuite("d3d9-depth-bias", "D3D9DepthBiasStateProbe.exe", ("bias",), redirect_output=True),
        "gl-depth-range": graphics.GraphicsSuite("wgl-depth-range", "WGLDepthRangeProbe.exe", ("depth",), redirect_output=True),
        "clip-planes": graphics.GraphicsSuite("d3d9-clip-planes", "D3D9ClipPlanesProbe.exe", ("caps",), redirect_output=True),
        "colorkey": graphics.GraphicsSuite("ddraw-colorkey", "DDrawColorKeyProbe.exe", ("colorkey",), redirect_output=True),
        "viewport-depth": graphics.GraphicsSuite("ddraw-depth", "DDrawDepthFollowup.exe", ("depth",), redirect_output=True),
        "clip-state": graphics.GraphicsSuite("ddraw-clip-state", "DDrawClipStateProbe.exe", ("clipstate",), redirect_output=True),
    }[args.probe]
    suite = replace(suite, exit_status_policy="zero")
    if not 1 <= args.repeat <= 100:
        parser.error("Repeat count must be between 1 and 100")
    if args.probe in ("mapped-buffers", "glsl-failure", "blitter-failure") and args.repeat != 1:
        parser.error(args.probe + " currently requires one launch per invocation")
    if args.repeat > 1:
        suite = replace(suite, repeat_count=args.repeat, cleanup_wait_seconds=15)
    if args.cleanup_wait_seconds is not None:
        if args.cleanup_wait_seconds < 0 or args.cleanup_wait_seconds >= args.timeout:
            parser.error("Cleanup wait must be nonnegative and shorter than the timeout")
        suite = replace(suite, cleanup_wait_seconds=args.cleanup_wait_seconds)
    if args.probe in ("glsl-failure", "blitter-failure") and suite.cleanup_wait_seconds < 15:
        parser.error(args.probe + " requires at least 15 seconds of cleanup observation")
    if suite.cleanup_wait_seconds is not None and suite.cleanup_wait_seconds >= args.timeout:
        parser.error("Cleanup observation must be shorter than the timeout")
    graphics.validate_web_build(args.build_dir)
    graphics.validate_test_executable(args.executable, suite)
    if not args.filesystem.is_file():
        parser.error(f"Filesystem does not exist: {args.filesystem}")
    result, manifest = graphics.run_browser_test(
        suite=suite, group=suite.groups[0], build_dir=args.build_dir.resolve(),
        filesystem=args.filesystem.resolve(), test_executable=args.executable.resolve(),
        chrome=args.chrome or graphics.find_chrome(), run_dir=args.output.resolve(),
        timeout=args.timeout, headless=args.headless, keep_browser_profile=args.keep_browser_profile,
        mode=args.mode,
    )
    if args.probe == "d3dx-tangent":
        from auditD3DXTangent import audit_tangent
        log = Path(manifest["artifacts"]["wine_log"]).read_text(encoding="utf-8", errors="replace")
        audit = audit_tangent(graphics.normalize_output(log))
        manifest["tangent_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="tangent vector/coverage audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "d3dx-states":
        from auditD3DXStates import audit_states
        log = Path(manifest["artifacts"]["wine_log"]).read_text(encoding="utf-8", errors="replace")
        audit = audit_states(graphics.normalize_output(log))
        manifest["states_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="D3DX state/pixel/coverage audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "d3dx-sphere":
        from auditD3DXSphere import audit_sphere
        log = Path(manifest["artifacts"]["wine_log"]).read_text(encoding="utf-8", errors="replace")
        audit = audit_sphere(graphics.normalize_output(log))
        manifest["sphere_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="D3DX sphere/input/coverage audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "texgen":
        from auditD3D9Texgen import audit_texgen
        log = Path(manifest["artifacts"]["wine_log"]).read_text(encoding="utf-8", errors="replace")
        audit = audit_texgen(graphics.normalize_output(log))
        manifest["texgen_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="generated texture coordinate/coverage audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "mapped-buffers":
        from auditMappedBuffers import audit_mapped_payload
        payload = json.loads(Path(manifest["artifacts"]["browser_payload"]).read_text(encoding="utf-8"))
        audit = audit_mapped_payload(payload)
        manifest["mapped_buffers_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="mapped vertex/index buffer pixel/coverage audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "blitter-failure":
        from auditBlitterFailure import audit_payload
        payload = json.loads(Path(manifest["artifacts"]["browser_payload"]).read_text(encoding="utf-8"))
        audit = audit_payload(payload, Path(manifest["artifacts"]["chrome_log"]).read_bytes(),
            manifest["launch_url"], args.blitter_injection)
        manifest["blitter_failure_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="blitter failure recovery/pixel/cache audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "glsl-failure":
        from auditGLSLFailure import audit_payload
        payload = json.loads(Path(manifest["artifacts"]["browser_payload"]).read_text(encoding="utf-8"))
        audit = audit_payload(payload, Path(manifest["artifacts"]["chrome_log"]).read_bytes(),
            manifest["launch_url"], args.glsl_injection)
        manifest["glsl_failure_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="GLSL failure recovery/pixel/source audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.probe == "ffp-failure":
        from auditFFPFailure import audit_ffp_failure
        log = Path(manifest["artifacts"]["wine_log"]).read_text(encoding="utf-8", errors="replace")
        audit = audit_ffp_failure(graphics.normalize_output(log), args.ffp_injection != "none")
        manifest["ffp_failure_audit"] = audit
        if not audit["passed"]:
            result = replace(result, passed=False, reason="fixed-function failure recovery/pixel audit failed")
            manifest["result"].update(passed=False, reason=result.reason)
        (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest["result"], indent=2))
    print(f"Artifacts: {args.output.resolve()}")
    return 0 if result.passed else 1


if __name__ == "__main__":
    raise SystemExit(main())

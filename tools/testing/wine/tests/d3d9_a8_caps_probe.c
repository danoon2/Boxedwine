/* Check A8 render-target policy without changing the prefix configuration.
 * i686-w64-mingw32-gcc -O2 -Wall d3d9_a8_caps_probe.c -ld3d9 -o a8-caps.exe
 * Usage: a8-caps.exe output.txt enabled|disabled expected-major expected-minor
 * Configure the Windows version and DisableA8 externally before each process.
 * "enabled" requires a host that supports A8 render targets.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int checks, failures;
#define CHECK(c, ...) do { ++checks; if (!(c)) { ++failures; printf("FAIL: " __VA_ARGS__); puts(""); } } while (0)

int main(int argc, char **argv)
{
    OSVERSIONINFOW version = {0};
    LONG (WINAPI *rtl_get_version)(OSVERSIONINFOW *);
    IDirect3D9 *d3d;
    D3DDISPLAYMODE mode;
    HRESULT hr, rt_result;

    if (argc != 5 || (strcmp(argv[2], "enabled") && strcmp(argv[2], "disabled"))) return 2;
    if (!freopen(argv[1], "w", stdout)) return 2;
    version.dwOSVersionInfoSize = sizeof(version);
    rt_result = !strcmp(argv[2], "disabled") ? D3DERR_NOTAVAILABLE : D3D_OK;
    /* GetVersionEx caps the reported version for unmanifested Windows 8+ apps. */
    rtl_get_version = (void *)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
    CHECK(rtl_get_version && !rtl_get_version(&version), "RtlGetVersion");
    printf("Windows %lu.%lu, A8 render targets expected %s\n",
            version.dwMajorVersion, version.dwMinorVersion, argv[2]);
    CHECK(version.dwMajorVersion == strtoul(argv[3], NULL, 10)
            && version.dwMinorVersion == strtoul(argv[4], NULL, 10), "Unexpected Windows version");
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    CHECK(d3d != NULL, "Direct3DCreate9");
    if (!d3d) goto done;
    hr = IDirect3D9_GetAdapterDisplayMode(d3d, 0, &mode);
    CHECK(hr == D3D_OK, "GetAdapterDisplayMode: %#lx", (unsigned long)hr);
    if (FAILED(hr)) goto release;

    hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
            0, D3DRTYPE_TEXTURE, D3DFMT_A8);
    CHECK(hr == D3D_OK, "Ordinary A8 texture: %#lx", (unsigned long)hr);
    hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
            D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, D3DFMT_A8);
    CHECK(hr == D3D_OK, "A8 texture filtering: %#lx", (unsigned long)hr);
    hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
            D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8);
    CHECK(hr == rt_result, "A8 render-target texture: %#lx", (unsigned long)hr);
    hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
            D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, D3DFMT_A8);
    CHECK(hr == rt_result, "A8 render-target surface: %#lx", (unsigned long)hr);
    hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
            D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, D3DFMT_A8);
    CHECK(hr == rt_result, "A8 render-target blending: %#lx", (unsigned long)hr);
    hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
            D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8);
    CHECK(hr == D3D_OK, "RGBA render-target fallback: %#lx", (unsigned long)hr);
release:
    IDirect3D9_Release(d3d);
done:
    printf("%u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}

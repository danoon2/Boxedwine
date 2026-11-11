/* Regression for Wine builtin modules reused through multiple assembly paths.
 * Build: i686-w64-mingw32-gcc -O2 builtin-dll-alias.c -o builtin-dll-alias.exe
 * Use --loader-only for repeated version.dll load/unload tests (also on Windows).
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(cond, msg) do { ++checks; if (!(cond)) { ++failures; printf("FAIL: %s (error %lu)\n", msg, GetLastError()); } } while (0)

static BOOL write_file(const char *path, const char *text)
{
    DWORD written;
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    BOOL result;
    if (file == INVALID_HANDLE_VALUE) return FALSE;
    result = WriteFile(file, text, (DWORD)strlen(text), &written, NULL) && written == strlen(text);
    CloseHandle(file);
    return result;
}

int main(int argc, char **argv)
{
    static const char manifest_format[] = "<?xml version=\"1.0\"?>"
        "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">"
        "<assemblyIdentity name=\"Boxedwine.BuiltinAlias\" version=\"1.0.0.0\" type=\"win32\"/>"
        "<file name=\"%s\"/>%s</assembly>";
    BOOL loader_only = argc > 1 && !strcmp(argv[1], "--loader-only");
    const char *dll = loader_only ? "version.dll" : "msvcr80.dll";
    char manifest[512];
    char source[MAX_PATH], temp[MAX_PATH], base[MAX_PATH], dir[3][MAX_PATH];
    char crt[3][MAX_PATH], cxx[MAX_PATH], manifest_path[MAX_PATH], path[MAX_PATH], filename[MAX_PATH];
    ACTCTXA act = {sizeof(act)};
    HANDLE context = INVALID_HANDLE_VALUE;
    HMODULE first = NULL, second = NULL, repeated = NULL, cpp = NULL, by_handle = NULL;
    ULONG_PTR cookie = 0;
    unsigned i, iteration;
    setvbuf(stdout, NULL, _IONBF, 0);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    GetSystemDirectoryA(source, MAX_PATH);
    snprintf(manifest, sizeof(manifest), manifest_format, dll, loader_only ? "" : "<file name=\"msvcp80.dll\"/>");
    GetTempPathA(MAX_PATH, temp);
    GetTempFileNameA(temp, "bwa", 0, base);
    DeleteFileA(base);
    CHECK(CreateDirectoryA(base, NULL), "create fixture directory");
    for (i = 0; i < 3; ++i)
    {
        snprintf(dir[i], MAX_PATH, "%s\\%u", base, i);
        CHECK(CreateDirectoryA(dir[i], NULL), "create alias directory");
        snprintf(crt[i], MAX_PATH, "%s\\%s", dir[i], dll);
        snprintf(path, MAX_PATH, "%s\\%s", source, dll);
        CHECK(CopyFileA(path, crt[i], FALSE), "copy C runtime fixture");
    }
    snprintf(cxx, MAX_PATH, "%s\\msvcp80.dll", dir[1]);
    snprintf(path, MAX_PATH, "%s\\msvcp80.dll", source);
    if (!loader_only) CHECK(CopyFileA(path, cxx, FALSE), "copy C++ runtime fixture");
    snprintf(manifest_path, MAX_PATH, "%s\\test.manifest", dir[1]);
    CHECK(write_file(manifest_path, manifest), "write activation manifest");
    act.lpSource = manifest_path;
    context = CreateActCtxA(&act);
    CHECK(context != INVALID_HANDLE_VALUE, "create activation context");
    if (failures) goto cleanup;
    /* The ELF VC80 CRT has a separate reinitialization failure on repeated
     * unload/reload. Exercise alias retirement with version.dll instead. */
    for (iteration = 0; iteration < (loader_only ? 3u : 1u); ++iteration)
    {
        first = LoadLibraryA(crt[0]);
        CHECK(first != NULL, "load first path");
        CHECK(GetModuleHandleA(crt[0]) == first, "find first loaded path");
        CHECK(GetModuleHandleA(crt[1]) == NULL, "unrequested alias must not be visible");
        second = LoadLibraryA(crt[1]);
        CHECK(second != NULL, "load second path");
        printf("iteration %u: first=%p second=%p lookup=%p\n", iteration, first, second, GetModuleHandleA(crt[1]));
        CHECK(GetModuleHandleA(crt[1]) == second, "find second loaded path");
        CHECK(GetModuleHandleA(crt[2]) == NULL, "never-loaded third path must not alias");
        GetModuleFileNameA(first, filename, MAX_PATH);
        CHECK(!lstrcmpiA(filename, crt[0]), "retain original module filename");
        repeated = LoadLibraryA(crt[1]);
        CHECK(repeated == second, "repeated alias load reuses handle");
        CHECK(ActivateActCtx(context, &cookie), "activate second path");
        CHECK(GetModuleHandleA(dll) == second, "find runtime through activation context");
        CHECK(GetModuleHandleExA(0, dll, &by_handle) && by_handle == second, "add reference through alias");
        if (by_handle) FreeLibrary(by_handle);
        by_handle = NULL;
        if (!loader_only) {
            cpp = LoadLibraryA("msvcp80.dll");
            CHECK(cpp != NULL, "C++ runtime initialization finds C runtime");
        }
        if (cpp) FreeLibrary(cpp);
        cpp = NULL;
        CHECK(DeactivateActCtx(0, cookie), "deactivate context");
        cookie = 0;
        if (repeated) FreeLibrary(repeated);
        repeated = NULL;
        if (first) FreeLibrary(first);
        first = NULL;
        CHECK(GetModuleHandleA(crt[1]) == second, "alias survives while referenced");
        if (second) FreeLibrary(second);
        second = NULL;
        CHECK(GetModuleHandleA(crt[0]) == NULL, "first path disappears after unload");
        CHECK(GetModuleHandleA(crt[1]) == NULL, "alias disappears after unload");
    }
cleanup:
    if (context != INVALID_HANDLE_VALUE) ReleaseActCtx(context);
    DeleteFileA(manifest_path);
    DeleteFileA(cxx);
    for (i = 0; i < 3; ++i) { DeleteFileA(crt[i]); RemoveDirectoryA(dir[i]); }
    RemoveDirectoryA(base);
    printf("BUILTIN_DLL_ALIAS_%s checks=%u failures=%u\n", failures ? "FAIL" : "PASS", checks, failures);
    return failures ? 1 : 0;
}

/* Exercise .x hierarchies around the original and reduced subobject limits.
 * No D3D device or graphics context is needed. Build as PE32 with MinGW.
 */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <initguid.h>
#include <dxfile.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned checks, failures;
static void check(int condition, const char *message, unsigned line)
{
    ++checks;
    if (condition) return;
    ++failures;
    printf("FAIL line %u: %s\n", line, message);
}
#define CHECK(c) check(!!(c), #c, __LINE__)

static int append(char *buffer, size_t capacity, size_t *used, const char *format, ...)
{
    va_list args;
    int count;
    va_start(args, format);
    count = vsnprintf(buffer + *used, capacity - *used, format, args);
    va_end(args);
    if (count < 0 || (size_t)count >= capacity - *used) return 0;
    *used += count;
    return 1;
}

static char *make_hierarchy(unsigned total, DWORD *length)
{
    unsigned groups = (total - 1 + 200) / 201;
    unsigned leaves = total - 1 - groups, id = 1, group, leaf, count;
    size_t capacity = total * 64 + 128, used = 0;
    char *text = malloc(capacity);
    if (!text) return NULL;
#define APPEND(...) do { if (!append(text, capacity, &used, __VA_ARGS__)) goto error; } while (0)
    APPEND("xof 0302txt 0064\nBWNode node_0 { 0;\n");
    for (group = 0; group < groups; ++group)
    {
        APPEND("BWNode node_%u { %u;\n", id, id);
        ++id;
        count = leaves > 200 ? 200 : leaves;
        for (leaf = 0; leaf < count; ++leaf)
        {
            APPEND("BWNode node_%u { %u; }\n", id, id);
            ++id;
        }
        leaves -= count;
        APPEND("}\n");
    }
    APPEND("}\n");
    CHECK(id == total && leaves == 0 && groups <= 200);
    if (id != total || leaves || groups > 200) goto error;
    *length = (DWORD)used;
    return text;
error:
    free(text);
    return NULL;
#undef APPEND
}

static void inspect_node(IDirectXFileData *node, unsigned char *seen, unsigned total, unsigned depth)
{
    IDirectXFileObject *object;
    IDirectXFileData *child;
    DWORD size = 0, *value = NULL;
    HRESULT hr;
    CHECK(depth <= 2);
    if (depth > 2) return;
    hr = IDirectXFileData_GetData(node, NULL, &size, (void **)&value);
    CHECK(hr == S_OK && size == sizeof(*value) && value != NULL);
    if (hr != S_OK || size != sizeof(*value) || !value) return;
    CHECK(*value < total);
    if (*value >= total) return;
    CHECK(!seen[*value]);
    seen[*value] = 1;
    while (1)
    {
        object = NULL;
        hr = IDirectXFileData_GetNextObject(node, &object);
        if (hr == DXFILEERR_NOMOREOBJECTS) break;
        CHECK(hr == S_OK && object != NULL);
        if (hr != S_OK || !object) break;
        child = NULL;
        hr = IDirectXFileObject_QueryInterface(object, &IID_IDirectXFileData, (void **)&child);
        CHECK(hr == S_OK && child != NULL);
        IDirectXFileObject_Release(object);
        if (hr == S_OK && child)
        {
            inspect_node(child, seen, total, depth + 1);
            IDirectXFileData_Release(child);
        }
    }
}

int main(void)
{
    static const unsigned totals[] = {1, 32, 511, 512, 513, 600, 1999, 2000};
    static const char templates[] = "xof 0302txt 0064\n"
        "template BWNode { <fd758ea4-2f9e-4e2e-8f31-107113556015> DWORD value; [...] }\n";
    HRESULT (WINAPI *create_file)(IDirectXFile **);
    HMODULE module = LoadLibraryA("d3dxof.dll");
    IDirectXFile *file = NULL;
    IDirectXFileEnumObject *enumerator;
    IDirectXFileData *root;
    DXFILELOADMEMORY memory;
    char module_path[MAX_PATH];
    unsigned char *seen;
    unsigned test, i, visited;
    HRESULT hr;

    CHECK(module != NULL);
    if (!module) goto done;
    i = GetModuleFileNameA(module, module_path, sizeof(module_path));
    CHECK(i > 0 && i < sizeof(module_path));
    if (i > 0 && i < sizeof(module_path)) printf("XFILE_OBJECTS module=%s\n", module_path);
    create_file = (void *)GetProcAddress(module, "DirectXFileCreate");
    CHECK(create_file != NULL);
    if (!create_file) goto done;
    hr = create_file(&file);
    CHECK(hr == S_OK && file != NULL);
    if (hr != S_OK || !file) goto done;
    hr = IDirectXFile_RegisterTemplates(file, (void *)templates, sizeof(templates) - 1);
    CHECK(hr == S_OK);
    if (hr != S_OK) goto done;
    for (test = 0; test < sizeof(totals) / sizeof(totals[0]); ++test)
    {
        memory.lpMemory = make_hierarchy(totals[test], &memory.dSize);
        seen = calloc(totals[test], 1);
        enumerator = NULL;
        root = NULL;
        CHECK(memory.lpMemory != NULL && seen != NULL);
        if (memory.lpMemory && seen)
        {
            hr = IDirectXFile_CreateEnumObject(file, &memory, DXFILELOAD_FROMMEMORY, &enumerator);
            printf("XFILE_OBJECTS total=%u enum_hr=%#lx\n", totals[test], (unsigned long)hr);
            CHECK(hr == S_OK && enumerator != NULL);
            if (hr == S_OK && enumerator)
            {
                hr = IDirectXFileEnumObject_GetNextDataObject(enumerator, &root);
                printf("XFILE_OBJECTS total=%u root_hr=%#lx\n", totals[test], (unsigned long)hr);
                CHECK(hr == S_OK && root != NULL);
                if (hr == S_OK && root) inspect_node(root, seen, totals[test], 0);
            }
            for (visited = 0, i = 0; i < totals[test]; ++i) visited += !!seen[i];
            printf("XFILE_OBJECTS total=%u visited=%u\n", totals[test], visited);
            CHECK(visited == totals[test]);
        }
        if (root) IDirectXFileData_Release(root);
        if (enumerator) IDirectXFileEnumObject_Release(enumerator);
        free(seen);
        free(memory.lpMemory);
    }
done:
    if (file) CHECK(IDirectXFile_Release(file) == 0);
    if (module) CHECK(FreeLibrary(module));
    printf("Summary: %u passed, %u failed, 0 skipped\n", checks - failures, failures);
    return failures ? 1 : 0;
}

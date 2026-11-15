/* Compile an SDK effect with the selected d3dcompiler_43 implementation.
 * This requires no graphics device. Keep source/bytecode and DLL identities
 * with the result; native precompilation is not a Wine compiler pass.
 */
#define COBJMACROS
#include <windows.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <stdlib.h>

typedef HRESULT (WINAPI *compile_fn)(const void *, SIZE_T, const char *,
        const D3D_SHADER_MACRO *, ID3DInclude *, const char *, const char *,
        UINT, UINT, ID3DBlob **, ID3DBlob **);

int main(int argc, char **argv)
{
    HMODULE module;
    compile_fn compile;
    ID3DBlob *code = NULL, *errors = NULL;
    FILE *file;
    char *source, module_path[MAX_PATH];
    long size;
    HRESULT hr;
    int result = 1;

    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "usage: effect_compile.exe SOURCE [OUTPUT]\n");
        return 2;
    }
    module = LoadLibraryA("d3dcompiler_43.dll");
    if (!module || !(compile = (compile_fn)GetProcAddress(module, "D3DCompile")))
    {
        fprintf(stderr, "D3DCompile unavailable: %lu\n", GetLastError());
        if (module) FreeLibrary(module);
        return 3;
    }
    if (!GetModuleFileNameA(module, module_path, sizeof(module_path)))
        module_path[0] = 0;
    printf("compiler=%s target=fx_2_0 flags=0 source=%s\n", module_path, argv[1]);
    file = fopen(argv[1], "rb");
    if (!file)
    {
        perror(argv[1]);
        FreeLibrary(module);
        return 4;
    }
    if (fseek(file, 0, SEEK_END) || (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET))
    {
        fclose(file);
        FreeLibrary(module);
        return 4;
    }
    source = malloc((size_t)size);
    if (!source || fread(source, 1, size, file) != (size_t)size)
    {
        fclose(file);
        free(source);
        FreeLibrary(module);
        return 4;
    }
    fclose(file);
    hr = compile(source, size, argv[1], NULL, NULL, NULL, "fx_2_0", 0, 0, &code, &errors);
    printf("D3DCompile hr=%08lx bytes=%lu\n", (unsigned long)hr,
            code ? (unsigned long)ID3D10Blob_GetBufferSize(code) : 0);
    if (errors)
    {
        fwrite(ID3D10Blob_GetBufferPointer(errors), 1, ID3D10Blob_GetBufferSize(errors), stdout);
        putchar('\n');
        ID3D10Blob_Release(errors);
    }
    if (SUCCEEDED(hr) && code)
    {
        result = 0;
        if (argc == 3)
        {
            /* Never replace an existing compiled effect silently. */
            HANDLE output = CreateFileA(argv[2], GENERIC_WRITE, 0, NULL, CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL, NULL);
            DWORD written = 0, bytes = (DWORD)ID3D10Blob_GetBufferSize(code);
            if (output == INVALID_HANDLE_VALUE)
            {
                fprintf(stderr, "Cannot create output: %lu\n", GetLastError());
                result = 5;
            }
            else
            {
                if (!WriteFile(output, ID3D10Blob_GetBufferPointer(code), bytes, &written, NULL)
                        || written != bytes) result = 5;
                CloseHandle(output);
            }
        }
    }
    if (code) ID3D10Blob_Release(code);
    free(source);
    FreeLibrary(module);
    return result;
}

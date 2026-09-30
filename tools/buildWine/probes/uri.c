#define COBJMACROS
#include <windows.h>
#include <urlmon.h>
#include <stdio.h>

static const struct
{
    const WCHAR *url;
    DWORD flags;
    const WCHAR *expected;
} tests[] =
{
    {L"file://localhost/c:/test.mp3", 0, L"c:\\test.mp3"},
    {L"file://localhost/c:/test.mp3", Uri_CREATE_FILE_USE_DOS_PATH, L"c:\\test.mp3"},
    {L"file://LOCALHOST/C:/test.mp3", Uri_CREATE_FILE_USE_DOS_PATH, L"C:\\test.mp3"},
    {L"file://localhost/C:/test dir/test.mp3", 0x25, L"C:\\test dir\\test.mp3"},
    {L"file://localhost/C:/test%20dir/test.mp3", Uri_CREATE_FILE_USE_DOS_PATH, L"C:\\test dir\\test.mp3"},
    {L"file:///c:/test.mp3", Uri_CREATE_FILE_USE_DOS_PATH, L"c:\\test.mp3"},
    {L"file://server/share/test.mp3", Uri_CREATE_FILE_USE_DOS_PATH, L"\\\\server\\share\\test.mp3"},
    {L"file://localhost/C:/CINEBENCH_R11/CINEBENCH R11/plugins/bench/res/strings_us/disclaimer.html",
        0x25, L"C:\\CINEBENCH_R11\\CINEBENCH R11\\plugins\\bench\\res\\strings_us\\disclaimer.html"},
};

int main(void)
{
    unsigned int i, failures = 0;
    CoInitialize(NULL);
    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
    {
        WCHAR path[1024] = {0};
        DWORD size = 0;
        IUri *uri = NULL;
        HRESULT hr = CreateUri(tests[i].url, tests[i].flags, 0, &uri);
        if (SUCCEEDED(hr))
        {
            hr = CoInternetParseIUri(uri, PARSE_PATH_FROM_URL, 0, path, 1024, &size, 0);
            IUri_Release(uri);
        }
        if (hr != S_OK || wcscmp(path, tests[i].expected))
        {
            ++failures;
            printf("FAIL %u flags=%#lx hr=%#lx path=[%ls] expected=[%ls]\n",
                i, tests[i].flags, hr, path, tests[i].expected);
        }
        else
            printf("PASS %u flags=%#lx path=[%ls]\n", i, tests[i].flags, path);
    }
    CoUninitialize();
    printf("%u tests, %u failures\n", i, failures);
    return failures != 0;
}

#if !defined (__EMSCRIPTEN__) && !defined (__TEST)

#ifdef U64
#undef U64
#endif

#include "boxedwine.h"
#include <iostream>
#include <fstream>
#include "networkutils.h"

#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "urlmon.lib")
class DownloadProgress : public IBindStatusCallback {
public:
    bool* cancel = nullptr;
    std::function<void(U64 bytesCompleted)> f;

    HRESULT __stdcall QueryInterface(const IID&, void**) {
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef(void) {
        return 1;
    }
    ULONG STDMETHODCALLTYPE Release(void) {
        return 1;
    }
    HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding* pib) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE GetPriority(LONG* pnPriority) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved) {
        return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD* grfBINDF, BINDINFO* pbindinfo) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pformatetc, STGMEDIUM* pstgmed) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown* punk) {
        return E_NOTIMPL;
    }

    virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
    {
        f(ulProgress);

        if (cancel && *cancel) {
            return E_ABORT;
        }
        return S_OK;
    }
};


bool downloadFile(BString url, BString filePath, std::function<void(U64 bytesCompleted)> f, NetworkProxy* proxy, BString& errorMsg, bool* cancel) {
    DownloadProgress progress;
    progress.cancel = cancel;
    progress.f = f;

    HRESULT hr = URLDownloadToFile(0, url.c_str(), filePath.c_str(), 0, static_cast<IBindStatusCallback*>(&progress));
    if (hr != S_OK) {
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        errorMsg = BString::copy(messageBuffer);
        LocalFree(messageBuffer);
    }
    return hr == S_OK;
}

#else
#include <curl/curl.h>

class ProgressData {
public:
    std::function<void(U64 bytesCompleted)> f;
    bool* cancel;
};

static size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

static int xferinfo(void* p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    ProgressData* data = (ProgressData*)p;
    data->f(dlnow);
    if (data->cancel && *data->cancel) {
        return 1;
    }
    return 0;
}

bool downloadFile(BString url, BString filePath, std::function<void(U64 bytesCompleted)> f, NetworkProxy* proxy, BString& errorMsg, bool* cancel) {
    CURL* curl;
    CURLcode res = CURLE_OK;
    ProgressData data;

    curl = curl_easy_init();
    if (curl) {
        data.f = f;
        data.cancel = cancel;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
        /* pass the struct pointer into the xferinfo function */
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &data);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        FILE* file = fopen(filePath.c_str(), "wb");
        if (file) {

            /* write the page body to this file handle */
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

            /* get it! */
            curl_easy_perform(curl);

            /* close the header file */
            fclose(file);
        }

        if (res != CURLE_OK)
            fprintf(stderr, "%s\n", curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return res == CURLE_OK;
}
#endif
#endif
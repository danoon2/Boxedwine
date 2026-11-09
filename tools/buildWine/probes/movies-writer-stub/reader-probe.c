#define COBJMACROS
#include <windows.h>
#include <wmsdk.h>
#include <nserror.h>
#include <stdio.h>
int wmain(int argc,WCHAR **argv)
{
    IWMSyncReader *reader=NULL;
    WCHAR path[MAX_PATH];
    HMODULE core=LoadLibraryA("wmvcore.dll");
    HRESULT (WINAPI *create)(IUnknown *,DWORD,IWMSyncReader **)=core?(void *)GetProcAddress(core,"WMCreateSyncReader"):NULL;
    HRESULT hr=create?create(NULL,0,&reader):E_FAIL;
    printf("WMCreateSyncReader: %08lx\n",hr);
    if (reader) {
        GetFullPathNameW(argc>1?argv[1]:L"data/intro/atvi_logo.wmv",MAX_PATH,path,NULL);
        hr=IWMSyncReader_Open(reader,path);
        printf("Open WMV intro: %08lx\n",hr);
        if (SUCCEEDED(hr)) {
            unsigned samples=0, stream_mask=0;
            unsigned limit=argc>2?10000:32;
            for (unsigned i=0;i<limit;++i) {
                INSSBuffer *sample=NULL;
                QWORD time=0,duration=0;
                DWORD flags=0,output=0,length=0;
                BYTE *bytes=NULL;
                WORD stream=0;
                hr=IWMSyncReader_GetNextSample(reader,0,&sample,&time,&duration,&flags,&output,&stream);
                if (FAILED(hr)) break;
                hr=INSSBuffer_GetBufferAndLength(sample,&bytes,&length);
                if (FAILED(hr)) { INSSBuffer_Release(sample); break; }
                if (length) { ++samples; if (stream<32) stream_mask |= 1u<<stream; }
                INSSBuffer_Release(sample);
            }
            printf("Decoded samples: %u; stream mask: %x; result: %08lx\n",samples,stream_mask,hr);
            if (argc>2 && hr==NS_E_NO_MORE_SAMPLES && samples && (stream_mask & (stream_mask-1))) hr=S_OK;
            else if (samples!=32 || (stream_mask & (stream_mask-1))==0) hr=E_FAIL;
            IWMSyncReader_Close(reader);
        }
        IWMSyncReader_Release(reader);
    }
    return FAILED(hr);
}

#define COBJMACROS
#include <windows.h>
#include <dshow.h>
#include <stdio.h>

int main(void)
{
    const CLSID clsid={0x7c23220e,0x55bb,0x11d3,{0x8b,0x16,0,0xc0,0x4f,0xb6,0xbd,0x3d}};
    IBaseFilter *filter=NULL;
    IUnknown *unknown=NULL, *again=NULL;
    IBaseFilter *roundtrip=NULL;
    IEnumPins *pins=NULL;
    IPin *pin=NULL;
    IGraphBuilder *graph=NULL;
    IFileSinkFilter *sink=NULL;
    FILTER_STATE state=State_Running;
    CLSID actual;
    HRESULT hr;
    int failures=0;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s\n",#c); ++failures; } } while (0)
    CoInitialize(NULL);
    hr=CoCreateInstance(&clsid,NULL,CLSCTX_INPROC_SERVER,&IID_IBaseFilter,(void **)&filter);
    CHECK(hr==S_OK && filter);
    if (filter) {
        CHECK(IBaseFilter_GetClassID(filter,&actual)==S_OK && IsEqualGUID(&actual,&clsid));
        CHECK(IBaseFilter_QueryInterface(filter,&IID_IUnknown,(void **)&unknown)==S_OK && unknown);
        if (unknown) {
            CHECK(IUnknown_QueryInterface(unknown,&IID_IBaseFilter,(void **)&roundtrip)==S_OK && roundtrip);
            if (roundtrip) {
                CHECK(IBaseFilter_QueryInterface(roundtrip,&IID_IUnknown,(void **)&again)==S_OK && again==unknown);
                if (again) IUnknown_Release(again);
                IBaseFilter_Release(roundtrip);
            }
            IUnknown_Release(unknown);
        }
        CHECK(IBaseFilter_EnumPins(filter,&pins)==S_OK && pins);
        if (pins) {
            CHECK(IEnumPins_Next(pins,1,&pin,NULL)==S_FALSE && !pin);
            IEnumPins_Release(pins);
        }
        CHECK(IBaseFilter_QueryInterface(filter,&IID_IFileSinkFilter,(void **)&sink)==E_NOINTERFACE && !sink);
        CHECK(CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,&IID_IGraphBuilder,(void **)&graph)==S_OK);
        if (graph) {
            CHECK(IGraphBuilder_AddFilter(graph,filter,L"Writer probe")==S_OK);
            CHECK(IGraphBuilder_RemoveFilter(graph,filter)==S_OK);
            IGraphBuilder_Release(graph);
        }
        CHECK(IBaseFilter_GetState(filter,0,&state)==S_OK && state==State_Stopped);
        CHECK(FAILED(IBaseFilter_Run(filter,0)));
        CHECK(FAILED(IBaseFilter_Pause(filter)));
        CHECK(IBaseFilter_GetState(filter,0,&state)==S_OK && state==State_Stopped);
        CHECK(IBaseFilter_Release(filter)==0);
    }
    CoUninitialize();
    printf("Writer probe: %d failures\n",failures);
    return failures;
}

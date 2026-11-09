/* Experimental startup-only ASF writer. Never claims to encode or save data. */
#define COBJMACROS
#include <windows.h>
#include <dshow.h>
#include <stdio.h>
#include <stdarg.h>

static const CLSID writer_clsid = {0x7c23220e,0x55bb,0x11d3,{0x8b,0x16,0,0xc0,0x4f,0xb6,0xbd,0x3d}};
static HMODULE module;
static LONG objects, locks;
static void trace(const char *format, ...)
{
    char text[1024]; DWORD count;
    va_list args; va_start(args, format); vsnprintf(text, sizeof(text), format, args); va_end(args);
    HANDLE file = CreateFileA("Z:\\home\\username\\writer-experiment\\stub-calls.txt", FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    if (file != INVALID_HANDLE_VALUE) { WriteFile(file, text, (DWORD)strlen(text), &count, NULL); CloseHandle(file); }
}
static void trace_guid(const char *label, REFGUID g)
{
    trace("%s {%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n", label, g->Data1,g->Data2,g->Data3,
        g->Data4[0],g->Data4[1],g->Data4[2],g->Data4[3],g->Data4[4],g->Data4[5],g->Data4[6],g->Data4[7]);
}
typedef struct writer {
    IBaseFilter filter;
    LONG refs;
    IReferenceClock *clock;
    IFilterGraph *graph; /* weak, as required by QueryFilterInfo */
    WCHAR name[128];
} writer;
static ULONG WINAPI filter_addref(IBaseFilter *iface) { return InterlockedIncrement(&((writer *)iface)->refs); }
static ULONG WINAPI filter_release(IBaseFilter *iface)
{
    writer *w = (writer *)iface; ULONG refs = InterlockedDecrement(&w->refs);
    if (!refs) { trace("Filter destroyed\n"); if (w->clock) IReferenceClock_Release(w->clock); HeapFree(GetProcessHeap(),0,w); InterlockedDecrement(&objects); }
    return refs;
}
static HRESULT WINAPI filter_qi(IBaseFilter *iface, REFIID iid, void **out)
{
    trace_guid("Filter QI",iid);
    if (!out) return E_POINTER;
    *out = NULL;
    if (IsEqualIID(iid,&IID_IUnknown) || IsEqualIID(iid,&IID_IPersist) || IsEqualIID(iid,&IID_IMediaFilter) || IsEqualIID(iid,&IID_IBaseFilter)) {
        *out=iface; filter_addref(iface); return S_OK;
    }
    return E_NOINTERFACE;
}
static HRESULT WINAPI filter_class(IBaseFilter *iface, CLSID *clsid) { trace("GetClassID\n"); if (!clsid) return E_POINTER; *clsid=writer_clsid; return S_OK; }
static HRESULT WINAPI filter_stop(IBaseFilter *iface) { trace("Stop\n"); return S_OK; }
static HRESULT WINAPI filter_pause(IBaseFilter *iface) { trace("Pause: unsupported encoding\n"); return E_NOTIMPL; }
static HRESULT WINAPI filter_run(IBaseFilter *iface, REFERENCE_TIME start) { trace("Run: unsupported encoding\n"); return E_NOTIMPL; }
static HRESULT WINAPI filter_state(IBaseFilter *iface, DWORD timeout, FILTER_STATE *state) { trace("GetState\n"); if (!state) return E_POINTER; *state=State_Stopped; return S_OK; }
static HRESULT WINAPI filter_setclock(IBaseFilter *iface, IReferenceClock *clock)
{
    writer *w=(writer *)iface; trace("SetSyncSource\n"); if (clock) IReferenceClock_AddRef(clock); if (w->clock) IReferenceClock_Release(w->clock); w->clock=clock; return S_OK;
}
static HRESULT WINAPI filter_getclock(IBaseFilter *iface, IReferenceClock **clock)
{
    writer *w=(writer *)iface; trace("GetSyncSource\n"); if (!clock) return E_POINTER; *clock=w->clock; if (*clock) IReferenceClock_AddRef(*clock); return S_OK;
}
static HRESULT WINAPI filter_enumpins(IBaseFilter *iface, IEnumPins **pins) { trace("EnumPins: unsupported encoding\n"); if (!pins) return E_POINTER; *pins=NULL; return E_NOTIMPL; }
static HRESULT WINAPI filter_findpin(IBaseFilter *iface, LPCWSTR id, IPin **pin) { trace("FindPin\n"); if (!pin) return E_POINTER; *pin=NULL; return VFW_E_NOT_FOUND; }
static HRESULT WINAPI filter_info(IBaseFilter *iface, FILTER_INFO *info)
{
    writer *w=(writer *)iface; trace("QueryFilterInfo\n"); if (!info) return E_POINTER;
    lstrcpynW(info->achName,w->name,128); info->pGraph=w->graph; if (info->pGraph) IFilterGraph_AddRef(info->pGraph); return S_OK;
}
static HRESULT WINAPI filter_join(IBaseFilter *iface, IFilterGraph *graph, LPCWSTR name)
{
    writer *w=(writer *)iface; trace("JoinFilterGraph\n"); w->graph=graph; lstrcpynW(w->name,name?name:L"",128); return S_OK;
}
static HRESULT WINAPI filter_vendor(IBaseFilter *iface, LPWSTR *vendor) { trace("QueryVendorInfo\n"); if (!vendor) return E_POINTER; *vendor=NULL; return E_NOTIMPL; }
static IBaseFilterVtbl filter_vtable = { filter_qi,filter_addref,filter_release,filter_class,filter_stop,filter_pause,filter_run,filter_state,filter_setclock,filter_getclock,filter_enumpins,filter_findpin,filter_info,filter_join,filter_vendor };
static ULONG WINAPI factory_addref(IClassFactory *iface) { return 2; }
static ULONG WINAPI factory_release(IClassFactory *iface) { return 1; }
static HRESULT WINAPI factory_qi(IClassFactory *iface, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out=NULL;
    if (IsEqualIID(iid,&IID_IUnknown) || IsEqualIID(iid,&IID_IClassFactory)) { *out=iface; return S_OK; }
    return E_NOINTERFACE;
}
static HRESULT WINAPI factory_create(IClassFactory *iface, IUnknown *outer, REFIID iid, void **out)
{
    writer *w; HRESULT hr; trace_guid("CreateInstance",iid);
    if (!out) return E_POINTER;
    *out=NULL;
    if (outer) return CLASS_E_NOAGGREGATION;
    w=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*w)); if (!w) return E_OUTOFMEMORY;
    w->filter.lpVtbl=&filter_vtable; w->refs=1; InterlockedIncrement(&objects);
    hr=filter_qi(&w->filter,iid,out); filter_release(&w->filter); return hr;
}
static HRESULT WINAPI factory_lock(IClassFactory *iface, BOOL lock) { if (lock) InterlockedIncrement(&locks); else InterlockedDecrement(&locks); return S_OK; }
static IClassFactoryVtbl factory_vtable={factory_qi,factory_addref,factory_release,factory_create,factory_lock};
static IClassFactory factory={&factory_vtable};
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,void *reserved) { if (reason==DLL_PROCESS_ATTACH) { module=instance; DisableThreadLibraryCalls(instance); } return TRUE; }
__declspec(dllexport) HRESULT WINAPI DllGetClassObject(REFCLSID clsid,REFIID iid,void **out)
{
    trace_guid("DllGetClassObject",clsid); if (!out) return E_POINTER; *out=NULL;
    if (!IsEqualCLSID(clsid,&writer_clsid)) return CLASS_E_CLASSNOTAVAILABLE;
    return factory_qi(&factory,iid,out);
}
__declspec(dllexport) HRESULT WINAPI DllCanUnloadNow(void) { return !objects && !locks ? S_OK:S_FALSE; }
__declspec(dllexport) HRESULT WINAPI DllRegisterServer(void)
{
    const char *key="CLSID\\{7c23220e-55bb-11d3-8b16-00c04fb6bd3d}\\InprocServer32";
    char path[MAX_PATH]; HKEY reg; LONG err;
    if (!GetModuleFileNameA(module,path,sizeof(path))) return HRESULT_FROM_WIN32(GetLastError());
    err=RegCreateKeyExA(HKEY_CLASSES_ROOT,key,0,NULL,0,KEY_WRITE,NULL,&reg,NULL); if (err) return HRESULT_FROM_WIN32(err);
    err=RegSetValueExA(reg,NULL,0,REG_SZ,(BYTE *)path,(DWORD)strlen(path)+1);
    if (!err) err=RegSetValueExA(reg,"ThreadingModel",0,REG_SZ,(BYTE *)"Both",5);
    RegCloseKey(reg); return HRESULT_FROM_WIN32(err);
}

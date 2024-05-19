#include "wineboxed.h"
#include "unixlib.h"
#include "wine/debug.h"

// WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

#if BOXED_WINE_VERSION >= 7120 && BOXED_WINE_VERSION < 7220
static unixlib_handle_t x11drv_handle;
#endif

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    BOOL ret = TRUE;

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        //TRACE("\n");
        DisableThreadLibraryCalls(hinst);
#if BOXED_WINE_VERSION >= 7120
        //TRACE("calling unix_init\n");
#if BOXED_WINE_VERSION >= 7220
        __wine_init_unix_call();
        WINE_UNIX_CALL(unix_init, NULL);        
#elif BOXED_WINE_VERSION >= 1
        if (NtQueryVirtualMemory(GetCurrentProcess(), hinst, MemoryWineUnixFuncs, &x11drv_handle, sizeof(x11drv_handle), NULL)) {
            return FALSE;
        }
        __wine_unix_call(x11drv_handle, unix_init, NULL);
#endif
        //TRACE("unix_init returned\n");
#else
        BOXEDDRV_ProcessAttach();
        BOXEDDRV_DisplayDevices_Init(FALSE);
#endif
        break;
    }
    return ret;
}

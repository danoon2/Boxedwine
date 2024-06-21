/*
 * X11DRV display device functions
 *
 * Copyright 2019 Zhiyi Zhang for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#if 0
MAKE_DEP_UNIX
#endif
#if BOXED_WINE_VERSION <= 7110
#define WINE_UNIX_LIB
#endif
// adapted and copies from dlls\winex11.drv\display.c in the Wine project
//#if BOXED_WINE_VERSION >= 7110
//#define WINE_UNIX_LIB
//#endif
#include "config.h"
#include "wineboxed.h"

#if WINE_GDI_DRIVER_VERSION < 71
#include "rpc.h"
#include "winreg.h"
#include "initguid.h"
#include "devguid.h"
#include "devpkey.h"
#include "setupapi.h"
#endif
#define WIN32_NO_STATUS
#include "winternl.h"
#include "wine/debug.h"
#ifdef INCLUDE_UNICODE
#include INCLUDE_UNICODE
#endif

#include "wingdi.h"
#include "wine/gdi_driver.h"
#include "driver.h"

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

#if BOXED_WINE_VERSION < 5000
void BOXEDDRV_DisplayDevices_Init(BOOL force) {
}
#else
#if BOXED_WINE_VERSION >= 4120 && BOXED_WINE_VERSION <= 7050
#define WINE_CDECL
#define GDI_CDECL
#else
#define WINE_CDECL
#define GDI_CDECL
#endif

INT GDI_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap);

#if WINE_GDI_DRIVER_VERSION < 71

struct boxed_gpu
{
    /* ID to uniquely identify a GPU in handler */
    ULONG_PTR id;
    /* Name */
    WCHAR name[128];
    /* PCI ID */
    UINT vendor_id;
    UINT device_id;
    UINT subsys_id;
    UINT revision_id;
    /* Vulkan device UUID */
    GUID vulkan_uuid;
};

struct boxed_adapter
{
    /* ID to uniquely identify an adapter in handler */
    ULONG_PTR id;
    /* as StateFlags in DISPLAY_DEVICE struct */
    DWORD state_flags;
};

struct boxed_monitor
{
    /* Name */
    WCHAR name[128];
    /* RcMonitor in MONITORINFO struct */
    RECT rc_monitor;
    /* RcWork in MONITORINFO struct */
    RECT rc_work;
    /* StateFlags in DISPLAY_DEVICE struct */
    DWORD state_flags;
};

DEFINE_DEVPROPKEY(DEVPROPKEY_GPU_LUID, 0x60b193cb, 0x5276, 0x4d0f, 0x96, 0xfc, 0xf1, 0x73, 0xab, 0xad, 0x3e, 0xc6, 2);
DEFINE_DEVPROPKEY(DEVPROPKEY_MONITOR_GPU_LUID, 0xca085853, 0x16ce, 0x48aa, 0xb1, 0x14, 0xde, 0x9c, 0x72, 0x33, 0x42, 0x23, 1);
DEFINE_DEVPROPKEY(DEVPROPKEY_MONITOR_OUTPUT_ID, 0xca085853, 0x16ce, 0x48aa, 0xb1, 0x14, 0xde, 0x9c, 0x72, 0x33, 0x42, 0x23, 2);

/* Wine specific properties */
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_GPU_VULKAN_UUID, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5c, 2);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_STATEFLAGS, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 2);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_RCMONITOR, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 3);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_RCWORK, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 4);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_ADAPTERNAME, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 5);

static const WCHAR driver_date_dataW[] = { 'D','r','i','v','e','r','D','a','t','e','D','a','t','a',0 };
static const WCHAR driver_dateW[] = { 'D','r','i','v','e','r','D','a','t','e',0 };
static const WCHAR driver_descW[] = { 'D','r','i','v','e','r','D','e','s','c',0 };
static const WCHAR displayW[] = { 'D','I','S','P','L','A','Y',0 };
static const WCHAR pciW[] = { 'P','C','I',0 };
static const WCHAR video_idW[] = { 'V','i','d','e','o','I','D',0 };
static const WCHAR symbolic_link_valueW[] = { 'S','y','m','b','o','l','i','c','L','i','n','k','V','a','l','u','e',0 };
static const WCHAR gpu_idW[] = { 'G','P','U','I','D',0 };
static const WCHAR monitor_id_fmtW[] = { 'M','o','n','i','t','o','r','I','D','%','d',0 };
static const WCHAR adapter_name_fmtW[] = { '\\','\\','.','\\','D','I','S','P','L','A','Y','%','d',0 };
static const WCHAR state_flagsW[] = { 'S','t','a','t','e','F','l','a','g','s',0 };
static const WCHAR guid_fmtW[] = {
    '{','%','0','8','x','-','%','0','4','x','-','%','0','4','x','-','%','0','2','x','%','0','2','x','-',
    '%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x','}',0 };
static const WCHAR gpu_instance_fmtW[] = {
    'P','C','I','\\',
    'V','E','N','_','%','0','4','X','&',
    'D','E','V','_','%','0','4','X','&',
    'S','U','B','S','Y','S','_','%','0','8','X','&',
    'R','E','V','_','%','0','2','X','\\',
    '%','0','8','X',0 };
static const WCHAR gpu_hardware_id_fmtW[] = {
    'P','C','I','\\',
    'V','E','N','_','%','0','4','X','&',
    'D','E','V','_','%','0','4','X','&',
    'S','U','B','S','Y','S','_','0','0','0','0','0','0','0','0','&',
    'R','E','V','_','0','0',0 };
static const WCHAR video_keyW[] = {
    'H','A','R','D','W','A','R','E','\\',
    'D','E','V','I','C','E','M','A','P','\\',
    'V','I','D','E','O',0 };
static const WCHAR adapter_key_fmtW[] = {
    'S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\',
    'V','i','d','e','o','\\',
    '%','s','\\',
    '%','0','4','x',0 };
static const WCHAR device_video_fmtW[] = {
    '\\','D','e','v','i','c','e','\\',
    'V','i','d','e','o','%','d',0 };
static const WCHAR machine_prefixW[] = {
    '\\','R','e','g','i','s','t','r','y','\\',
    'M','a','c','h','i','n','e','\\',0 };
static const WCHAR nt_classW[] = {
    '\\','R','e','g','i','s','t','r','y','\\',
    'M','a','c','h','i','n','e','\\',
    'S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\',
    'C','l','a','s','s','\\',0 };
static const WCHAR monitor_instance_fmtW[] = {
    'D','I','S','P','L','A','Y','\\',
    'D','e','f','a','u','l','t','_','M','o','n','i','t','o','r','\\',
    '%','0','4','X','&','%','0','4','X',0 };
static const WCHAR monitor_hardware_idW[] = {
    'M','O','N','I','T','O','R','\\',
    'D','e','f','a','u','l','t','_','M','o','n','i','t','o','r',0,0 };
static const WCHAR driver_date_fmtW[] = { '%','u','-','%','u','-','%','u',0 };

static HANDLE get_display_device_init_mutex(void)
{
    static const WCHAR init_mutexW[] = { 'd','i','s','p','l','a','y','_','d','e','v','i','c','e','_','i','n','i','t',0 };
    HANDLE mutex = CreateMutexW(NULL, FALSE, init_mutexW);

    WaitForSingleObject(mutex, INFINITE);
    return mutex;
}

static void release_display_device_init_mutex(HANDLE mutex)
{
    ReleaseMutex(mutex);
    CloseHandle(mutex);
}

static void prepare_devices(HKEY video_hkey)
{
    static const BOOL not_present = FALSE;
    SP_DEVINFO_DATA device_data = { sizeof(device_data) };
    HDEVINFO devinfo;
    DWORD i = 0;

    /* Remove all monitors */
    devinfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_MONITOR, displayW, NULL, 0);
    while (SetupDiEnumDeviceInfo(devinfo, i++, &device_data))
    {
        if (!SetupDiRemoveDevice(devinfo, &device_data))
            ERR("Failed to remove monitor\n");
    }
    SetupDiDestroyDeviceInfoList(devinfo);

    /* Clean up old adapter keys for reinitialization */
    RegDeleteTreeW(video_hkey, NULL);

    /* FIXME:
     * Currently SetupDiGetClassDevsW with DIGCF_PRESENT is unsupported, So we need to clean up not present devices in
     * case application uses SetupDiGetClassDevsW to enumerate devices. Wrong devices could exist in registry as a result
     * of prefix copying or having devices unplugged. But then we couldn't simply delete GPUs because we need to retain
     * the same GUID for the same GPU. */
    i = 0;
    devinfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, pciW, NULL, 0);
    while (SetupDiEnumDeviceInfo(devinfo, i++, &device_data))
    {
        if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPKEY_Device_IsPresent, DEVPROP_TYPE_BOOLEAN,
            (const BYTE*)&not_present, sizeof(not_present), 0))
            ERR("Failed to set GPU present property\n");
    }
    SetupDiDestroyDeviceInfoList(devinfo);
}

/* Initialize a GPU instance.
 * Return its GUID string in guid_string, driver value in driver parameter and LUID in gpu_luid */
static BOOL BOXEDDRV_InitGpu(HDEVINFO devinfo, const struct boxed_gpu* gpu, INT gpu_index, WCHAR* guid_string,
    WCHAR* driver, LUID* gpu_luid)
{
    static const BOOL present = TRUE;
    SP_DEVINFO_DATA device_data = { sizeof(device_data) };
    WCHAR instanceW[MAX_PATH];
    DEVPROPTYPE property_type;
    SYSTEMTIME systemtime;
    WCHAR bufferW[1024];
    FILETIME filetime;
    HKEY hkey = NULL;
    GUID guid;
    LUID luid;
    INT written;
    DWORD size;
    BOOL ret = FALSE;

    TRACE("GPU id:0x%s name:%s.\n", wine_dbgstr_longlong(gpu->id), wine_dbgstr_w(gpu->name));

    sprintfW(instanceW, gpu_instance_fmtW, gpu->vendor_id, gpu->device_id, gpu->subsys_id, gpu->revision_id, gpu_index);
    if (!SetupDiOpenDeviceInfoW(devinfo, instanceW, NULL, 0, &device_data))
    {
        SetupDiCreateDeviceInfoW(devinfo, instanceW, &GUID_DEVCLASS_DISPLAY, gpu->name, NULL, 0, &device_data);
        if (!SetupDiRegisterDeviceInfo(devinfo, &device_data, 0, NULL, NULL, NULL))
            goto done;
    }

    /* Write HardwareID registry property, REG_MULTI_SZ */
    written = sprintfW(bufferW, gpu_hardware_id_fmtW, gpu->vendor_id, gpu->device_id);
    bufferW[written + 1] = 0;
    if (!SetupDiSetDeviceRegistryPropertyW(devinfo, &device_data, SPDRP_HARDWAREID, (const BYTE*)bufferW,
        (written + 2) * sizeof(WCHAR)))
        goto done;

    /* Write DEVPKEY_Device_IsPresent property */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPKEY_Device_IsPresent, DEVPROP_TYPE_BOOLEAN,
        (const BYTE*)&present, sizeof(present), 0))
        goto done;

    /* Write DEVPROPKEY_GPU_LUID property */
    if (!SetupDiGetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_GPU_LUID, &property_type,
        (BYTE*)&luid, sizeof(luid), NULL, 0))
    {
        if (!AllocateLocallyUniqueId(&luid))
            goto done;

        if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_GPU_LUID,
            DEVPROP_TYPE_UINT64, (const BYTE*)&luid, sizeof(luid), 0))
            goto done;
    }
    *gpu_luid = luid;
    TRACE("LUID:%08x:%08x.\n", luid.HighPart, luid.LowPart);

    /* Write WINE_DEVPROPKEY_GPU_VULKAN_UUID property */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_GPU_VULKAN_UUID,
        DEVPROP_TYPE_GUID, (const BYTE*)&gpu->vulkan_uuid,
        sizeof(gpu->vulkan_uuid), 0))
        goto done;
    TRACE("Vulkan UUID:%s.\n", wine_dbgstr_guid(&gpu->vulkan_uuid));

    /* Open driver key.
     * This is where HKLM\System\CurrentControlSet\Control\Video\{GPU GUID}\{Adapter Index} links to */
    hkey = SetupDiCreateDevRegKeyW(devinfo, &device_data, DICS_FLAG_GLOBAL, 0, DIREG_DRV, NULL, NULL);

    /* Write DriverDesc value */
    if (RegSetValueExW(hkey, driver_descW, 0, REG_SZ, (const BYTE*)gpu->name,
        (strlenW(gpu->name) + 1) * sizeof(WCHAR)))
        goto done;
    /* Write DriverDateData value, using current time as driver date, needed by Evoland */
    GetSystemTimeAsFileTime(&filetime);
    if (RegSetValueExW(hkey, driver_date_dataW, 0, REG_BINARY, (BYTE*)&filetime, sizeof(filetime)))
        goto done;

    GetSystemTime(&systemtime);
    sprintfW(bufferW, driver_date_fmtW, systemtime.wMonth, systemtime.wDay, systemtime.wYear);
    if (RegSetValueExW(hkey, driver_dateW, 0, REG_SZ, (BYTE*)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
        goto done;

    RegCloseKey(hkey);

    /* Retrieve driver value for adapters */
    if (!SetupDiGetDeviceRegistryPropertyW(devinfo, &device_data, SPDRP_DRIVER, NULL, (BYTE*)bufferW, sizeof(bufferW),
        NULL))
        goto done;
    lstrcpyW(driver, nt_classW);
    lstrcatW(driver, bufferW);

    /* Write GUID in VideoID in .../instance/Device Parameters, reuse the GUID if it's existent */
    hkey = SetupDiCreateDevRegKeyW(devinfo, &device_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, NULL, NULL);

    size = sizeof(bufferW);
    if (RegQueryValueExW(hkey, video_idW, 0, NULL, (BYTE*)bufferW, &size))
    {
        UuidCreate(&guid);
        sprintfW(bufferW, guid_fmtW, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
            guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
        if (RegSetValueExW(hkey, video_idW, 0, REG_SZ, (const BYTE*)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
            goto done;
    }
    lstrcpyW(guid_string, bufferW);

    ret = TRUE;
done:
    RegCloseKey(hkey);
    if (!ret)
        ERR("Failed to initialize GPU\n");
    return ret;
}

static BOOL BOXEDDRV_InitAdapter(HKEY video_hkey, INT video_index, INT gpu_index, INT adapter_index, INT monitor_count,
    const struct boxed_gpu* gpu, const WCHAR* guid_string,
    const WCHAR* gpu_driver, const struct boxed_adapter* adapter)
{
    WCHAR adapter_keyW[MAX_PATH];
    WCHAR key_nameW[MAX_PATH];
    WCHAR bufferW[1024];
    HKEY hkey = NULL;
    BOOL ret = FALSE;
    LSTATUS ls;
    INT i;

    sprintfW(key_nameW, device_video_fmtW, video_index);
    lstrcpyW(bufferW, machine_prefixW);
    sprintfW(adapter_keyW, adapter_key_fmtW, guid_string, adapter_index);
    lstrcatW(bufferW, adapter_keyW);

    /* Write value of \Device\Video? (adapter key) in HKLM\HARDWARE\DEVICEMAP\VIDEO\ */
    if (RegSetValueExW(video_hkey, key_nameW, 0, REG_SZ, (const BYTE*)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
        goto done;

    /* Create HKLM\System\CurrentControlSet\Control\Video\{GPU GUID}\{Adapter Index} link to GPU driver */
    ls = RegCreateKeyExW(HKEY_LOCAL_MACHINE, adapter_keyW, 0, NULL, REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
        KEY_ALL_ACCESS, NULL, &hkey, NULL);
    if (ls == ERROR_ALREADY_EXISTS)
        RegCreateKeyExW(HKEY_LOCAL_MACHINE, adapter_keyW, 0, NULL, REG_OPTION_VOLATILE | REG_OPTION_OPEN_LINK,
            KEY_ALL_ACCESS, NULL, &hkey, NULL);
    if (RegSetValueExW(hkey, symbolic_link_valueW, 0, REG_LINK, (const BYTE*)gpu_driver,
        strlenW(gpu_driver) * sizeof(WCHAR)))
        goto done;
    RegCloseKey(hkey);
    hkey = NULL;

    /* FIXME:
     * Following information is Wine specific, it doesn't really exist on Windows. It is used so that we can
     * implement EnumDisplayDevices etc by querying registry only. This information is most likely reported by the
     * device driver on Windows */
    RegCreateKeyExW(HKEY_CURRENT_CONFIG, adapter_keyW, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hkey, NULL);

    /* Write GPU instance path so that we can find the GPU instance via adapters quickly. Another way is trying to match
     * them via the GUID in Device Parameters/VideoID, but it would require enumerating all GPU instances */
    sprintfW(bufferW, gpu_instance_fmtW, gpu->vendor_id, gpu->device_id, gpu->subsys_id, gpu->revision_id, gpu_index);
    if (RegSetValueExW(hkey, gpu_idW, 0, REG_SZ, (const BYTE*)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
        goto done;

    /* Write all monitor instances paths under this adapter */
    for (i = 0; i < monitor_count; i++)
    {
        sprintfW(key_nameW, monitor_id_fmtW, i);
        sprintfW(bufferW, monitor_instance_fmtW, video_index, i);
        if (RegSetValueExW(hkey, key_nameW, 0, REG_SZ, (const BYTE*)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
            goto done;
    }

    /* Write StateFlags */
    if (RegSetValueExW(hkey, state_flagsW, 0, REG_DWORD, (const BYTE*)&adapter->state_flags,
        sizeof(adapter->state_flags)))
        goto done;

    ret = TRUE;
done:
    RegCloseKey(hkey);
    if (!ret)
        ERR("Failed to initialize adapter\n");
    return ret;
}

static BOOL BOXEDDRV_InitMonitor(HDEVINFO devinfo, const struct boxed_monitor* monitor, int monitor_index,
    int video_index, const LUID* gpu_luid, UINT output_id)
{
    SP_DEVINFO_DATA device_data = { sizeof(SP_DEVINFO_DATA) };
    WCHAR bufferW[MAX_PATH];
    HKEY hkey;
    BOOL ret = FALSE;

    /* Create GUID_DEVCLASS_MONITOR instance */
    sprintfW(bufferW, monitor_instance_fmtW, video_index, monitor_index);
    SetupDiCreateDeviceInfoW(devinfo, bufferW, &GUID_DEVCLASS_MONITOR, monitor->name, NULL, 0, &device_data);
    if (!SetupDiRegisterDeviceInfo(devinfo, &device_data, 0, NULL, NULL, NULL))
        goto done;

    /* Write HardwareID registry property */
    if (!SetupDiSetDeviceRegistryPropertyW(devinfo, &device_data, SPDRP_HARDWAREID,
        (const BYTE*)monitor_hardware_idW, sizeof(monitor_hardware_idW)))
        goto done;

    /* Write DEVPROPKEY_MONITOR_GPU_LUID */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_MONITOR_GPU_LUID,
        DEVPROP_TYPE_INT64, (const BYTE*)gpu_luid, sizeof(*gpu_luid), 0))
        goto done;

    /* Write DEVPROPKEY_MONITOR_OUTPUT_ID */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_MONITOR_OUTPUT_ID,
        DEVPROP_TYPE_UINT32, (const BYTE*)&output_id, sizeof(output_id), 0))
        goto done;

    /* Create driver key */
    hkey = SetupDiCreateDevRegKeyW(devinfo, &device_data, DICS_FLAG_GLOBAL, 0, DIREG_DRV, NULL, NULL);
    RegCloseKey(hkey);

    /* FIXME:
     * Following properties are Wine specific, see comments in X11DRV_InitAdapter for details */
     /* StateFlags */
#if BOXED_WINE_VERSION < 9030
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_STATEFLAGS, DEVPROP_TYPE_UINT32,
        (const BYTE*)&monitor->state_flags, sizeof(monitor->state_flags), 0))
        goto done;
#endif
    /* RcMonitor */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_RCMONITOR, DEVPROP_TYPE_BINARY,
        (const BYTE*)&monitor->rc_monitor, sizeof(monitor->rc_monitor), 0))
        goto done;
    /* RcWork */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_RCWORK, DEVPROP_TYPE_BINARY,
        (const BYTE*)&monitor->rc_work, sizeof(monitor->rc_work), 0))
        goto done;
    /* Adapter name */
    sprintfW(bufferW, adapter_name_fmtW, video_index + 1);
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_ADAPTERNAME, DEVPROP_TYPE_STRING,
        (const BYTE*)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR), 0))
        goto done;

    ret = TRUE;
done:
    if (!ret)
        ERR("Failed to initialize monitor\n");
    return ret;
}

static void cleanup_devices(void)
{
    SP_DEVINFO_DATA device_data = { sizeof(device_data) };
    HDEVINFO devinfo;
    DWORD type;
    DWORD i = 0;
    BOOL present;

    devinfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, pciW, NULL, 0);
    while (SetupDiEnumDeviceInfo(devinfo, i++, &device_data))
    {
        present = FALSE;
        SetupDiGetDevicePropertyW(devinfo, &device_data, &DEVPKEY_Device_IsPresent, &type, (BYTE*)&present,
            sizeof(present), NULL, 0);
        if (!present && !SetupDiRemoveDevice(devinfo, &device_data))
            ERR("Failed to remove GPU\n");
    }
    SetupDiDestroyDeviceInfoList(devinfo);
}

void BOXEDDRV_DisplayDevices_Init(BOOL force)
{
    HANDLE mutex;
    struct boxed_gpu gpus[1];
    struct boxed_adapter adapters[1];
    struct boxed_monitor monitors[1];
    INT gpu_count, adapter_count, monitor_count;
    INT gpu, adapter, monitor;
    HDEVINFO gpu_devinfo = NULL, monitor_devinfo = NULL;
    HKEY video_hkey = NULL;
    INT video_index = 0;
    DWORD disposition = 0;
    WCHAR guidW[40];
    WCHAR driverW[1024];
    LUID gpu_luid;
    UINT output_id = 0;
    static const WCHAR wine_adapterW[] = { 'W','i','n','e',' ','A','d','a','p','t','e','r',0 };
    static const WCHAR generic_nonpnp_monitorW[] = {
        'G','e','n','e','r','i','c',' ',
        'N','o','n','-','P','n','P',' ','M','o','n','i','t','o','r',0 };

    mutex = get_display_device_init_mutex();

    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, video_keyW, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &video_hkey,
        &disposition))
    {
        ERR("Failed to create video device key\n");
        goto done;
    }

    /* Avoid unnecessary reinit */
    if (!force && disposition != REG_CREATED_NEW_KEY)
        goto done;

    prepare_devices(video_hkey);

    gpu_devinfo = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_DISPLAY, NULL);
    monitor_devinfo = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_MONITOR, NULL);

    /* Initialize GPUs */
    // if (!handler->get_gpus(&gpus, &gpu_count))
    //     goto done;    
    lstrcpyW(gpus[0].name, wine_adapterW);
    gpus[0].id = 1;
    gpus[0].vendor_id = 0;
    gpus[0].device_id = 0;
    gpus[0].subsys_id = 0;
    gpus[0].revision_id = 0;
    memset(&gpus[0].vulkan_uuid, 0, sizeof(gpus[0].vulkan_uuid));
#if BOXED_WINE_VERSION >= 9000
    gpus[0].memory_size = 4*1024*1024*1024; // :TODO: get this value
#endif

    gpu_count = 1;
    TRACE("GPU count: %d\n", gpu_count);

    for (gpu = 0; gpu < gpu_count; gpu++)
    {
        if (!BOXEDDRV_InitGpu(gpu_devinfo, &gpus[gpu], gpu, guidW, driverW, &gpu_luid))
            goto done;

        /* Initialize adapters */
        /*
        if (!handler->get_adapters(gpus[gpu].id, &adapters, &adapter_count))
            goto done;
            */
        adapter_count = 1;
        adapters[0].id = 1;
#if BOXED_WINE_VERSION < 9030
        adapters[0].state_flags = DISPLAY_DEVICE_PRIMARY_DEVICE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;
#endif
        TRACE("GPU: %#lx %s, adapter count: %d\n", gpus[gpu].id, wine_dbgstr_w(gpus[gpu].name), adapter_count);

        for (adapter = 0; adapter < adapter_count; adapter++)
        {
            /*
            if (!handler->get_monitors(adapters[adapter].id, &monitors, &monitor_count))
                goto done;
                */
            INT width = boxeddrv_GetDeviceCaps(NULL, DESKTOPHORZRES);
            INT height = boxeddrv_GetDeviceCaps(NULL, DESKTOPVERTRES);
            monitor_count = 1;
#if BOXED_WINE_VERSION < 9030
            monitors[0].state_flags = DISPLAY_DEVICE_ATTACHED | DISPLAY_DEVICE_ACTIVE;
#endif
            SetRect(&monitors[0].rc_monitor, 0, 0, width, height);
            SetRect(&monitors[0].rc_work, 0, 0, width, height);
            lstrcpyW(monitors[0].name, generic_nonpnp_monitorW);
            TRACE("adapter: %#lx, monitor count: %d\n", adapters[adapter].id, monitor_count);

            if (!BOXEDDRV_InitAdapter(video_hkey, video_index, gpu, adapter, monitor_count,
                &gpus[gpu], guidW, driverW, &adapters[adapter]))
                goto done;

            /* Initialize monitors */
            for (monitor = 0; monitor < monitor_count; monitor++)
            {
                TRACE("monitor: %#x %s\n", monitor, wine_dbgstr_w(monitors[monitor].name));
                if (!BOXEDDRV_InitMonitor(monitor_devinfo, &monitors[monitor], monitor, video_index, &gpu_luid, output_id++))
                    goto done;
            }

            // handler->free_monitors(monitors);
            // monitors = NULL;
            video_index++;
        }

        // handler->free_adapters(adapters);
        // adapters = NULL;
    }

done:
    cleanup_devices();
    SetupDiDestroyDeviceInfoList(monitor_devinfo);
    SetupDiDestroyDeviceInfoList(gpu_devinfo);
    RegCloseKey(video_hkey);
    release_display_device_init_mutex(mutex);
    /*
    if (gpus)
        handler->free_gpus(gpus);
    if (adapters)
        handler->free_adapters(adapters);
    if (monitors)
        handler->free_monitors(monitors);
        */
}

void boxedwine_displayChanged() {
}

#else 
static BOOL force_display_devices_refresh;

#if WINE_GDI_DRIVER_VERSION >= 81
BOOL WINE_CDECL boxedwine_UpdateDisplayDevices(const struct gdi_device_manager* device_manager, BOOL force, void* param) {
#elif WINE_GDI_DRIVER_VERSION >= 70
void WINE_CDECL boxedwine_UpdateDisplayDevices(const struct gdi_device_manager* device_manager, BOOL force, void* param) {
#endif
    DWORD len;
    INT width = boxeddrv_GetDeviceCaps(NULL, DESKTOPHORZRES);
    INT height = boxeddrv_GetDeviceCaps(NULL, DESKTOPVERTRES);
    const char* gpuName = "Boxedwine GPU";
#if BOXED_WINE_VERSION < 8020
    const char* monitorName = "Boxedwine Monitor";
#endif
    RECT r = { 0, 0, width, height };
    struct gdi_gpu gdi_gpu =
    {
        .id = 1,
        .vendor_id = 1,
        .device_id = 1,
        .subsys_id = 1,
        .revision_id = 1,
    };
    struct gdi_adapter gdi_adapter =
    {
        .id = 1,
        .state_flags = DISPLAY_DEVICE_PRIMARY_DEVICE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP,
    };
    struct gdi_monitor gdi_monitor =
    {
        .rc_monitor = r,
        .rc_work = r,
#if BOXED_WINE_VERSION < 9030
        .state_flags = DISPLAY_DEVICE_ATTACHED | DISPLAY_DEVICE_ACTIVE,
#endif
    };

    if (!force && !force_display_devices_refresh) {
        TRACE("Not forced %d %d\n", BOXED_WINE_VERSION, WINE_GDI_DRIVER_VERSION);
#if WINE_GDI_DRIVER_VERSION >= 81
        return TRUE;
#else
        return;
#endif
    }
    TRACE("Forced %d %d\n", BOXED_WINE_VERSION, WINE_GDI_DRIVER_VERSION);
    force_display_devices_refresh = FALSE;
        

    RtlUTF8ToUnicodeN(gdi_gpu.name, sizeof(gdi_gpu.name), &len, gpuName, strlen(gpuName));
    device_manager->add_gpu(&gdi_gpu, param);
    device_manager->add_adapter(&gdi_adapter, param);    

#if BOXED_WINE_VERSION < 8020
    RtlUTF8ToUnicodeN(gdi_monitor.name, sizeof(gdi_monitor.name), &len, monitorName, strlen(monitorName));
#endif
    device_manager->add_monitor(&gdi_monitor, param);

#if WINE_GDI_DRIVER_VERSION >= 81 && BOXED_WINE_VERSION >= 7140
    {
        DEVMODEW devMode;
        DEVMODEW curMode;
        DWORD i = 0;
        TRACE("adding modes\n");
        boxeddrv_EnumDisplaySettingsEx(1, -1, &curMode, 2);
        while (boxeddrv_EnumDisplaySettingsEx(0, i, &devMode, 0)) {
            BOOL isCurrent = memcmp(&curMode, &devMode, sizeof(DEVMODEW)) == 0;
            TRACE("mode: %dx%dx%dbpp @%d Hz, cur=%d %sstretched %sinterlaced\n", (int)devMode.dmPelsWidth, (int)devMode.dmPelsHeight,
                (int)devMode.dmBitsPerPel, (int)devMode.dmDisplayFrequency, (int)isCurrent,
                devMode.dmDisplayFixedOutput == DMDFO_STRETCH ? "" : "un",
                devMode.dmDisplayFlags & DM_INTERLACED ? "" : "non-");
#if BOXED_WINE_VERSION >= 8060
            device_manager->add_mode(&devMode, isCurrent, param);
#else
            device_manager->add_mode(&devMode, param);
#endif
            i++;
        }        
    }
#endif
#if WINE_GDI_DRIVER_VERSION >= 81
    return TRUE;
#endif
}

#if BOXED_WINE_VERSION >= 7210
BOOL WINE_CDECL boxeddrv_GetCurrentDisplaySettings(LPCWSTR name, BOOL is_primary, LPDEVMODEW devmode) {
#else
BOOL WINE_CDECL boxeddrv_GetCurrentDisplaySettings(LPCWSTR name, LPDEVMODEW devmode) {
#endif
    return boxeddrv_EnumDisplaySettingsEx(name, ENUM_CURRENT_SETTINGS, devmode, 0);
}

void BOXEDDRV_DisplayDevices_Init(BOOL force) {
    UINT32 num_path, num_mode;

    TRACE("force=%d\n", force);
    if (force) force_display_devices_refresh = TRUE;
    /* trigger refresh in win32u */
    NtUserGetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &num_path, &num_mode);
}
#endif
#endif

// removed in version 73
#if BOXED_WINE_VERSION <= 7110
BOOL WINE_CDECL boxeddrv_EnumDisplayMonitors(HDC hdc, LPRECT rect, MONITORENUMPROC proc, LPARAM lparam) {
    RECT r;
    r.left = 0;
    r.right = internal_GetDeviceCaps(HORZRES);
    r.top = 0;
    r.bottom = internal_GetDeviceCaps(VERTRES);

    TRACE("hdc=%p rect=%s proc=%p lparam=0x%08x\n", hdc, wine_dbgstr_rect(rect), proc, (int)lparam);
    if (hdc) {
        POINT origin;
        RECT limit;
        RECT monrect = r;

        if (!GetDCOrgEx(hdc, &origin)) return FALSE;
        if (GetClipBox(hdc, &limit) == ERROR) return FALSE;

        if (rect && !IntersectRect(&limit, &limit, rect)) return TRUE;

        if (IntersectRect(&monrect, &monrect, &limit)) {
            if (!proc((HMONITOR)1, hdc, &monrect, lparam))
                return FALSE;
        }
    }
    else {
        RECT monrect = r;
        RECT unused;

        if (!rect || IntersectRect(&unused, &monrect, rect)) {
            TRACE("calling proc hdc=%p monrect=%s proc=%p lparam=0x%08x\n", hdc, wine_dbgstr_rect(&monrect), proc, (int)lparam);
            if (!proc((HMONITOR)1, hdc, &monrect, lparam))
                return FALSE;
        }
    }

    return TRUE;
}
#endif
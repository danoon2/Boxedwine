#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <dplay8.h>
#include <dplobby8.h>
#include <stdio.h>
#include <string.h>

static HRESULT WINAPI message_handler(void *context, DWORD message, void *data)
{
    return S_OK;
}

static void get_caps(IDirectPlay8Peer *peer, const char *name, DWORD size, DWORD flags)
{
    DPN_CAPS_EX caps;
    HRESULT hr;
    memset(&caps, 0xcc, sizeof(caps));
    caps.dwSize = size;
    hr = IDirectPlay8Peer_GetCaps(peer, (DPN_CAPS *)&caps, flags);
    printf("GET %s size=%lu flags=%lu hr=%08lx values=", name, size, flags, (unsigned long)hr);
    for (unsigned int i = 0; i < sizeof(caps)/sizeof(DWORD); i++)
        printf("%s%lu", i ? "," : "", ((DWORD *)&caps)[i]);
    puts("");
}

static void set_caps(IDirectPlay8Peer *peer, const char *name, DPN_CAPS_EX *caps, DWORD flags)
{
    HRESULT hr = IDirectPlay8Peer_SetCaps(peer, (DPN_CAPS *)caps, flags);
    printf("SET %s hr=%08lx\n", name, (unsigned long)hr);
    get_caps(peer, name, sizeof(DPN_CAPS_EX), 0);
}

int main(int argc, char **argv)
{
    IDirectPlay8Peer *peer = NULL;
    DPN_CAPS_EX caps;
    HRESULT hr;
    if (argc > 1 && !freopen(argv[1], "w", stdout)) return 3;
    setbuf(stdout, NULL);
    CoInitialize(NULL);
    hr = CoCreateInstance(&CLSID_DirectPlay8Peer, NULL, CLSCTX_INPROC_SERVER,
            &IID_IDirectPlay8Peer, (void **)&peer);
    printf("CREATE %08lx\n", (unsigned long)hr);
    if (FAILED(hr)) return 2;
    get_caps(peer, "before-init", sizeof(DPN_CAPS), 0);
    get_caps(peer, "before-init-bad-flags", sizeof(DPN_CAPS), 1);
    get_caps(peer, "before-init-bad-size", 0, 0);
    printf("GET before-init-null hr=%08lx\n", (unsigned long)IDirectPlay8Peer_GetCaps(peer, NULL, 0));
    hr = IDirectPlay8Peer_Initialize(peer, NULL, message_handler, 0);
    printf("INIT %08lx\n", (unsigned long)hr);
    get_caps(peer, "default", sizeof(DPN_CAPS), 0);
    get_caps(peer, "default-ex", sizeof(DPN_CAPS_EX), 0);
    get_caps(peer, "bad-flags", sizeof(DPN_CAPS), 1);
    get_caps(peer, "zero-size", 0, 0);
    get_caps(peer, "short-size", sizeof(DPN_CAPS)-1, 0);
    get_caps(peer, "long-size", sizeof(DPN_CAPS)+1, 0);
    printf("GET null hr=%08lx\n", (unsigned long)IDirectPlay8Peer_GetCaps(peer, NULL, 0));
    memset(&caps, 0, sizeof(caps));
    caps.dwSize = sizeof(DPN_CAPS_EX);
    hr = IDirectPlay8Peer_GetCaps(peer, (DPN_CAPS *)&caps, 0);
    if (SUCCEEDED(hr)) {
        DPN_CAPS_EX changed = caps;
        changed.dwSize = sizeof(DPN_CAPS);
        changed.dwConnectTimeout = 1234;
        changed.dwConnectRetries = 7;
        changed.dwTimeoutUntilKeepAlive = 5678;
        set_caps(peer, "roundtrip", &changed, 0);
        changed.dwSize = sizeof(DPN_CAPS_EX);
        changed.dwConnectTimeout = 4321;
        set_caps(peer, "roundtrip-ex", &changed, 0);
        changed.dwMaxRecvMsgSize = 33333;
        changed.dwNumSendRetries = 12;
        changed.dwMaxSendRetryInterval = 6000;
        changed.dwDropThresholdRate = 9;
        changed.dwThrottleRate = 30;
        changed.dwNumHardDisconnectSends = 5;
        changed.dwMaxHardDisconnectPeriod = 700;
        set_caps(peer, "extended-fields", &changed, 0);
        set_caps(peer, "bad-flags", &caps, 1);
        changed = caps;
        changed.dwFlags = 1;
        set_caps(peer, "reserved-flags", &changed, 0);
        memset(&changed, 0, sizeof(changed));
        changed.dwSize = sizeof(DPN_CAPS);
        set_caps(peer, "zero-values", &changed, 0);
        changed.dwSize = 0;
        set_caps(peer, "zero-size", &changed, 0);
        printf("SET null hr=%08lx\n", (unsigned long)IDirectPlay8Peer_SetCaps(peer, NULL, 0));
    }
    printf("CLOSE %08lx\n", (unsigned long)IDirectPlay8Peer_Close(peer, 0));
    get_caps(peer, "after-close", sizeof(DPN_CAPS), 0);
    hr = IDirectPlay8Peer_Initialize(peer, NULL, message_handler, 0);
    printf("REINIT %08lx\n", (unsigned long)hr);
    get_caps(peer, "after-reinit", sizeof(DPN_CAPS_EX), 0);
    IDirectPlay8Peer_Close(peer, 0);
    IDirectPlay8Peer_Release(peer);
    {
        IDirectPlay8LobbiedApplication *lobby = NULL;
        DPNHANDLE connection = 0xcccccccc;
        DWORD size = 0;
        DPL_CONNECTION_SETTINGS settings;
        hr = CoCreateInstance(&CLSID_DirectPlay8LobbiedApplication, NULL, CLSCTX_INPROC_SERVER,
                &IID_IDirectPlay8LobbiedApplication, (void **)&lobby);
        printf("LOBBY create=%08lx\n", (unsigned long)hr);
        if (SUCCEEDED(hr)) {
            hr = IDirectPlay8LobbiedApplication_GetConnectionSettings(lobby, 0, NULL, &size, 0);
            printf("LOBBY before-init=%08lx size=%lu\n", (unsigned long)hr, size);
            hr = IDirectPlay8LobbiedApplication_Initialize(lobby, NULL, message_handler, &connection, 1);
            printf("LOBBY init=%08lx connection=%08lx\n", (unsigned long)hr, connection);
            hr = IDirectPlay8LobbiedApplication_GetConnectionSettings(lobby, connection, NULL, &size, 0);
            printf("LOBBY get=%08lx size=%lu\n", (unsigned long)hr, size);
            hr = IDirectPlay8LobbiedApplication_GetConnectionSettings(lobby, 1234, NULL, &size, 0);
            printf("LOBBY bad-handle=%08lx size=%lu\n", (unsigned long)hr, size);
            hr = IDirectPlay8LobbiedApplication_GetConnectionSettings(lobby, 0, NULL, NULL, 0);
            printf("LOBBY null-size=%08lx\n", (unsigned long)hr);
            hr = IDirectPlay8LobbiedApplication_GetConnectionSettings(lobby, 0, NULL, &size, 1);
            printf("LOBBY bad-flags=%08lx\n", (unsigned long)hr);
            memset(&settings, 0xcc, sizeof(settings));
            size = sizeof(settings);
            hr = IDirectPlay8LobbiedApplication_GetConnectionSettings(lobby, 0, &settings, &size, 0);
            printf("LOBBY buffer=%08lx size=%lu first=%08lx\n", (unsigned long)hr, size, settings.dwSize);
            IDirectPlay8LobbiedApplication_Close(lobby, 0);
            IDirectPlay8LobbiedApplication_Release(lobby);
        }
    }
    CoUninitialize();
    return 0;
}

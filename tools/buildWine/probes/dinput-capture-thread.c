/* Independent reproduction of cross-thread mouse polling with a blocked window
 * thread. See dinput-capture-thread.md for the XTest runner and build commands.
 * SendInput alone cannot exercise the X11 grab connection which caused the bug. */
#define COBJMACROS
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0700
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dinput.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static IDirectInputDeviceA *mouse;
static unsigned button;
static HANDLE notification;
static unsigned failures;

static void check_buffer( DWORD expected )
{
    DIDEVICEOBJECTDATA data[64];
    DWORD count=64, i, matches=0;
    HRESULT hr=IDirectInputDevice_GetDeviceData(mouse,sizeof(data[0]),data,&count,0);
    if (FAILED(hr)) { printf("BUFFER_ERROR %08lx\n",hr); ++failures; return; }
    for(i=0;i<count;i++) if(data[i].dwOfs==DIMOFS_BUTTON0+button) {
        printf("BUTTON_EVENT offset=%lu data=%lx seq=%lu\n",data[i].dwOfs,data[i].dwData,data[i].dwSequence);
        if(data[i].dwData!=expected) ++failures;
        ++matches;
    }
    if(matches!=1) { printf("BUTTON_EVENT_COUNT expected=1 got=%lu\n",matches); ++failures; }
}
static CRITICAL_SECTION game_lock;
static volatile LONG phase;
static BOOL pump;
static DWORD start;
static int result = 2;

static void pump_messages(DWORD duration)
{
    DWORD now = GetTickCount();
    MSG msg;
    do {
        while (PeekMessageA(&msg,NULL,0,0,PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(1);
    } while (GetTickCount()-now < duration);
}

static void note(const char *text)
{
    printf("%lu tid=%04lx %s\n", GetTickCount()-start, GetCurrentThreadId(), text);
    fflush(stdout);
}

static DWORD WINAPI poll_mouse(void *arg)
{
    DIMOUSESTATE2 state = {0};
    DWORD deadline = GetTickCount()+45000, polls = 0;
    LONG move_x=0, move_y=0, wheel=0;
    HRESULT hr;
    note("POLL_READY");
    while (GetTickCount()<deadline) {
        hr=IDirectInputDevice_GetDeviceState(mouse,sizeof(state),&state);
        if (FAILED(hr)) { printf("GETSTATE_ERROR %08lx\n",hr); break; }
        if (state.rgbButtons[button]&0x80) {
            EnterCriticalSection(&game_lock);
            check_buffer(0x80);
            ResetEvent(notification);
            note("DOWN_SEEN");
            InterlockedExchange(&phase,1);
            /* Give the window thread time to enter the application lock. */
            Sleep(100);
            deadline=GetTickCount()+3000;
            do {
                hr=IDirectInputDevice_GetDeviceState(mouse,sizeof(state),&state);
                if (SUCCEEDED(hr)) { move_x+=state.lX; move_y+=state.lY; wheel+=state.lZ; }
                ++polls;
                if (FAILED(hr) || !(state.rgbButtons[button]&0x80)) break;
                Sleep(1);
            } while (GetTickCount()<deadline);
            result=SUCCEEDED(hr) && !(state.rgbButtons[button]&0x80) ? 0 : 1;
            printf("%lu RELEASE_WHILE_UI_%s %s hr=%08lx button=%02x polls=%lu\n",
                   GetTickCount()-start,pump?"PUMPING":"BLOCKED",result?"FAIL":"PASS",
                   hr,state.rgbButtons[button],polls);
            fflush(stdout);
            if (!result) {
                check_buffer(0);
                if(WaitForSingleObject(notification,0)!=WAIT_OBJECT_0) {
                    note("RELEASE_NOTIFICATION_MISSING"); ++failures;
                }
                printf("MOTION x=%ld y=%ld wheel=%ld\n",move_x,move_y,wheel);
                if(GetEnvironmentVariableA("PROBE_CHECK_MOTION",NULL,0) &&
                   (move_x!=17 || move_y!=-11 || wheel!=120)) ++failures;
            }
            InterlockedExchange(&phase,2);
            LeaveCriticalSection(&game_lock);
            Sleep(500);
            hr=IDirectInputDevice_GetDeviceState(mouse,sizeof(state),&state);
            printf("AFTER_UI_RESUMES hr=%08lx button=%02x\n",hr,state.rgbButtons[button]);
            InterlockedExchange(&phase,3);
            return 0;
        }
        Sleep(1);
    }
    note("NO_BUTTON_DOWN");
    InterlockedExchange(&phase,3);
    return 0;
}

static LRESULT CALLBACK wndproc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    if (msg==WM_CLOSE) { InterlockedExchange(&phase,3); return 0; }
    if (msg==WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc=BeginPaint(hwnd,&ps);
        TextOutA(dc,20,30,"DirectInput 7 thread probe: drag inside this window",50);
        TextOutA(dc,20,60,"The test exits automatically after the button release.",53);
        EndPaint(hwnd,&ps);
        return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}

int main(int argc,char **argv)
{
    HINSTANCE instance=GetModuleHandleA(NULL);
#if DIRECTINPUT_VERSION >= 0x0800
    IDirectInput8A *di=NULL;
#else
    IDirectInputA *di=NULL;
#endif
    DIPROPDWORD buffer_size={{sizeof(buffer_size),sizeof(DIPROPHEADER),0,DIPH_DEVICE},64};
    WNDCLASSA cls={0};
    HWND hwnd;
    MSG msg;
    HANDLE worker;
    HRESULT hr;
    DWORD t;
    BOOL blocked=FALSE;
    const char *(__cdecl *wine_version)(void)=(void *)GetProcAddress(GetModuleHandleA("ntdll.dll"),"wine_get_version");
    setvbuf(stdout,NULL,_IONBF,0);
    start=GetTickCount();
    pump=argc>1 && !strcmp(argv[1],"--pump");
    if(argc>2) button=atoi(argv[2]);
    if(button>4) return 2;
    printf("INPUT_VERSION %x BUTTON %u\n",DIRECTINPUT_VERSION,button);
    printf("RUNTIME %s mode=%s\n",wine_version?wine_version():"Windows",pump?"pump":"block");
    InitializeCriticalSection(&game_lock);
    cls.lpfnWndProc=wndproc;
    cls.hInstance=instance;
    cls.lpszClassName="DInputThreadProbe";
    cls.hCursor=LoadCursorA(NULL,IDC_ARROW);
    cls.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    if (!RegisterClassA(&cls)) return 2;
    hwnd=CreateWindowA(cls.lpszClassName,"DirectInput 7 thread probe",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                      100,100,600,400,NULL,NULL,instance,NULL);
    if (!hwnd) return 2;
    ShowWindow(hwnd,SW_SHOWNORMAL);
    ShowWindow(hwnd,SW_SHOW);
    SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
    note("WINDOW_READY");
    /* Settle foreground and clipping messages before acquiring on this thread. */
    for(t=GetTickCount();GetTickCount()-t<500;Sleep(1))
        while(PeekMessageA(&msg,NULL,0,0,PM_REMOVE)) {TranslateMessage(&msg);DispatchMessageA(&msg);}
    notification=CreateEventA(NULL,FALSE,FALSE,NULL);
    if (FAILED(hr=
#if DIRECTINPUT_VERSION >= 0x0800
        DirectInput8Create(instance,DIRECTINPUT_VERSION,&IID_IDirectInput8A,(void **)&di,NULL)
#else
        DirectInputCreateA(instance,DIRECTINPUT_VERSION,&di,NULL)
#endif
        ) ||
        FAILED(hr=
#if DIRECTINPUT_VERSION >= 0x0800
        IDirectInput8_CreateDevice(di,&GUID_SysMouse,(IDirectInputDevice8A **)&mouse,NULL)
#else
        IDirectInput_CreateDevice(di,&GUID_SysMouse,&mouse,NULL)
#endif
        ) ||
        FAILED(hr=IDirectInputDevice_SetDataFormat(mouse,&c_dfDIMouse2)) ||
        FAILED(hr=IDirectInputDevice_SetCooperativeLevel(mouse,hwnd,DISCL_EXCLUSIVE|DISCL_FOREGROUND))) {
        printf("SETUP_ERROR %08lx\n",hr);return 2;
    }
    if (!notification ||
        FAILED(hr=IDirectInputDevice_SetProperty(mouse,DIPROP_BUFFERSIZE,&buffer_size.diph)) ||
        FAILED(hr=IDirectInputDevice_SetEventNotification(mouse,notification))) {
        printf("BUFFER_SETUP_ERROR %08lx\n",hr);return 2;
    }
    for(t=GetTickCount();;) {
        hr=IDirectInputDevice_Acquire(mouse);
        if(SUCCEEDED(hr)) break;
        if(GetTickCount()-t>30000) {printf("ACQUIRE_ERROR %08lx\n",hr);return 2;}
        while(PeekMessageA(&msg,NULL,0,0,PM_REMOVE)) {TranslateMessage(&msg);DispatchMessageA(&msg);}
        Sleep(100);
    }
    if (argc>3 && !strcmp(argv[3],"--lifecycle")) {
        unsigned i;
        DIMOUSESTATE2 state;
        POINT pos;
        RECT small={200,200,400,400};
        HWND other;

        for (i=0;i<16;i++) {
            if (FAILED(IDirectInputDevice_Unacquire(mouse))) ++failures;
            pump_messages(20);
            if (FAILED(IDirectInputDevice_Acquire(mouse))) ++failures;
            pump_messages(20);
        }
        ClipCursor(&small);
        pump_messages(100);
        SetCursorPos(700,600);
        pump_messages(100);
        GetCursorPos(&pos);
        if (pos.x<200 || pos.x>=400 || pos.y<200 || pos.y>=400) ++failures;
        IDirectInputDevice_Unacquire(mouse);
        ClipCursor(NULL);
        pump_messages(100);
        SetCursorPos(700,600);
        pump_messages(100);
        GetCursorPos(&pos);
        if (pos.x!=700 || pos.y!=600) ++failures;
        IDirectInputDevice_Acquire(mouse);
        pump_messages(100);

        other=CreateWindowA(cls.lpszClassName,"DirectInput focus target",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                            750,20,220,180,NULL,NULL,instance,NULL);
        if (!other) return 2;
        SetForegroundWindow(other);
        note("FOCUS_LOSS_READY");
        for (t=GetTickCount();GetTickCount()-t<5000;) {
            pump_messages(10);
            hr=IDirectInputDevice_GetDeviceState(mouse,sizeof(state),&state);
            if (hr==DIERR_NOTACQUIRED || hr==DIERR_INPUTLOST) break;
        }
        if (hr!=DIERR_NOTACQUIRED && hr!=DIERR_INPUTLOST) ++failures;
        SetForegroundWindow(hwnd);
        note("FOCUS_REGAIN_READY");
        for (t=GetTickCount();GetTickCount()-t<5000;) {
            pump_messages(10);
            if (GetForegroundWindow()!=hwnd) continue;
            hr=IDirectInputDevice_Acquire(mouse);
            if (SUCCEEDED(hr)) break;
        }
        if (FAILED(hr)) ++failures;
        DestroyWindow(other);
        pump_messages(100);
        printf("LIFECYCLE failures=%u\n",failures);
    }
    worker=CreateThread(NULL,0,poll_mouse,NULL,0,NULL);
    if (!worker) return 2;
    while(InterlockedCompareExchange(&phase,0,0)!=3) {
        if (!pump && !blocked && InterlockedCompareExchange(&phase,0,0)==1) {
            blocked=TRUE;
            note("UI_BLOCK");
            EnterCriticalSection(&game_lock);
            LeaveCriticalSection(&game_lock);
            note("UI_RESUME");
        }
        while(PeekMessageA(&msg,NULL,0,0,PM_REMOVE)) {TranslateMessage(&msg);DispatchMessageA(&msg);}
        Sleep(1);
    }
    if (WaitForSingleObject(worker,5000)!=WAIT_OBJECT_0) return 2;
    CloseHandle(worker);
    IDirectInputDevice_Unacquire(mouse);
    IDirectInputDevice_Release(mouse);
#if DIRECTINPUT_VERSION >= 0x0800
    IDirectInput8_Release(di);
#else
    IDirectInput_Release(di);
#endif
    CloseHandle(notification);
    DestroyWindow(hwnd);
    DeleteCriticalSection(&game_lock);
    note("DONE");
    printf("REGRESSION_RESULT %s failures=%u\n",result||failures?"FAIL":"PASS",failures);
    return result||failures;
}

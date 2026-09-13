#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Exercise the public Glide ABI through dynamic exports; no vendor SDK needed.
typedef void (WINAPI *VoidFn)(void);
typedef void (WINAPI *VersionFn)(char *);
typedef unsigned (WINAPI *OpenFn)(unsigned, unsigned, unsigned, unsigned, unsigned, int, int);
typedef void (WINAPI *ClearFn)(unsigned, unsigned char, unsigned short);
typedef void (WINAPI *SwapFn)(int);
typedef void (WINAPI *Close3Fn)(unsigned);
static FARPROC symbol(HMODULE dll, const char *name) {
  FARPROC f=GetProcAddress(dll,name);
  if(!f){printf("MISSING %s error=%lu\n", name, GetLastError());exit(3);} return f;
}
int main(int argc,char **argv) {
  setvbuf(stdout,NULL,_IONBF,0);
  const char *name=argc>1?argv[1]:"glide2x.dll";
  printf("LOAD %s\n",name);
  HMODULE dll=LoadLibraryA(name);
  if(!dll){printf("LOAD_FAILED error=%lu\n",GetLastError());return 2;}
  printf("LOADED\n");
  if(argc>2 && !strcmp(argv[2],"load-only")){FreeLibrary(dll);puts("PASS_LOAD_ONLY");return 0;}
  ((VoidFn)symbol(dll,"grGlideInit"))(); puts("INITIALIZED");
  if(strstr(name,"2x")){char version[128]={0};((VersionFn)symbol(dll,"grGlideGetVersion"))(version);printf("VERSION %s\n",version);}
  WNDCLASSA wc={0};wc.lpfnWndProc=DefWindowProcA;wc.hInstance=GetModuleHandleA(NULL);wc.lpszClassName="BoxedwineGlideProbe";
  RegisterClassA(&wc);RECT rect={0,0,640,480};AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
  HWND window=CreateWindowA(wc.lpszClassName,"Glide diagnostic",WS_OVERLAPPEDWINDOW,20,20,rect.right-rect.left,rect.bottom-rect.top,NULL,NULL,wc.hInstance,NULL);
  if(!window){printf("WINDOW_FAILED error=%lu\n",GetLastError());return 5;}
  ShowWindow(window,SW_SHOW);UpdateWindow(window);
  typedef void (WINAPI *ConfigFn)(unsigned);
  ConfigFn configure=(ConfigFn)GetProcAddress(dll,"setConfig");if(configure)configure(1);
  unsigned context=((OpenFn)symbol(dll,"grSstWinOpen"))((unsigned)(uintptr_t)window,7,0,1,0,2,1);
  printf("CONTEXT %u\n",context); if(!context)return 4;
  ClearFn clear=(ClearFn)symbol(dll,"grBufferClear"); SwapFn swap=(SwapFn)symbol(dll,"grBufferSwap");
  DWORD start=GetTickCount();unsigned frames=0, lastPhase=~0u;MSG msg;
  while(GetTickCount()-start<(argc>3?(unsigned)atoi(argv[3]):12000)){
    while(PeekMessageA(&msg,0,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageA(&msg);}
    // ABGR: a blue field, then green, then red, four seconds each.
    unsigned phase=((GetTickCount()-start)/4000)%3;
    if(phase!=lastPhase){printf("PHASE %u frames=%u\n",phase,frames);lastPhase=phase;}
    if(frames<3)printf("CLEAR_BEGIN %u\n",frames);
    clear(phase==0?0x00c06020:phase==1?0x0040b040:0x003040c0,255,65535);
    if(frames<3)printf("SWAP_BEGIN %u\n",frames);
    swap(1);
    if(frames<3)printf("SWAP_END %u\n",frames);
    ++frames;Sleep(16);
  }
  if(strstr(name,"3x"))((Close3Fn)symbol(dll,"grSstWinClose"))(context);
  else ((VoidFn)symbol(dll,"grSstWinClose"))();
  ((VoidFn)symbol(dll,"grGlideShutdown"))();FreeLibrary(dll);
  DestroyWindow(window);printf("PASS frames=%u\n",frames);return 0;
}

"""Load only original dialog resources and inspect native Win32 Enter handling."""
import argparse
import ctypes as c
from ctypes import wintypes as w
from pathlib import Path
import hashlib
import json

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('executable', type=Path)
parser.add_argument('--output', type=Path, required=True)
args=parser.parse_args()
exe=args.executable.resolve()
user=c.WinDLL('user32',use_last_error=True)
kernel=c.WinDLL('kernel32',use_last_error=True)
LPARAM=c.c_ssize_t; WPARAM=c.c_size_t; LRESULT=c.c_ssize_t
PROC=c.WINFUNCTYPE(LRESULT,w.HWND,w.UINT,WPARAM,LPARAM)
class MSG(c.Structure):
    _fields_=[('hwnd',w.HWND),('message',w.UINT),('wParam',WPARAM),('lParam',LPARAM),('time',w.DWORD),('pt',w.POINT),('private',w.DWORD)]
def api(dll,name,args,result):
    fn=getattr(dll,name);fn.argtypes=args;fn.restype=result;return fn
load=api(kernel,'LoadLibraryExW',[w.LPCWSTR,w.HANDLE,w.DWORD],w.HMODULE)
find=api(kernel,'FindResourceW',[w.HMODULE,w.LPCWSTR,w.LPCWSTR],w.HANDLE)
loadres=api(kernel,'LoadResource',[w.HMODULE,w.HANDLE],w.HANDLE)
lock=api(kernel,'LockResource',[w.HANDLE],c.c_void_p)
size=api(kernel,'SizeofResource',[w.HMODULE,w.HANDLE],w.DWORD)
create=api(user,'CreateDialogIndirectParamW',[w.HMODULE,c.c_void_p,w.HWND,PROC,LPARAM],w.HWND)
send=api(user,'SendMessageW',[w.HWND,w.UINT,WPARAM,LPARAM],LRESULT)
get=api(user,'GetDlgItem',[w.HWND,c.c_int],w.HWND)
focus=api(user,'GetFocus',[],w.HWND)
ctrlid=api(user,'GetDlgCtrlID',[w.HWND],c.c_int)
isdialog=api(user,'IsDialogMessageW',[w.HWND,c.POINTER(MSG)],w.BOOL)
destroy=api(user,'DestroyWindow',[w.HWND],w.BOOL)
visible=api(user,'IsWindowVisible',[w.HWND],w.BOOL)
module=load(str(exe),None,2) # LOAD_LIBRARY_AS_DATAFILE: no game code or imports execute.
assert module,c.get_last_error()
resource=find(module,c.cast(c.c_void_p(110),w.LPCWSTR),c.cast(c.c_void_p(5),w.LPCWSTR))
assert resource,c.get_last_error()
pointer=lock(loadres(module,resource))
raw=c.string_at(pointer,size(module,resource))
# Resource 110 in the playable demo must remain hidden; do not show a different resource.
extended=int.from_bytes(raw[2:4], 'little')==0xffff
style=int.from_bytes(raw[12:16] if extended else raw[:4], 'little')
assert not style & 0x10000000, 'Dialog resource requests a visible window'
events=[]
@PROC
def callback(hwnd,message,wp,lp):
    if message==0x110:return 1 # Use the dialog manager's normal initial focus.
    if message==0x111:
        events.append(dict(message='WM_COMMAND',id=wp&0xffff,notification=wp>>16))
        return 1 # Keep the diagnostic window alive after the command.
    return 0
rows=[]
for mode in ('initial','explicit-ok-focus','explicit-cancel-focus'):
    hwnd=create(module,pointer,None,callback,0)
    assert hwnd,c.get_last_error()
    try:
        assert not visible(hwnd),'This diagnostic must remain hidden'
        if mode!='initial':send(hwnd,0x28,get(hwnd,1 if mode=='explicit-ok-focus' else 2),1) # WM_NEXTDLGCTL
        before=dict(default_button=send(hwnd,0x400,0,0)&0xffff,focused_control=ctrlid(focus()) if focus() else None)
        start=len(events)
        msg=MSG(hwnd=focus() or hwnd,message=0x100,wParam=13,lParam=1|(0x1c<<16))
        handled=bool(isdialog(hwnd,c.byref(msg)))
        rows.append(dict(mode=mode,before=before,handled=handled,commands=events[start:]))
    finally:destroy(hwnd)
record=dict(scope='Native Win32 resource/dialog control; original game code not executed and window never shown',
    executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),resource_sha256=hashlib.sha256(raw).hexdigest(),rows=rows)
record['passed'] = [row['commands'] for row in rows] == [[dict(message='WM_COMMAND',id=value,notification=0)] for value in (2,1,2)]
record['source_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
with args.output.open('x') as stream:json.dump(record,stream,indent=2);stream.write('\n')
print(json.dumps(record))

raise SystemExit(0 if record['passed'] else 1)

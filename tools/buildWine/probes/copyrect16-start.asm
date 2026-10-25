; Minimal Win16 startup. Use KERNEL's DOS3CALL for process exit so the probe
; does not depend on the compiler CRT's direct DOS interrupts or initialization.
.286
.model large
extrn INITTASK:far
extrn INITAPP:far
extrn DOS3CALL:far
extrn WINMAIN:far
.stack 8192
.data
dw 8 dup (0) ; Win16 instance data reserved at the start of DGROUP.
.code
public _wstart_
_wstart_:
    call INITTASK
    or ax, ax
    jz finished
    push di                 ; hInstance
    push si                 ; hPrevInstance
    push es                 ; lpCmdLine selector
    push bx                 ; lpCmdLine offset
    push dx                 ; nCmdShow
    push di
    call INITAPP
    call WINMAIN
finished:
    mov ah, 4ch
    call DOS3CALL
    int 3
end _wstart_

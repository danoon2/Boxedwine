; Freestanding Linux i386 benchmark, assembled with nasm -f bin.
; Separate RX/RW segments also work on hosts with 16 KiB pages.
BITS 32
ORG 0x08048000
elf:
    db 0x7f, 'ELF', 1, 1, 1, 0
    times 8 db 0
    dw 2, 3
    dd 1, start, phdr - $$, 0, 0
    dw 52, 32, 2, 0, 0, 0
phdr:
    dd 1, 0, $$, $$, data_start - $$, data_start - $$, 5, 16384
    dd 1, data_start - $$, data_start, data_start, file_end - data_start, file_end - data_start, 6, 16384

start:
%if PREHEAT_NS > 0
    call preheat
%endif
    ; BENCHMARK_SETUP
    mov esi, input
    mov edi, scratch
    mov ebp, WARMUP_ITERATIONS
    call initialize
    call kernel
    call initialize
    mov eax, 265                 ; clock_gettime(CLOCK_MONOTONIC, ...)
    mov ebx, 1
    mov ecx, result
    int 0x80
    test eax, eax
    jnz failed_start
    call initialize
    mov ebp, ITERATIONS
    call kernel
    mov [result + 144], eax
    mov [result + 148], ebx
    mov [result + 152], ecx
    mov [result + 156], edx
    mov [result + 160], esi
    mov [result + 164], edi
    mov [result + 168], ebp
    mov eax, 265
    mov ebx, 1
    mov ecx, result + 8
    int 0x80
    test eax, eax
    jnz failed_end
%assign i 0
%rep 8
    movups [result + 16 + i * 16], xmm %+ i
%assign i i+1
%endrep
    mov esi, scratch
    mov edi, result + 176
    mov ecx, 64
    rep movsd
    mov eax, 5                   ; open result file after the timed region
    mov ebx, result_path
    mov ecx, 577                 ; O_WRONLY | O_CREAT | O_TRUNC
    mov edx, 384                 ; 0600
    int 0x80
    test eax, eax
    js failed_open
    mov ebx, eax
    mov eax, 4
    mov ecx, result
    mov edx, 432
    int 0x80
    cmp eax, 432
    jne failed_write
    mov eax, 6
    int 0x80
    xor ebx, ebx
    mov eax, 1
    int 0x80
failed_start:
    mov byte [failure_stage], '2'
    jmp failed
failed_end:
    mov byte [failure_stage], '3'
    jmp failed
failed_open:
    mov byte [failure_stage], '4'
    jmp failed
failed_write:
    mov byte [failure_stage], '5'
failed:
    mov eax, 4
    mov ebx, 1
    mov ecx, failure_message
    mov edx, failure_end - failure_message
    int 0x80
    mov ebx, 2
    mov eax, 1
    int 0x80

%if PREHEAT_NS > 0
; Preheat is excluded from timings and leaves no state used by the kernel.
preheat:
    mov eax, 265
    mov ebx, 1
    mov ecx, warm_start
    int 0x80
    test eax, eax
    jnz failed_start
.again:
    mov edi, 200000
    mov esi, 1
.burn:
    imul esi, esi, 1664525
    add esi, 1013904223
    dec edi
    jnz .burn
    mov eax, 265
    mov ebx, 1
    mov ecx, warm_now
    int 0x80
    test eax, eax
    jnz failed_start
    mov eax, [warm_now]
    sub eax, [warm_start]
    cmp eax, 2
    jae .done
    imul eax, eax, 1000000000
    add eax, [warm_now+4]
    sub eax, [warm_start+4]
    cmp eax, PREHEAT_NS
    jb .again
.done:
    ret
%endif

initialize:
    mov eax, 3
    mov ebx, 5
    mov ecx, 7
    mov edx, 11
    mov esi, input
    mov edi, scratch
%assign i 0
%rep 8
    movups xmm %+ i, [input + i * 16]
%assign i i+1
%endrep
    ret

align 64
kernel:
    ; BENCHMARK_BODY
    dec ebp
    jnz kernel
    ret

align 16384, db 0
data_start:
result_path: db 'result.bin', 0
failure_message: db 'BENCH_FAILURE stage '
failure_stage: db '?', 10
failure_end:
align 16, db 0
result: times 432 db 0
input:
%rep 8
    dd 0x3f800000, 0x40000000, 0x40400000, 0x40800000
%endrep
; BENCHMARK_DATA
mapped_stack_top: dd 0
mapped_stack_saved_sp: dd 0
%if PREHEAT_NS > 0
align 16, db 0
warm_start: times 8 db 0
warm_now: times 8 db 0
%endif
scratch: times 4096 db 0
; Fully map the final host-page group so the main memory cases exercise the
; direct linear path on 16 KiB hosts, rather than a mixed-permission fallback.
align 16384, db 0
file_end:

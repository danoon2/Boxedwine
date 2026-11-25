#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

/* Run with Wine's Windows 95 version profile. Its instruction-emulation
 * handler must not mistake a NULL read for an interrupt-table read. */
extern char null_read_instruction __asm__("null_read_instruction");
static int nullFault;

static LONG CALLBACK handler(EXCEPTION_POINTERS *exception)
{
    if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
        exception->ExceptionRecord->ExceptionAddress == &null_read_instruction &&
        exception->ExceptionRecord->ExceptionInformation[1] == 0) {
        nullFault++;
        exception->ContextRecord->Eip += 2;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

int main(void)
{
    struct __attribute__((packed)) { WORD limit; DWORD base; } idt;
    __asm__ volatile("sidt %0" : "=m" (idt));
    HMODULE kernel16 = LoadLibraryA("krnl386.exe16");
    PVOID registration = AddVectoredExceptionHandler(0, handler);
    if (!registration) return 2;
    __asm__ volatile("xorl %%ecx, %%ecx\n"
        ".global null_read_instruction\n"
        "null_read_instruction:\n"
        "movb (%%ecx), %%bl" : : : "ecx", "ebx", "memory");
    RemoveVectoredExceptionHandler(registration);
    int passed = kernel16 && (GetVersion() & 0x80000000) && nullFault == 1;
    FILE *file = fopen("Z:\\files\\idt-result.txt", "w");
    if (!file) return 2;
    fprintf(file, "win9x NULL read idt=%08lx limit=%u kernel16=%d faults=%d %s\n",
        idt.base, idt.limit, kernel16 != NULL, nullFault, passed ? "PASS" : "FAIL");
    fclose(file);
    return passed ? 0 : 1;
}

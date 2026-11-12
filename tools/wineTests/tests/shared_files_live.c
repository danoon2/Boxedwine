/* Shared Y: integration probe. The host harness adds, changes and removes files
 * after READY; the guest must observe them without restarting the engine. */
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <string.h>

static int exists(const char* path) { return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES; }
static int has_text(const char* path, const char* text) {
    char data[80] = {0}; DWORD count = 0;
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    ReadFile(file, data, sizeof(data) - 1, &count, NULL); CloseHandle(file);
    return strcmp(data, text) == 0;
}
static void list(const char* pattern) {
    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA(pattern, &data);
    if (find != INVALID_HANDLE_VALUE) { while (FindNextFileA(find, &data)) {} FindClose(find); }
}
int main(void) {
    HANDLE notify = FindFirstChangeNotificationA("Y:\\", TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (notify == INVALID_HANDLE_VALUE) { printf("FAIL watch %lu\n", GetLastError()); return 1; }
    list("Y:\\*"); list("Y:\\Nested\\*");
    printf("SHARED_READY\n"); fflush(stdout);
    DWORD until = GetTickCount() + 45000;
    int notified = 0;
    while ((LONG)(until - GetTickCount()) > 0) {
        if (WaitForSingleObject(notify, 0) == WAIT_OBJECT_0) { notified = 1; FindNextChangeNotification(notify); }
        list("Y:\\*"); list("Y:\\Nested\\*");
        if (notified && has_text("Y:\\patch.txt", "copied") && has_text("Y:\\Nested\\changed.txt", "updated")
            && exists("Y:\\renamed.txt") && !exists("Y:\\old.txt") && !exists("Y:\\removed.txt")) {
            HANDLE file = CreateFileA("Y:\\guest-output.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
            if (file == INVALID_HANDLE_VALUE) { printf("FAIL guest write %lu\n", GetLastError()); return 1; }
            DWORD written; WriteFile(file, "from Wine", 9, &written, NULL); CloseHandle(file);
            FindCloseChangeNotification(notify);
            printf("SHARED_PASS\n"); fflush(stdout); return 0;
        }
        Sleep(100);
    }
    printf("FAIL timeout notification=%d new=%d nested=%d renamed=%d removed=%d\n", notified,
        exists("Y:\\patch.txt"), has_text("Y:\\Nested\\changed.txt", "updated"), exists("Y:\\renamed.txt"), exists("Y:\\removed.txt"));
    FindCloseChangeNotification(notify);
    return 1;
}

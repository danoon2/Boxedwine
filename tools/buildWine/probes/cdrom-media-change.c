/* Run with -cdrom <ISO labelled BOXEDWINE_CD> d, or pass another drive letter. */
#include <windows.h>
#include <winioctl.h>
#include <dbt.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static unsigned int checks, failures, removals, arrivals;
static char drive_root[] = "D:\\", file_path[] = "D:\\FILE.TXT", device_path[] = "\\\\.\\D:";
static unsigned int drive_mask = 1u << 3;

static void check(int condition, const char *message)
{
    ++checks;
    if (!condition)
    {
        printf("FAIL: %s (error %lu)\n", message, GetLastError());
        ++failures;
    }
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_DEVICECHANGE && lparam &&
        (wparam == DBT_DEVICEARRIVAL || wparam == DBT_DEVICEREMOVECOMPLETE))
    {
        const DEV_BROADCAST_HDR *header = (const DEV_BROADCAST_HDR *)lparam;
        if (header->dbch_devicetype == DBT_DEVTYP_VOLUME)
        {
            const DEV_BROADCAST_VOLUME *volume = (const DEV_BROADCAST_VOLUME *)lparam;
            if ((volume->dbcv_unitmask & drive_mask) && (volume->dbcv_flags & DBTF_MEDIA))
            {
                if (wparam == DBT_DEVICEARRIVAL) ++arrivals;
                else ++removals;
            }
        }
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

static void wait_for_notice(const unsigned int *counter)
{
    DWORD start = GetTickCount();
    MSG message;
    while (!*counter && GetTickCount() - start < 10000)
    {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        Sleep(10);
    }
}

int main(int argc, char **argv)
{
    WNDCLASSA window_class = {0};
    HWND window;
    HANDLE drive;
    DWORD serial = 0, reinserted_serial = 0, returned;
    char label[256] = {0}, filesystem[256] = {0};

    if (argc > 1)
    {
        int letter = toupper((unsigned char)argv[1][0]);
        if (strlen(argv[1]) != 1 || letter < 'A' || letter > 'Z')
        {
            fprintf(stderr, "Usage: cdrom-media-change.exe [drive letter]\n");
            return 2;
        }
        drive_root[0] = file_path[0] = device_path[4] = (char)letter;
        drive_mask = 1u << (letter - 'A');
    }

    SetErrorMode(SEM_FAILCRITICALERRORS);
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = GetModuleHandleA(NULL);
    window_class.lpszClassName = "BoxedwineCdromMediaProbe";
    check(RegisterClassA(&window_class) != 0, "register window class");
    window = CreateWindowA(window_class.lpszClassName, "CD-ROM media probe", 0,
                          0, 0, 1, 1, NULL, NULL, window_class.hInstance, NULL);
    check(window != NULL, "create notification window");
    check(GetDriveTypeA(drive_root) == DRIVE_CDROM, "selected drive is a CD-ROM");
    check(GetVolumeInformationA(drive_root, label, sizeof(label), &serial, NULL, NULL,
                               filesystem, sizeof(filesystem)), "query inserted volume");
    check(!strcmp(label, "BOXEDWINE_CD"), "inserted volume label");
    {
        HANDLE file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ,
                                  NULL, OPEN_EXISTING, 0, NULL);
        WCHAR file_label[256] = {0};
        check(file != INVALID_HANDLE_VALUE, "open a file on the CD-ROM");
        if (file != INVALID_HANDLE_VALUE)
        {
            check(GetVolumeInformationByHandleW(file, file_label, 256, NULL, NULL, NULL, NULL, 0),
                  "query a CD volume through an ordinary file handle");
            check(!wcscmp(file_label, L"BOXEDWINE_CD"), "file handle volume label");
            CloseHandle(file);
        }
    }

    drive = CreateFileA(device_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, 0, NULL);
    check(drive != INVALID_HANDLE_VALUE, "open CD-ROM device");
    if (drive != INVALID_HANDLE_VALUE)
    {
        check(DeviceIoControl(drive, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0,
                              &returned, NULL), "eject media");
        wait_for_notice(&removals);
        check(removals != 0, "receive media removal notification");
        check(!GetVolumeInformationA(drive_root, label, sizeof(label), &reinserted_serial,
                                     NULL, NULL, NULL, 0), "ejected volume query fails");
        check(GetLastError() == ERROR_NOT_READY, "ejected volume reports no media");
        check(!GetVolumeInformationA(drive_root, NULL, 0, NULL, NULL, NULL,
                                     filesystem, sizeof(filesystem)), "ejected filesystem query fails");
        check(GetLastError() == ERROR_NOT_READY, "ejected filesystem reports no media");
        check(GetDriveTypeA(drive_root) == DRIVE_CDROM, "empty drive remains a CD-ROM");
        check(DeviceIoControl(drive, IOCTL_STORAGE_LOAD_MEDIA, NULL, 0, NULL, 0,
                              &returned, NULL), "reload media");
        wait_for_notice(&arrivals);
        check(arrivals != 0, "receive media arrival notification");
        check(GetVolumeInformationA(drive_root, label, sizeof(label), &reinserted_serial,
                                    NULL, NULL, NULL, 0), "query reinserted volume");
        check(!strcmp(label, "BOXEDWINE_CD") && serial == reinserted_serial,
              "reinserted volume restores label and serial");
        CloseHandle(drive);
    }
    if (window) DestroyWindow(window);
    printf("CDROM_MEDIA_%s: %u failures, %u removals, %u arrivals\n",
           failures ? "FAIL" : "PASS", failures, removals, arrivals);
    printf("Summary: %u passed, %u failed, 0 skipped\n", checks - failures, failures);
    return failures != 0;
}

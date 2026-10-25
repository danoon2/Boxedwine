/* Win16 USER.74 probe. Build with Open Watcom; see copyrect16.md. */
#include <windows.h>

static char log_path[260];
static int quiet;

static int contains(LPCSTR text, LPCSTR word)
{
    LPCSTR a, b;
    for (; *text; ++text) {
        a = text;
        b = word;
        while (*b && *a == *b) { ++a; ++b; }
        if (!*b) return 1;
    }
    return 0;
}

static void log_line(const char *line)
{
    HFILE file = _lopen(log_path, OF_WRITE);
    if (file == HFILE_ERROR) return;
    _llseek(file, 0L, 2);
    _lwrite(file, line, lstrlen(line));
    _lwrite(file, "\r\n", 2);
    _lclose(file); /* Preserve the last message if the next API call faults. */
}

static void probe(const char *name, RECT FAR *dest, const RECT FAR *src,
                  RECT *observed)
{
    char line[256];
    wsprintf(line,
        "BEFORE %s dest=%04x:%04x src=%04x:%04x badRead=%u badWrite=%u",
        name, HIWORD((DWORD)dest), LOWORD((DWORD)dest),
        HIWORD((DWORD)src), LOWORD((DWORD)src),
        IsBadReadPtr(src, sizeof(RECT)), IsBadWritePtr(dest, sizeof(RECT)));
    log_line(line);
    /* Win16 CopyRect is void. Do not interpret AX as a documented return value. */
    CopyRect(dest, src);
    wsprintf(line, "AFTER %s observed=%d,%d,%d,%d", name,
        observed->left, observed->top, observed->right, observed->bottom);
    log_line(line);
}

int PASCAL WinMain(HINSTANCE instance, HINSTANCE previous,
                  LPSTR command, int show)
{
    RECT source = { 11, 22, 33, 44 };
    RECT dest = { -1, -2, -3, -4 };
    DWORD version = GetVersion();
    HFILE file;
    char line[300];
    char *name;
    int i, case_id = 18;
    (void)previous;
    (void)show;

    /* No argument reproduces Robot City's exact source pointer. */
    if (contains(command, "valid")) case_id = 1;
    else if (contains(command, "null")) case_id = 0;
    else if (contains(command, "dest18")) case_id = 19;
    else if (contains(command, "faults")) case_id = 20;
    quiet = contains(command, "quiet");

    GetModuleFileName(instance, log_path, sizeof(log_path));
    name = log_path;
    for (i = 0; log_path[i]; ++i)
        if (log_path[i] == '\\' || log_path[i] == '/' || log_path[i] == ':')
            name = log_path + i + 1;
    if (name - log_path > sizeof(log_path) - 13) return 2;
    lstrcpy(name, "RECT16.TXT");
    file = _lcreat(log_path, 0);
    if (file == HFILE_ERROR) {
        MessageBox(NULL, "Cannot create RECT16.TXT beside the executable.",
            "CopyRect16 probe", MB_OK | MB_ICONSTOP);
        return 2;
    }
    _lclose(file);
    wsprintf(line, "CopyRect16 probe; Windows %u.%u; GetVersion=%08lx; sizeof(RECT)=%u",
        LOBYTE(LOWORD(version)), HIBYTE(LOWORD(version)), version, sizeof(RECT));
    log_line(line);
    probe("valid", &dest, &source, &dest);
    if (dest.left != 11 || dest.top != 22 || dest.right != 33 || dest.bottom != 44) {
        log_line("ERROR: valid control did not copy the rectangle");
        return 3;
    }
    if (case_id != 1) {
        dest.left = -1; dest.top = -2; dest.right = -3; dest.bottom = -4;
    }
    if (case_id == 18) probe("source-0000:0018", &dest, (const RECT FAR *)0x18UL, &dest);
    else if (case_id == 0) probe("source-0000:0000", &dest, (const RECT FAR *)0UL, &dest);
    else if (case_id == 19) probe("dest-0000:0018", (RECT FAR *)0x18UL, &source, &dest);
    else if (case_id == 20) {
        probe("source-ffff:0018", &dest, (const RECT FAR *)0xffff0018UL, &dest);
        probe("dest-ffff:0018", (RECT FAR *)0xffff0018UL, &source, &dest);
    }
    log_line("DONE: all selected calls returned");
    if (!quiet) {
        wsprintf(line, "The selected CopyRect calls returned.\r\n\r\nLog: %s\r\n\r\nFinal rectangle: %d,%d,%d,%d",
            log_path, dest.left, dest.top, dest.right, dest.bottom);
        MessageBox(NULL, line, "CopyRect16 probe", MB_OK);
    }
    return 0;
}

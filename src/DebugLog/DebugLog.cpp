// Debug output, SQZ.EXE 0x418a60 / 0x418eb0 / 0x419250-0x419360: message box, debug.log, console, debug listbox.
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern HANDLE g_logFile;        // 0x4ba7e0
extern HWND g_debugList;        // 0x4ba7e4
extern int g_toConsole;         // 0x4ba7e8
extern char g_logBuf[];         // 0x4c6550
extern HWND g_listWnd;          // 0x4c6950
extern unsigned g_maxLines;     // 0x4c6548
extern char g_lineFlag;         // 0x4c6958
void AddDebugLine(const char *text);   // 0x418f60

void ErrorBox(int unused, const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    MessageBox(0, buf, "Squeezil Application Error", MB_ICONHAND | MB_RETRYCANCEL);
}

void OpenLog()
{
    g_logFile = CreateFile("debug.log", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
}

void EnableConsole()
{
    g_toConsole = 1;
}

int DebugLogAll(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n;
    vsprintf(g_logBuf, fmt, ap);
    DWORD written;
    if (g_debugList) {
        AddDebugLine(g_logBuf);
        InvalidateRect(g_debugList, 0, 1);
        UpdateWindow(g_debugList);
    }
    if (g_toConsole && GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), g_logBuf, strlen(g_logBuf), &written, 0);
    if (g_logFile)
        WriteFile(g_logFile, g_logBuf, strlen(g_logBuf), &written, 0);
    return n;
}

int DebugLog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n;
    vsprintf(g_logBuf, fmt, ap);
    DWORD written;
    if (g_logFile)
        WriteFile(g_logFile, g_logBuf, strlen(g_logBuf), &written, 0);
    return n;
}

LRESULT CALLBACK DebugWndProc(HWND, UINT, WPARAM, LPARAM);
void CreateDebugList(HWND parent, int unused, unsigned maxLines)
{
    g_listWnd = CreateWindowEx(0, "listbox", 0, 0x40a00100, 0, 0, 0, 0, parent, 0,
                               (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE), 0);
    ShowWindow(g_listWnd, 1);
    UpdateWindow(g_listWnd);
    g_maxLines = maxLines;
    if (g_maxLines > 100)
        g_maxLines = 100;
    g_lineFlag = 0;
}

void PositionDebugList(unsigned width, unsigned height)
{
    unsigned y = (100 - g_maxLines) * height / 100;
    MoveWindow(g_listWnd, 0, y, width - 1, height - y - 1, 1);
}

void CreateDebugWindow(int unused, HINSTANCE inst)
{
    WNDCLASS wc;
    wc.style = 3;
    wc.lpfnWndProc = DebugWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 4;
    wc.hInstance = inst;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(0);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "DebugOutputWindow";
    if (!RegisterClass(&wc))
        return;
    g_debugList = CreateWindowEx(0, "DebugOutputWindow", "Debug Output", 0x10c40000, CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, inst, 0);
    if (!g_debugList) {
        GetLastError();
        return;
    }
    g_debugList = g_debugList;
    ShowWindow(g_debugList, 1);
    UpdateWindow(g_debugList);
    CreateDebugList(g_debugList, (int)inst, 100);
}

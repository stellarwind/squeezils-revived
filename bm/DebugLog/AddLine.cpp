// AddDebugLine, SQZ.EXE 0x418f60: append text to the pending line buffer and move complete lines into the debug listbox.
#include <windows.h>
#include <string.h>

extern char g_lineBuf[];       // 0x4c6958
extern HWND g_listWnd;         // 0x4c6950

void AddDebugLine(const char *text)
{
    int i;
    int n;
    strcat(g_lineBuf, text);
    n = strlen(g_lineBuf);
    for (i = 0; g_lineBuf[i]; i++) {
        if (g_lineBuf[i] == '\n') {
            g_lineBuf[i] = 0;
            break;
        }
    }
    if (n != i) {
        int idx = SendMessage(g_listWnd, LB_ADDSTRING, -1, (LPARAM)g_lineBuf);
        if (idx == -1)
            return;
        while (idx == -2) {
            SendMessage(g_listWnd, LB_DELETESTRING, 0, 0);
            i++;
        }
        SendMessage(g_listWnd, LB_SETCURSEL, n - 1, 0);
        int start = ++i;
        while (g_lineBuf[i]) {
            g_lineBuf[i - start] = g_lineBuf[i];
            i++;
        }
        g_lineBuf[i - start] = 0;
        if (strlen(g_lineBuf))
            AddDebugLine("");
    }
}

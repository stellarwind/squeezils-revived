// Error reporter (SQZ.EXE 0x41eae0, cdecl): remember the message and, depending on the error mode, format it and show a message box.
#include <windows.h>
#include <stdio.h>
#include <string.h>

extern char g_lastError[];      // 0x4c6d58
extern int g_errorMode;         // 0x4bac78

void ShowError(const char *where, const char *message)
{
    char text[0x100];
    if (message) {
        strcpy(g_lastError, message);
        if (g_errorMode) {
            if (g_errorMode == 1)
                sprintf(text, "Error: %s.", message);
            if (g_errorMode == 2)
                sprintf(text, "Error: at %s.", where, message);
            MessageBox(0, text, "Error", 0x30);
        }
    }
}

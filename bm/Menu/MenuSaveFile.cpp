// OpenSaveFile (0x406dc0): create a save file, asking before overwriting an existing one.
#include <windows.h>

HANDLE OpenSaveFile(const char *path, HWND parent)
{
    SECURITY_ATTRIBUTES sa; sa.nLength = 12; sa.lpSecurityDescriptor = 0; sa.bInheritHandle = 0;
    HANDLE h = CreateFile(path, GENERIC_WRITE, 0, &sa, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE) {
    DWORD err = GetLastError();
    if (err != 0x50 && err != 0xb7) {
        MessageBox(parent, "Error Creating File!", "Error", 0);
        return 0;
    }
    if (MessageBox(parent, "File Already Exists! Overwrite?", "Error Creating File", MB_YESNO) == IDYES)
        h = CreateFile(path, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE) {
        MessageBox(parent, "Error Creating File!", "Error", 0);
        return 0;
    }
    return h;
    }
    return h;
}

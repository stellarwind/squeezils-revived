// FileOpenDialog (SQZ.EXE 0x41ed80, cdecl): the Open File dialog; the chosen path, or NULL.
#include <windows.h>
#include <commdlg.h>
#include <string.h>

extern char kEmptyString[];     // 0x4b9488
extern char g_filePath[0x100];      // 0x4c6e58
extern char g_fileTitle[0x100];     // 0x4c6f58

char *FileOpenDialog(HWND owner)
{
    OPENFILENAME ofn;
    g_filePath[0] = kEmptyString[0];
    g_fileTitle[0] = kEmptyString[0];
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.hInstance = (HINSTANCE)GetWindowLong(owner, GWL_HINSTANCE);
    ofn.lpstrFilter = "Gif files (*.gif)\0*.gif\0All Files (*.*)\0*.*\0";
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = g_filePath;
    ofn.nMaxFile = 0x100;
    ofn.lpstrFileTitle = g_fileTitle;
    ofn.nMaxFileTitle = 0x100;
    ofn.lpstrInitialDir = 0;
    ofn.lpstrTitle = "Load a Six-View Warp Gif";
    ofn.lpstrDefExt = "gif";
    ofn.Flags = 0x1810;
    ofn.nFileOffset = 0;
    ofn.lCustData = 0;
    ofn.nFileExtension = 0;
    if (GetOpenFileName(&ofn))
        return ofn.lpstrFile;
    return 0;
}

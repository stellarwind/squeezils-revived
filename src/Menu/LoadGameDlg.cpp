// LoadGameDlgProc (0x406eb0): the Load Game dialog (resource 0x71).
// Lists the files in the save folder with their dates; buttons Load, Save, Delete.
#include <windows.h>
#include <stdlib.h>
#include <string.h>

extern char g_saveName[];      // 0x4c63d0
extern char g_loadName[];      // 0x4c63e8
extern char g_saveDir[];       // 0x4c6400
extern HINSTANCE g_hInstance;  // 0x4c73c8

struct Game {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8();
    virtual int CanSaveHere();            // vtable+0x24
    virtual int Load(const char *path);   // vtable+0x28
    virtual int Save(const char *path);   // vtable+0x2c
};
Game *GetGame();                                        // 0x420690
HANDLE OpenSaveFile(const char *path, HWND parent);     // 0x406dc0
int CALLBACK NameDlgProc(HWND, UINT, WPARAM, LPARAM);   // 0x4077c0

#define IDC_SAVE_LIST   0x41a
#define IDC_SAVE_SAVE   0x418
#define IDC_SAVE_DELETE 0x419
#define IDC_SAVE_LOAD   0x41b

int CALLBACK LoadGameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int count;
    SYSTEMTIME st;
    char text[100];
    char name[100];
    char path2[200];
    WIN32_FIND_DATA fd;
    char path[200];
    HWND list = GetDlgItem(hDlg, IDC_SAVE_LIST);

    if (msg == WM_INITDIALOG) {
        int n = SendMessage(list, LB_GETCOUNT, 0, 0);
        for (; n > 0; n--)
            SendMessage(list, LB_DELETESTRING, 0, 0);
        count = 0;
        strcpy(path, g_saveDir);
        strcat(path, "*.*");
        HANDLE find = FindFirstFile(path, &fd);
        if (find != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    // "name          M/D/YYYY", the name padded to 14 columns
                    strcpy(name, fd.cFileName);
                    for (int pad = 14 - strlen(name); pad > 0; pad--)
                        strcat(name, " ");
                    FileTimeToSystemTime(&fd.ftLastAccessTime, &st);
                    _itoa(st.wMonth, name + strlen(name), 10);
                    strcat(name, "/");
                    _itoa(st.wDay, name + strlen(name), 10);
                    strcat(name, "/");
                    _itoa(st.wYear, name + strlen(name), 10);
                    SendMessage(list, LB_ADDSTRING, 0, (LPARAM)name);
                    count++;
                }
            } while (FindNextFile(find, &fd));
        }
        if (count > 0)
            SendMessage(list, LB_SETCURSEL, 0, 0);
        SetFocus(list);
    } else if (msg == WM_COMMAND) {
        short id = (short)wParam;
        if (id == IDOK) {
            EndDialog(hDlg, 1);
        } else if (id == IDCANCEL) {
            EndDialog(hDlg, 0);
        } else if (id == IDC_SAVE_DELETE) {
            int sel = SendMessage(list, LB_GETCURSEL, 0, 0);
            if (sel >= 0) {
                size_t len = 0;
                SendMessage(list, LB_GETTEXT, sel, (LPARAM)text);
                while (text[len] != ' ')
                    len++;
                strncpy(name, text, len);
                name[len] = 0;
                strcpy(text, "Delete ");
                strcat(text, name);
                strcat(text, " ?");
                if (MessageBox(hDlg, text, "Delete Game", MB_YESNO) == IDYES) {
                    strcpy(text, g_saveDir);
                    strcat(text, name);
                    if (!DeleteFile(text)) {
                        MessageBox(hDlg, "Could not delete saved game!", "Error", 0);
                        SendMessage(list, LB_SETCURSEL, sel, 0);
                    } else {
                        SendMessage(list, LB_DELETESTRING, sel, 0);
                        SendMessage(list, LB_SETCURSEL, 0, 0);
                    }
                }
            }
        } else if (id == IDC_SAVE_SAVE) {
            if (!DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x72), hDlg, (DLGPROC)NameDlgProc, 0)) {
                MessageBox(hDlg, "Did not save game", "Save Game", 0);
            } else {
                strcpy(name, g_saveDir);
                strcat(name, g_saveName);
                HANDLE file = OpenSaveFile(name, hDlg);
                if (!file) {
                    strcpy(text, "Could not save game: ");
                    strcat(text, g_saveName);
                    MessageBox(hDlg, text, "Save Game", 0);
                } else {
                    CloseHandle(file);
                    strcpy(text, "Saved game: ");
                    strcat(text, g_saveName);
                    MessageBox(hDlg, text, "Save Game", MB_ICONEXCLAMATION);
                    if (!GetGame()->CanSaveHere())
                        MessageBox(hDlg, "When you load this save game, you will be\nin the region you started from.", "Save Game", 0);
                    if (GetGame()->Save(name)) {
                        SendMessage(hDlg, WM_INITDIALOG, 0, 0);   // refill the list
                    } else {
                        strcpy(text, "Could not save game: ");
                        strcat(text, g_saveName);
                        MessageBox(hDlg, text, "Save Game", 0);
                    }
                }
            }
        } else if (id == IDC_SAVE_LOAD) {
            int sel = SendMessage(list, LB_GETCURSEL, 0, 0);
            if (sel >= 0) {
                size_t len = 0;
                SendMessage(list, LB_GETTEXT, sel, (LPARAM)text);
                while (text[len] != ' ')
                    len++;
                strncpy(name, text, len);
                name[len] = 0;
                strcpy(path2, g_saveDir);
                strcat(path2, name);
                strcpy(g_loadName, name);
                EndDialog(hDlg, 1);
                return 1;
            }
        }
    }
    return 0;
}

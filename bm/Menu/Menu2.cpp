// Menu helpers, SQZ.EXE 0x406670-0x406a60: help topics, the dialog launchers, HitTest.
#include <windows.h>
// one spare declaration (symbol count, see padscan.sh)
void PadDecl1();
#include <ctype.h>
#include <string.h>

struct App { char pad[0x18]; HWND hwnd; };
extern App *g_app;                 // 0x4c75dc
extern HINSTANCE g_hInstance;      // 0x4c73c8
extern HWND g_mainWnd;             // 0x4c73cc
extern const char kHelpFile[];     // 0x4b9558 ".\help\sqz.hlp"

int CALLBACK Proc406cb0(HWND, UINT, WPARAM, LPARAM);
int CALLBACK Proc406d60(HWND, UINT, WPARAM, LPARAM);
int CALLBACK Proc4078e0(HWND, UINT, WPARAM, LPARAM);
int CALLBACK Proc407d40(HWND, UINT, WPARAM, LPARAM);
int CALLBACK Proc408410(HWND, UINT, WPARAM, LPARAM);

int ActionHelp1()
{
    WinHelp(g_app->hwnd, ".\help\sqz.hlp", 1, 1);
    return 0;
}

int ActionHelp2()
{
    WinHelp(g_app->hwnd, ".\help\sqz.hlp", 1, 2);
    return 0;
}

int ShowJoystick(HWND parent)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x74), parent, (DLGPROC)Proc4078e0, 0);
    RedrawWindow(parent, 0, 0, 0x585);
    return 0;
}

int ShowDialog6a(HWND parent)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x6a), parent, (DLGPROC)Proc407d40, 0);
    RedrawWindow(parent, 0, 0, 0x585);
    return 0;
}

int ShowDialog73(HWND parent)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x73), parent, (DLGPROC)Proc408410, 0);
    RedrawWindow(parent, 0, 0, 0x585);
    return 0;
}

int ActionDialog81()
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x81), g_mainWnd, (DLGPROC)Proc406cb0, 0);
    return 0;
}

int ActionDialog80()
{
    return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x80), g_mainWnd, (DLGPROC)Proc406d60, 0) != 0;
}

struct MItem { char pad[0x20]; int left, top, right, bottom; };
struct MenuH {
    char pad[0x18 - 8];
    int hdr[2];
    MItem items[10];
    int HitTest(int index, int x, int y);
};

int MenuH::HitTest(int index, int x, int y)
{
    MItem *it = (MItem *)((char *)this + index * 0x24);
    if (it->left <= x && it->right >= x && it->top < y && it->bottom >= y)
        return 1;
    return 0;
}

int CALLBACK ConfirmDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        return 1;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            EndDialog(hwnd, 1);
            return 0;
        }
        if (LOWORD(wParam) == 2) {
            EndDialog(hwnd, 0);
            return 0;
        }
    }
    return 0;
}

int IsAlnumString(const char *s)
{
    if (strlen(s) == 0)
        return 0;
    for (unsigned i = 0; i < strlen(s); i++) {
        if (!isalnum(s[i]))
            return 0;
    }
    return 1;
}

int CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, 0x415, 1);
        CheckDlgButton(hwnd, 0x413, 1);
        return 1;
    case WM_COMMAND:
        if (LOWORD(wParam) == 2) {
            EndDialog(hwnd, 0);
            return 0;
        }
        if (LOWORD(wParam) == 0x417) {
            int r;
            if (IsDlgButtonChecked(hwnd, 0x415))
                r = IsDlgButtonChecked(hwnd, 0x413) ? 1 : 3;
            else
                r = IsDlgButtonChecked(hwnd, 0x413) ? 2 : 4;
            EndDialog(hwnd, r);
            return 0;
        }
    }
    return 0;
}

extern char g_name[];     // 0x4c63d0

int CALLBACK NameDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        strcpy(g_name, "");
        SendMessage(GetDlgItem(hwnd, 0x41d), 0xc5, 10, 0);
        return 1;
    case WM_COMMAND:
        if (LOWORD(wParam) == 0x417) {
            *(int *)g_name = 10;
            int len = SendMessage(GetDlgItem(hwnd, 0x41d), 0xc4, 0, (LPARAM)g_name);
            g_name[len] = 0;
            if (IsAlnumString(g_name)) {
                EndDialog(hwnd, 1);
                return 1;
            }
            MessageBox(hwnd, "Please use only letters and Valid Filenames contain alphanumeric characters only", "Error", MB_ICONHAND);
            EndDialog(hwnd, 0);
            return 1;
        }
        if (LOWORD(wParam) == 2) {
            EndDialog(hwnd, 0);
            return 0;
        }
    }
    return 0;
}

extern float g_vol1;      // 0x4c77c8
extern float g_vol2;      // 0x4c77cc
extern float g_vol3;      // 0x4c77a0
extern const float kToSlider;     // 0x49d294
extern const float kFromSlider;   // 0x49d298
void SetVolumeA(float v);   // 0x44aa10
void SetVolumeB(float v);   // 0x44a940
void SetVolumeC(float v);   // 0x44aab0
void DebugLog(const char *fmt, ...);   // 0x419290

int CALLBACK VolumeDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND s1 = GetDlgItem(hwnd, 0x420);
    HWND s2 = GetDlgItem(hwnd, 0x421);
    HWND s3 = GetDlgItem(hwnd, 0x422);
    switch (msg) {
    case WM_INITDIALOG:
        SendMessage(s1, 0x406, 1, 0xa0000);
        SendMessage(s2, 0x406, 1, 0xa0000);
        SendMessage(s3, 0x406, 1, 0xa0000);
        SendMessage(s1, 0x405, 1, (int)(g_vol1 * kToSlider));
        SendMessage(s3, 0x405, 1, (int)(g_vol2 * kToSlider));
        SendMessage(s2, 0x405, 1, (int)(g_vol3 * kToSlider));
        return 1;
    case WM_COMMAND:
        if (LOWORD(wParam) == 0x417) {
            DWORD v = SendMessage(s1, 0x400, 0, 0);
            SetVolumeA((float)v * kFromSlider);
            DWORD w = SendMessage(s3, 0x400, 0, 0);
            float f = (float)w * kFromSlider;
            DebugLog("settingpercent: %f pos:%d \n", (double)f, w);
            SetVolumeB(f);
            DWORD x = SendMessage(s2, 0x400, 0, 0);
            SetVolumeC((float)x * kFromSlider);
            EndDialog(hwnd, 1);
            return 1;
        }
        if (LOWORD(wParam) == 2) {
            EndDialog(hwnd, 0);
            return 0;
        }
    }
    return 0;
}

struct CalObj {
    void Prepare();            // 0x447880
    int Ready();               // 0x4479c0
    int Run(HWND dlg);         // 0x447f00
};
struct Prefs {
    int GetMode();             // 0x43aee0
    int SetMode(int m);        // 0x43aef0
    int HasJoystick();         // 0x43c7a0
    char pad[0x10 - 1];
    CalObj *cal;               // +0x10
};
Prefs *GetPrefs();             // 0x4206b0

int CALLBACK JoystickDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        int mode = GetPrefs()->GetMode();
        if (mode == 0)
            CheckRadioButton(hwnd, 0x3fe, 0x400, 0x3fe);
        else if (mode == 1)
            CheckRadioButton(hwnd, 0x3fe, 0x400, 0x3ff);
        else if (mode == 2)
            CheckRadioButton(hwnd, 0x3fe, 0x400, 0x400);
        return 1;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            int a = IsDlgButtonChecked(hwnd, 0x3fe);
            int b = IsDlgButtonChecked(hwnd, 0x3ff);
            int c = IsDlgButtonChecked(hwnd, 0x400);
            if (c && !GetPrefs()->HasJoystick()) {
                MessageBox(hwnd, "No joystick detected. Please check your connection\nand run the Joystick Calibration application using\nthe windows control panel.", "Error:No Joystick Detected", MB_ICONHAND);
                RedrawWindow(hwnd, 0, 0, 0x585);
                return 0;
            }
            if (a)
                GetPrefs()->SetMode(0);
            else if (b)
                GetPrefs()->SetMode(1);
            else if (c) {
                if (!GetPrefs()->SetMode(2)) {
                    MessageBox(hwnd, "Unable to connect with joystick", "Error", MB_ICONEXCLAMATION);
                    EndDialog(hwnd, 0);
                    RedrawWindow(hwnd, 0, 0, 0x585);
                    return 0;
                }
            }
            EndDialog(hwnd, 1);
        }
        if (LOWORD(wParam) == 2)
            EndDialog(hwnd, 0);
        if (LOWORD(wParam) == 0x429) {
            if (!GetPrefs()->HasJoystick()) {
                MessageBox(hwnd, "No joystick detected. Please check your connection\nand run the Joystick Calibration application using\nthe windows control panel.", "Error:No Joystick Detected", MB_ICONHAND);
                RedrawWindow(hwnd, 0, 0, 0x585);
                return 0;
            }
            GetPrefs()->cal->Prepare();
            if (!GetPrefs()->cal->Ready()) {
                if (!GetPrefs()->cal->Run(hwnd)) {
                    MessageBox(hwnd, "Unable to connect with joystick", "Error", MB_ICONEXCLAMATION);
                    RedrawWindow(hwnd, 0, 0, 0x585);
                    return 0;
                }
                MessageBox(hwnd, "Joystick Calibration Complete!", "Calibration", MB_ICONEXCLAMATION);
                RedrawWindow(hwnd, 0, 0, 0x585);
                return 0;
            }
            GetPrefs()->SetMode(2);
            CheckRadioButton(hwnd, 0x3fe, 0x400, 0x400);
            EndDialog(hwnd, 1);
            return 1;
        }
    }
    return 0;
}

extern char g_text[];      // 0x4c6468
extern int g_dirty;        // 0x4b9510

int CALLBACK TextDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowText(GetDlgItem(hwnd, 0x411), g_text);
        return 1;
    case WM_COMMAND: {
        if (LOWORD(wParam) == 2) {
            g_dirty = 0;
            EndDialog(hwnd, 0);
            return 0;
        }
        WORD code = HIWORD(wParam);
        if (code == 0x300) {
            if (LOWORD(wParam) == 0x411 && g_dirty) {
                *(int *)g_text = 3;
                SendMessage(GetDlgItem(hwnd, 0x411), 0xc4, 0, (LPARAM)g_text);
                g_dirty = 0;
                EndDialog(hwnd, 1);
                return 0;
            }
        }
        if (code == 0x300 && LOWORD(wParam) == 0x411 && g_dirty == 0)
            g_dirty = 1;
    }
    }
    return 0;
}

int CALLBACK OptionsMenuDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        return 1;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            EndDialog(hwnd, 1);
            return 0;
        }
        if (LOWORD(wParam) == 2) {
            EndDialog(hwnd, 0);
            return 0;
        }
        if (LOWORD(wParam) == 0x9c6a) {
            ShowJoystick(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == 0x42f) {
            ShowDialog6a(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == 0x430) {
            ShowDialog73(hwnd);
            return 0;
        }
    }
    return 0;
}

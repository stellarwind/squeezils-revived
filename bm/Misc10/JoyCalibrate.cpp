// Joystick::Calibrate (SQZ.EXE 0x447f00, ret 4): run the calibration dialogs, then capture the stick; clears the ready flag on failure.
#include <windows.h>
#include <mmsystem.h>

struct JoyRenderer { char pad[0x18]; HWND hwnd; };
extern JoyRenderer *g_renderer;             // 0x4c75dc
extern HINSTANCE g_hInstance;               // 0x4c73c8
BOOL CALLBACK JoyDlgA(HWND, UINT, WPARAM, LPARAM);   // 0x447a30
BOOL CALLBACK JoyDlgB(HWND, UINT, WPARAM, LPARAM);   // 0x447af0
BOOL CALLBACK JoyDlgC(HWND, UINT, WPARAM, LPARAM);   // 0x447d80

struct JoyCal {
    char pad0[0x2c];
    int ready;                                  // 0x2c
    char pad1[0x44 - 0x30];
    int stage;                                  // 0x44
    int Calibrate(HWND parent);
};

int JoyCal::Calibrate(HWND parent)
{
    JOYCAPSA caps;
    int r;
    joyGetDevCapsA(0, &caps, sizeof(JOYCAPSA));
    if (stage == 1) {
        if (!DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x75), parent, (DLGPROC)JoyDlgA, 0)) {
            ready = 0;
            joyReleaseCapture(0);
            return 0;
        }
        if (!DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x76), parent, (DLGPROC)JoyDlgA, 0)) {
            ready = 0;
            joyReleaseCapture(0);
            return 0;
        }
        if (!DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x79), parent, (DLGPROC)JoyDlgA, 0)) {
            ready = 0;
            joyReleaseCapture(0);
            return 0;
        }
        r = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x77), parent, (DLGPROC)JoyDlgB, 0);
        if (!r) {
            ready = 0;
            joyReleaseCapture(0);
            return 0;
        }
    } else {
        if (stage == 2)
            r = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x77), parent, (DLGPROC)JoyDlgC, 0);
        else
            r = *(int *)&caps;
    }
    MMRESULT m = joyReleaseCapture(0);
    if (!r || m != 0 || ((m = joySetThreshold(0, 65000)) == 0 && (m = joySetCapture(g_renderer->hwnd, 0, 0, 1)) != 0))
        ready = 0;
    return ready;
}

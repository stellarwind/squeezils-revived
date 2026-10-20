// 0x447a30 (ret 0x10): joystick calibration dialog procedure.
#include <windows.h>
#include <mmsystem.h>
struct Joystick8 { int pad[2]; int xPos; };      // field at +8 receives the X reading
extern Joystick8 *g_joystick;                    // 0x4bd6f8
BOOL CALLBACK JoyDlgProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    JOYINFOEX info;
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNALL;
    switch (msg) {
    case WM_INITDIALOG:
        joyReleaseCapture(0);
        joySetCapture(hdlg, 0, 0, TRUE);
        return 1;
    case WM_COMMAND:
        if (LOWORD(wp) == IDCANCEL) {
            EndDialog(hdlg, 0);
            return 0;
        }
        break;
    case MM_JOY1BUTTONDOWN:
        if (joyGetPosEx(0, &info))
            return 0;
        g_joystick->xPos = info.dwXpos;
        EndDialog(hdlg, 1);
        return 1;
    }
    return 0;
}

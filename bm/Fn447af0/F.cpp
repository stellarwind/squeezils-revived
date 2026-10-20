// 0x447af0: joystick calibration dialog procedure: moves out to the extremes, then fills in centre, range and scale.
#include <windows.h>
#include <mmsystem.h>
struct JoyCal {
    int centerX, centerY;       // 0x00 0x04
    int minX, maxX;             // 0x08 0x0c
    int minY, maxY;             // 0x10 0x14
    int scaleX, scaleY;         // 0x18 0x1c
    int homeX, homeY;           // 0x20 0x24
    int range;                  // 0x28
    int valid;                  // 0x2c
};
extern int gJoyY, gJoyX;                                    // 0x4c7794 0x4c7790 (stored as x then y order below)
extern int gSamples;                                        // 0x4bd730
extern int gMaxXs, gMaxYs, gMinXs, gMinYs;                  // placeholders, see below
extern JoyCal *gCal;                                        // 0x4bd6f8
extern int gHiA, gHiB, gLoA, gLoB;                          // 0x4bd734 0x4bd738 0x4bd73c 0x4bd740
int Half(JoyCal *c);                                        // 0x4482a0
int Half2(JoyCal *c);                                       // 0x4482c0
BOOL CALLBACK JoyProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp)
{
    int jx = (lp >> 16) & 0xffff;
    int jy = lp & 0xffff;
    gJoyX = jx;
    gJoyY = jy;
    switch (msg) {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(dlg, 0x42d), 0x401, 0, 0x320000);
        SendMessage(GetDlgItem(dlg, 0x42d), 0x404, 1, 0);
        joyReleaseCapture(0);
        joySetCapture(dlg, 0, 0, 1);
        return joySetThreshold(0, 0) == 0;
    case WM_COMMAND:
        if ((short)wp == 2) {
            gSamples = 0;
            EndDialog(dlg, 1);
            return 0;
        }
        break;
    case 0x3a0:
        {
        gSamples++;
        if (gSamples >= 0x32) {
            int range = gHiB - gLoB;
            if (range <= gHiA - gLoA)
                range = gHiA - gLoA;
            gCal->centerX = (gLoA - gHiA) / 2 + gHiA;
            gCal->centerY = (gLoB - gHiB) / 2 + gHiB;
            gCal->range = range;
            gCal->homeX = gCal->centerX;
            gCal->homeY = gCal->centerY;
            gCal->maxX = gCal->minX + gCal->centerX * 2;
            gCal->maxY = gCal->minY + gCal->centerY * 2;
            gCal->scaleX = (int)((float)(gCal->maxX - gCal->minX) / (float)(Half(gCal) * 2));
            gCal->scaleY = (int)((float)(gCal->maxY - gCal->minY) / (float)(Half2(gCal) * 2));
            gCal->valid = 1;
            gSamples = 0;
            EndDialog(dlg, 1);
            return 1;
        }
        if (jy < gLoA)
            gLoA = jy;
        else if (jy > gHiA)
            gHiA = jy;
        if (jx < gLoB)
            gLoB = jx;
        else if (jx > gHiB)
            gHiB = jx;
        SendMessage(GetDlgItem(dlg, 0x42d), 0x405, 0, 0);
        }
        break;
    }
    return 0;
}

// 0x447de0: sample the centred joystick and compute screen-coordinate scales.
#include <windows.h>
#include <mmsystem.h>

struct JoystickCalibration {
    unsigned centreX, centreY;
    int pad08[4];
    float scaleX, scaleY;               // +0x18 / +0x1c
    unsigned rawX, rawY;                // +0x20 / +0x24
    int centreReady, scaleReady;        // +0x28 / +0x2c
    int OffsetX();                      // 0x4482a0
    int OffsetY();                      // 0x4482c0
    int CalibrateCentre();
};

int JoystickCalibration::CalibrateCentre()
{
    JOYINFOEX info;
    JOYCAPSA caps;
    unsigned maxX = 0, maxY = 0, minX = 65000, minY = 65000;
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNX | JOY_RETURNY;
    for (int i = 0; i < 200; i++) {
        if (joyGetPosEx(0, &info))
            return 0;
        if (minX > info.dwXpos) minX = info.dwXpos;
        if (maxX < info.dwXpos) maxX = info.dwXpos;
        if (minY > info.dwYpos) minY = info.dwYpos;
        if (maxY < info.dwYpos) maxY = info.dwYpos;
    }
    joyGetDevCapsA(0, &caps, sizeof(caps));
    centreReady = 1;
    centreX = minX;
    centreY = minY;
    rawX = minX;
    rawY = minY;
    scaleX = (float)caps.wXmax / (OffsetX() * 2);
    scaleY = (float)caps.wYmax / (OffsetY() * 2);
    scaleReady = 1;
    return 1;
}

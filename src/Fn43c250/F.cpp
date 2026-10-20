// 0x43c250: AppInput::LoadSettings - read the mouse, keyboard timing and interface options from the game ini; always 1.
#include <windows.h>
#include <stdlib.h>
#include "IniReader/IniReader.h"
extern char g_iniPath[];                                    // 0x4c74d8

struct AppInput {
    char pad0[0x58];
    float accelerationSeconds;                              // 0x58
    float jumpSeconds;                                      // 0x5c
    float spitSeconds;                                      // 0x60
    char pad64[0x80 - 0x64];
    int mouseVertical;                                      // 0x80: 0, 1 or 2
    int mouseMode;                                          // 0x84
    char pad88[0x98 - 0x88];
    int joystickMode;                                       // 0x98
    char pad9c[0xb0 - 0x9c];
    int toggleWindowsMenu;                                  // 0xb0
    char padb4[0xc0 - 0xb4];
    int logEnabled;                                         // 0xc0
    int LoadSettings();
};

int AppInput::LoadSettings()
{
    char value[100];
    joystickMode = 0;
    mouseVertical = 0;
    GetPrivateProfileString("MouseDefaults", "MouseVertical", "", value, 100, g_iniPath);
    if (value[0] == '0')
        mouseVertical = 0;
    if (value[0] == '1')
        mouseVertical = 1;
    if (value[0] == '2')
        mouseVertical = 2;
    mouseMode = 0;
    GetPrivateProfileString("MouseDefaults", "Mode", "", value, 100, g_iniPath);
    if (value[0] == '1')
        mouseMode = 1;

    accelerationSeconds = 2.0f;
    if (GetPrivateProfileString("Keyboard", "SecondstoMaxAcceleration", "", value, 100, g_iniPath)) {
        accelerationSeconds = (float)atof(value);
        if (accelerationSeconds < 0.0f)
            accelerationSeconds = 0.0f;
        if (accelerationSeconds > 10.0f)
            accelerationSeconds = 10.0f;
    }
    jumpSeconds = 3.0f;
    if (GetPrivateProfileString("Keyboard", "SecondstoMaxJump", "", value, 100, g_iniPath)) {
        jumpSeconds = (float)atof(value);
        if (jumpSeconds < 0.0f)
            jumpSeconds = 0.0f;
        if (jumpSeconds > 10.0f)
            jumpSeconds = 10.0f;
    }
    spitSeconds = 0.5f;
    if (GetPrivateProfileString("Keyboard", "SecondstoMaxSpit", "", value, 100, g_iniPath)) {
        spitSeconds = (float)atof(value);
        if (spitSeconds < 0.0f)
            spitSeconds = 0.0f;
        if (spitSeconds > 10.0f)
            spitSeconds = 10.0f;
    }

    toggleWindowsMenu = 0;
    GetPrivateProfileString("Interface", "ToggleWindowsMenu", "", value, 100, g_iniPath);
    toggleWindowsMenu = 1;
    if (value[0] != '1')
        toggleWindowsMenu = 0;

    logEnabled = 0;
    IniReader ini;
    ini.SetSection("Interface");
    logEnabled = ini.GetInt("LogEnabled", 0);
    return 1;
}

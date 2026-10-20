// 0x459320: read the [MouseDefaults] section of the ini file.
#include <windows.h>
extern char g_iniPath[];        // 0x4c74d8
struct MouseSettings {
    int pad0;
    int showCursor;             // 0x04
    char pad[0x18 - 8];
    int vertical;               // 0x18
    void LoadDefaults();
};
void MouseSettings::LoadDefaults()
{
    char buf[20];
    showCursor = 0;
    GetPrivateProfileString("MouseDefaults", "ShowCursor", "", buf, 100, g_iniPath);
    if (buf[0] == '1')
        showCursor = 1;
    vertical = 0;
    GetPrivateProfileString("MouseDefaults", "MouseVertical", "", buf, 100, g_iniPath);
    if (buf[0] == '0')
        vertical = 0;
    if (buf[0] == '1')
        vertical = 1;
    if (buf[0] == '2')
        vertical = 2;
}

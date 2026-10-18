// SQZ.EXE 0x43ec70: read a registry value (key path in a global, value name at +0xd9c) into the object's buffer at +0xc.
#include <windows.h>
extern char g_regKey[];             // 0x4c75ec
struct RegSettings {
    char pad[0xc];
    char data[0xd8c];               // +0xc
    int pad2;
    char valueName[1];              // +0xd9c
    int Read();
};
int RegSettings::Read()
{
    HKEY key;
    DWORD type;
    DWORD size;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_regKey, 0, KEY_READ, &key) != 0)
        return 0;
    size = 0xd8c;
    LONG r = RegQueryValueEx(key, valueName, 0, &type, (BYTE *)data, &size);
    RegCloseKey(HKEY_LOCAL_MACHINE);
    return r == 0;
}

// 0x43ecf0: write the 0xd8c-byte binding block to the registry.
#include <windows.h>
void DebugLog(const char *fmt, ...);     // 0x419290
extern char g_regKey[];                  // 0x4c75ec
struct BindingsReg {
    char pad[0xc];
    char block[0xd8c];       // 0x0c
    int pad2;                // 0xd98
    char valueName[0x20];    // 0xd9c
    int Save();
};
int BindingsReg::Save()
{
    HKEY key;
    LONG r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_regKey, 0, KEY_READ, &key);
    if (r) {
        DebugLog("COULD NOT OPEN REG KEY: %ld \n", r);
        return 0;
    }
    DebugLog("OPENED REG KEY \n ");
    r = RegSetValueEx(key, valueName, 0, REG_BINARY, (const BYTE *)block, 0xd8c);
    RegCloseKey(HKEY_LOCAL_MACHINE);
    return r == 0;
}

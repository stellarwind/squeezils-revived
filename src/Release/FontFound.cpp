// EnumFontFamilies callback (SQZ.EXE 0x42e710): keep the first font found, but prefer one whose style is "bold".
#include <windows.h>
#include <string.h>
void DebugLog(const char *format, ...);   // 0x419290
int g_fontFound;                          // 0x4bbea0

int CALLBACK FontFound(const LOGFONT *font, const TEXTMETRIC *, DWORD, LPARAM param)
{
    const ENUMLOGFONT *found = (const ENUMLOGFONT *)font;
    DebugLog("MESSAGE DEBUG: EnumFamCallBack, fullName %s, style %s.\n", found->elfFullName, found->elfStyle);
    ENUMLOGFONT *keep = (ENUMLOGFONT *)param;
    if (!g_fontFound) {
        memcpy(keep, found, sizeof(ENUMLOGFONT));
        g_fontFound = 1;
    }
    const char *keepStyle = (const char *)keep->elfStyle;
    if (_stricmp((const char *)found->elfStyle, "bold") == 0 && _stricmp(keepStyle, "bold") != 0)
        memcpy(keep, found, sizeof(ENUMLOGFONT));
    return 1;
}

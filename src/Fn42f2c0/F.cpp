// 0x42f2c0: StatusBar::LoadMessages - reset the counters and the face list, then load the "dead" and "won" message
// bitmaps named by [Interface] DeadMessage / WonMessage from <game dir>\ui\; 0 when a bitmap fails to load.
#include <string.h>
#include "IniReader/IniReader.h"
extern char kGameDirectory[];                               // 0x4c73d8
struct Counter { char d[0x34]; void Reset(); };             // 0x42f900
struct FaceList { void Clear(); };                          // 0x415b40
struct MessageBitmap { int Load(const char *path); };       // 0x4739d0
struct StatusBarM {
    Counter food;                                           // 0x00
    Counter amo;                                            // 0x34
    char pad68[4];
    FaceList faces;                                         // 0x6c
    char pad6d[0x37c - 0x6d];
    MessageBitmap *deadMessage;                             // 0x37c
    MessageBitmap *wonMessage;                              // 0x380
    int LoadMessages();
};

int StatusBarM::LoadMessages()
{
    food.Reset();
    amo.Reset();
    faces.Clear();
    IniReader ini;
    ini.SetSection("Interface");
    char dir[100];
    char name[52];
    char path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    strcpy(name, ini.GetString("DeadMessage"));
    if (deadMessage) {
        strcpy(path, dir);
        strcat(path, name);
        strcat(path, ".bmp");
        if (!deadMessage->Load(path))
            return 0;
    }
    strcpy(name, ini.GetString("WonMessage"));
    if (wonMessage) {
        strcpy(path, dir);
        strcat(path, name);
        strcat(path, ".bmp");
        if (!wonMessage->Load(path))
            return 0;
    }
    return 1;
}

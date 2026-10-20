// QuestItems::LoadCombined (SQZ.EXE 0x430360, ret 4): load the combined picture (the original always answers FALSE).
#include <string.h>
struct BitmapQ { int Load(const char *path); };   // 0x4739d0
extern char kGameDirectory[];                       // 0x4c73d8

struct QuestItemsQ {
    BitmapQ *combined;                              // 0x00
    char combinedFile[0x64];                        // 0x04
    int LoadCombined(int index);
};

int QuestItemsQ::LoadCombined(int)
{
    char dir[100], path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\ui\\");
    strcpy(path, dir);
    strcat(path, combinedFile);
    combined->Load(path);
    return 0;
}

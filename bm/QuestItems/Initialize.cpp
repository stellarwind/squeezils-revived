// 0x430150: load the key picture configuration for the selected place.
#include "QuestItems/QuestItems.h"
#include "IniReader/IniReader.h"
#include "Bitmap/Bitmap.h"
#include <stdlib.h>
#include <string.h>
#pragma intrinsic(strcpy, strcat)
struct QuestApplication { const char *GetPlaceName(int index); }; // 0x41e8c0
QuestApplication *GetQuestApplication(); // 0x420680
extern Renderer *g_questRenderer; // 0x4c75dc
int QuestItems::Init(int a, int b, int index)
{
    char iniPath[60];
    strcpy(iniPath, GetQuestApplication()->GetPlaceName(index));
    strcat(iniPath, "\\region\\region.ini");
    IniReader ini(iniPath);
    ini.SetSection("UIResources");
    strcpy(combinedFile, ini.GetString("CombinedKey"));
    char number[8];
    strcat(combinedFile, _itoa(index, number, 10));
    strcat(combinedFile, ".bmp");
    combined = new Bitmap(g_questRenderer);
    x = a;
    y = b;
    if (LoadCombined(index))
        return 1;
    LoadKeys();
    return 0;
}

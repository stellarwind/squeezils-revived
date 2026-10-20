// 0x42fca0: load the three key-piece sprites once, from region 1's resources.
#include "QuestItems/QuestItems.h"
#include "IniReader/IniReader.h"
#include "Bitmap/Bitmap.h"
#include <string.h>
#pragma intrinsic(strcpy, strcat)
struct QuestApplication { const char *GetPlaceName(int index); };
QuestApplication *GetQuestApplication();
extern Renderer *g_questRenderer;
extern char g_questGameDirectory[]; // 0x4c73d8
extern int g_questKeysLoaded; // 0x4bc01c
int QuestItems::LoadKeys()
{
    if (g_questKeysLoaded)
        return 1;
    char iniPath[60];
    strcpy(iniPath, GetQuestApplication()->GetPlaceName(1));
    strcat(iniPath, "\\region\\region.ini");
    IniReader ini(iniPath);
    ini.SetSection("UIResources");
    char directory[100], path[100];
    strcpy(directory, g_questGameDirectory);
    strcat(directory, "\\ui\\");

    strcpy(path, directory);
    const char *leftText = ini.GetString("LeftKey");
    char *leftName = leftFile;
    strcpy(leftName, leftText);
    strcat(leftName, ".bmp");
    strcat(path, leftName);
    left = new Bitmap(g_questRenderer);
    if (!((Bitmap *)left)->Load(path))
        return 0;

    strcpy(path, directory);
    const char *middleText = ini.GetString("MiddleKey");
    char *middleName = middleFile;
    strcpy(middleName, middleText);
    strcat(middleName, ".bmp");
    strcat(path, middleName);
    middle = new Bitmap(g_questRenderer);
    if (!((Bitmap *)middle)->Load(path))
        return 0;

    strcpy(path, directory);
    const char *rightText = ini.GetString("RightKey");
    char *rightName = rightFile;
    strcpy(rightName, rightText);
    strcat(rightName, ".bmp");
    strcat(path, rightName);
    right = new Bitmap(g_questRenderer);
    if (!((Bitmap *)right)->Load(path))
        return 0;
    g_questKeysLoaded = 1;
    return 1;
}

// Menu::LoadItemBitmap (0x406aa0): load a picture from the game's ui folder.
#include <string.h>

extern char g_gameDir[];       // 0x4c73d8
struct Bmp { int Load(const char *path); };   // 0x4739d0
struct MenuL { int LoadItemBitmap(const char *file, Bmp *bmp); };

int MenuL::LoadItemBitmap(const char *file, Bmp *bmp)
{
    char dir[100], path[100];
    strcpy(dir, g_gameDir);
    strcat(dir, "\ui\\");
    strcpy(path, dir);
    strcat(path, file);
    return bmp->Load(path) != 0;
}

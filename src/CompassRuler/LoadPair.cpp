// CompassRuler::LoadPair (SQZ.EXE 0x414d90, ret 8): load <name>.bmp and <name>1.bmp from the ui folder; frees everything on failure.
#include <string.h>
#include "Bitmap/Bitmap.h"
struct BitmapP;
extern char kGameDirectory[];                      // 0x4c73d8
struct CompassP {
    char pad[0xf4];
    char path[0x48];                               // 0xf4
    int FreeBitmaps();                             // 0x414fa0 (answers 0)
    int LoadPair(BitmapP **pair, const char *file);
};

int CompassP::LoadPair(BitmapP **pair, const char *file)
{
    strcpy(path, kGameDirectory);
    strcat(path, "\ui\\");
    char full[100];
    strcpy(full, path);
    strcat(full, file);
    strcat(full, ".bmp");
    if (!((Bitmap *)pair[0])->Load(full))
        return FreeBitmaps();
    strcpy(full, path);
    strcat(full, file);
    strcat(full, "1");
    strcat(full, ".bmp");
    if (!((Bitmap *)pair[1])->Load(full))
        return FreeBitmaps();
    return 1;
}

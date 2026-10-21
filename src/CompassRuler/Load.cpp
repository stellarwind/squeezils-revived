// CompassRuler::Load (SQZ.EXE 0x414c10): load the pictures of coast, desert, spacer, jungle and pine. A failure frees everything (0x414fa0, answers 0).
#include <string.h>
#include "Bitmap/Bitmap.h"
struct BitmapL {
    virtual void l0();
    virtual int CopyFrom(BitmapL *other);       // slot 1 (+4)
};
struct CompassL {
    BitmapL *picture[12];                       // 0x00
    char pad[0x7c - 0x30];
    char name[6][20];                           // 0x7c
    char path[0x48];                            // 0xf4
    int LoadPair(BitmapL **pair, const char *name);   // 0x414d90
    int FreeBitmaps();                          // 0x414fa0
    int Load();
};

int CompassL::Load()
{
    if (!LoadPair(&picture[2], name[1]))
        return FreeBitmaps();
    if (!LoadPair(&picture[4], name[2]))
        return FreeBitmaps();
    char full[100];
    strcpy(full, path);
    strcat(full, name[4]);
    strcat(full, ".bmp");
    if (!((Bitmap *)picture[10])->Load(full))
        return FreeBitmaps();
    if (!LoadPair(&picture[8], name[3]))
        return FreeBitmaps();
    if (!LoadPair(&picture[6], name[5]))
        return FreeBitmaps();
    if (!picture[11]->CopyFrom(picture[6]))
        return FreeBitmaps();
    return 1;
}

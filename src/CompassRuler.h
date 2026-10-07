// CompassRuler: the compass in the in-game panel. A strip of picture (the "ruler" bitmaps: c_ruler, d_ruler, j_ruler, p_ruler ...) that
// slides sideways as the player turns: the heading falls in one of eight 45 degree sectors, four of which show a place's emblem
// (pine, desert, coast, jungle) and the four between them the plain spacer, and the strip is made of the right part of the sector's
// picture followed by the left part of its neighbour's, so the emblem moves smoothly. Looking up or down scrolls it vertically.
// Which emblem shows for a place (and whether its second, "visited" picture is used) comes from the player's inventory table.
// Reconstructed from SQZ.EXE 0x414380-0x4150b0 and 0x4153e0 (the original called it the "region screen" in the panel; the object is
// 0x13c bytes at Panel + 0x480). It reads the ini files with the plain Windows profile functions, as the original does.
#pragma once
#include "Bitmap.h"

class CompassRuler {
public:
    CompassRuler();                     // @414380
    ~CompassRuler();                    // @4143d0 (0x4143c0 jumps to it)
    BOOL Init();                        // @414490 read the picture names ([Compass]) and which region number is which place ([RegionList])
    BOOL Load();                        // @414c10 load the pictures of coast, desert, spacer, jungle and pine
    BOOL LoadPair(Bitmap **pair, const char *name);   // @414d90 load <name>.bmp and <name>1.bmp from the ui folder; frees everything on failure
    BOOL UpdateSprites();               // @414af0 (thunk @4153e0) choose each sector's picture: the first, or the second once the place is done
    void SetCompass(float angle, float roll);          // @414fb0 build the strip for this heading and draw it on the panel
    BOOL Compose(int sector, int other, float fraction, float roll);   // @4150b0 the strip: this sector's picture slid by the fraction, then its neighbour's

    Bitmap *picture[12];                // 0x00 river (0, 1), coast (2, 3), desert (4, 5), pine (6, 7), jungle (8, 9) and the spacer (10); picture 11 is the strip that gets built (a copy of the pine picture in size)
    int     x;                          // 0x30 where the strip sits on the panel (261)
    int     y;                          // 0x34 (29)
    Bitmap *sprite[8];                  // 0x38 the picture for each sector: pine, spacer, desert, spacer, coast, spacer, jungle, spacer
    int     sector;                     // 0x58 the sector of the last heading
    int     sectorCount;                // 0x5c 8
    float   fullTurn;                   // 0x60 6.283185
    int     pineRegion;                 // 0x64 the region number (from [RegionList], minus 1) of each place whose emblem shows
    int     desertRegion;               // 0x68
    int     coastRegion;                // 0x6c
    int     jungleRegion;               // 0x70
    float   quarter;                    // 0x74 a quarter of a pi: the width of a sector
    float   eighth;                     // 0x78 an eighth of a pi
    char    name[6][20];                // 0x7c picture names from [Compass]: river 0x7c, coast 0x90, desert 0xa4, jungle 0xb8, spacer 0xcc, pine 0xe0
    char    path[0x48];                 // 0xf4 the ui folder, used while loading

private:
    void FreeBitmaps();
};
static_assert(sizeof(CompassRuler) == 0x13c, "CompassRuler layout");

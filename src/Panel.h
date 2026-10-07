// Panel: the in-game panel (the HUD along the screen edge: nut counters, mood face, quest items, compass, messages). One large
// object (0x6a8 bytes), created by the application's start-up code and kept in a global (0x4badb0). Its constructor (0x42d5f0)
// builds the widgets in the order below; the destructor (0x42d880) frees them in reverse. What is reconstructed so far: the
// constructor and destructor, the status bar, the quest panel, the nut counters, the mood face and the bitmap loader. The region
// screen (compass and region list), the goodie list and the font set-up are still original code, reached through Original.h.
#pragma once
#include "Bitmap.h"
#include "StatusBar.h"
#include "QuestPanel.h"

struct Float3 { float x, y, z; };

struct Panel {
    int        originX;            // 0x000 screen position the panel is placed from (GetScreenOrigin, taken at construction)
    int        originY;            // 0x004
    StatusBar  status;             // 0x008 nut counters, goodie list, mood face, dead / won messages
    QuestPanel quest;              // 0x394 quest item pictures and the places-completed counter
    Bitmap    *digits[10];         // 0x428 the digit pictures numb_0.bmp .. numb_9.bmp used by the counters
    int        field450;           // 0x450 2
    int        field454;           // 0x454 2
    int        field458;           // 0x458 -1
    int        field45c;           // 0x45c 0
    Bitmap    *background;         // 0x460 the panel's own picture
    char       backgroundFile[20]; // 0x464 its file name, from [Interface] Panel
    int        field478;           // 0x478 0
    int        field47c;           // 0x47c 0
    char       regionScreen[0x13c];// 0x480 the region screen widget (original code) starts here; the rest of this area is not mapped yet
    float      field5bc;           // 0x5bc 0.01
    float      verticalChange;     // 0x5c0 [Compass] VerticalChange (default 1.0)
    float      verticalThreshold;  // 0x5c4 [Compass] VerticalThreshold (default 0.1)
    Float3     vec5c8, vec5d4, vec5e0, vec5ec, vec5f8;   // 0x5c8 compass working vectors (their constructors do nothing)
    Float3     vec604;             // 0x604 (-5, -5, -5)
    float      field610;           // 0x610 6.283185 (a full turn)
    int        field614;           // 0x614 0
    HFONT      font;               // 0x618 the message font made by the font set-up (deleted with the panel), 0 until then
    char       pad61c[20];         // 0x61c
    char       messageFont[100];   // 0x630 first byte cleared at construction (font name from [Interface] FontName)
    char       pad694[20];         // 0x694 first byte cleared at construction

    Panel();                       // @42d5f0
    ~Panel();                      // @42d880
    BOOL LoadBitmaps();            // @42e500 read the panel picture, the region screen's pictures and the ten digits
};
static_assert(sizeof(Panel) == 0x6a8, "Panel layout");

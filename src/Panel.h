// Panel: the in-game panel (the HUD along the screen edge: nut counters, mood face, quest items, compass, messages). One large
// object (0x6a8 bytes), created by the application's start-up code and kept in a global (0x4badb0). Its constructor (0x42d5f0)
// builds the widgets in the order below; the destructor (0x42d880) frees them in reverse. Everything the panel owns is reconstructed:
// the status bar (counters, goodie list, mood face), the quest panel, the compass ruler, the message text and fonts.
#pragma once
#include "Bitmap.h"
#include "StatusBar.h"
#include "QuestPanel.h"
#include "CompassRuler.h"

#include "Float3.h"

struct Panel {
    int        originX;            // 0x000 screen position the panel is placed from (GetScreenOrigin, taken at construction)
    int        originY;            // 0x004
    StatusBar  status;             // 0x008 nut counters, goodie list, mood face, dead / won messages
    QuestPanel quest;              // 0x394 quest item pictures and the places-completed counter
    Bitmap    *digits[10];         // 0x428 the digit pictures numb_0.bmp .. numb_9.bmp used by the counters
    int        mode;               // 0x450 what the panel shows: 0 the status bar, 1 the quest items (starts at 2: nothing yet)
    int        lastMode;           // 0x454 the mode last drawn (a change forces a redraw)
    int        questKey;           // 0x458 the place whose quest items are shown (-1 none)
    int        dirty;              // 0x45c set to make the next Update redraw everything
    Bitmap    *background;         // 0x460 the panel's own picture
    char       backgroundFile[20]; // 0x464 its file name, from [Interface] Panel
    int        backgroundX;        // 0x478 where the panel picture sits, from the screen origin
    int        backgroundY;        // 0x47c
    CompassRuler compass;          // 0x480 the compass strip (the original called this the region screen)
    float      turnThreshold;      // 0x5bc 0.01: the smallest change of heading (radians) the compass bothers to follow
    float      verticalChange;     // 0x5c0 [Compass] VerticalChange (default 1.0)
    float      verticalThreshold;  // 0x5c4 [Compass] VerticalThreshold (default 0.1)
    Float3     row0;               // 0x5c8 the player's matrix, first row (the heading comes from its z and the third row's z)
    Float3     position;           // 0x5d4 the matrix's position row (only its height is used)
    Float3     row2;               // 0x5e0 the matrix, third row
    Float3     prevRow0;           // 0x5ec last frame's copies of the three rows (written, never read back)
    Float3     prevPosition;       // 0x5f8
    Float3     prevRow2;           // 0x604 (-5, -5, -5 at construction)
    float      fullTurn;           // 0x610 6.283185, a full turn in radians (added to a negative heading)
    int        event;              // 0x614 set by the goodie list while an animation runs (1), cleared when it ends
    HFONT      font;               // 0x618 the message font made by the font set-up (deleted with the panel), 0 until then
    int        fontHeight;         // 0x61c line height the font was made for (pixels), used to space the lines
    int        messageX;           // 0x620 [Interface] Messagex where the message line starts (screen)
    int        messageY;           // 0x624 Messagey
    int        messageWidth;       // 0x628 MaxMessageWidth
    int        messageLines;       // 0x62c MaxMessageHeight / fontHeight
    char       message[100];       // 0x630 the message on show (from [Interface] Message at start-up)
    char       note[20];           // 0x694 a short note shown below it

    Panel();                       // @42d5f0
    ~Panel();                      // @42d880
    BOOL LoadBitmaps();            // @42e500 read the panel picture, the region screen's pictures and the ten digits
    BOOL Load();                   // @42d9f0 load the bitmaps, set up the status bar, load the quest items
    BOOL Update(int force);        // @42db10 once a frame: redraw what changed (everything when force or dirty)
    BOOL DrawBackground();         // @42dc50 put the panel picture on the screen and remember where the screen starts
    void ShowQuest(int key);       // @42da60 switch to the quest items of place key
    void ShowStatus(int foodGoal);    // @42dac0 switch to the status bar; foodGoal is the second food number
    void Invalidate() { dirty = 1; }   // @42db00
    void ClearMessage() { message[0] = 0; }   // @42dd00
    void ClearNote() { note[0] = 0; }         // @42dcd0
    void ClearEvent() { event = 0; }         // @42ea60
    void StartGoodieAnimation(int steps, float seconds);   // @42e6f0 the goodie list animates for a while (the event flag is set until it ends)
    BOOL UpdateRegionSprites() { return compass.UpdateSprites(); }   // @42e2a0 (the game calls this when a place is finished: show the compass emblems again)
    void UpdateCompass(int force);             // @42e2b0 turn the compass to the player's heading, with the up and down roll animation (PanelCompass.cpp)
    BOOL DrawMessage(const char *text);        // @42dd30 word-wrapped text in the message area, over the cleaned panel picture
    BOOL DrawNote(const char *text);           // @42e0e0 the short note
};
static_assert(sizeof(Panel) == 0x6a8, "Panel layout");

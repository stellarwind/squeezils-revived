// Splash: the load screen. A full-screen picture (ui\<name>.bmp, named by the ini file of the place or region being loaded)
// with a progress bar made of repeated slices of ui\loadbar.bmp drawn straight onto the primary surface. Reconstructed from
// SQZ.EXE 0x44e900-0x44f080 (RTTI class Splash, vtable 0x49ed34). The application creates one at start and keeps it in the
// global at 0x4c75e8; original code calls SetProgress and Step through the vtable, so the object carries the original vtable
// and the two virtual methods are plain methods here (slots 0 and 1).
#pragma once
#include "Bitmap.h"

class Splash {
public:
    Splash();                                  // @44e900
    ~Splash();                                 // @44e9e0

    // slot 0 @44f010: move the bar to the fraction p (0..1), drawing the slices in between
    void SetProgress(float p);
    // slot 1 @44ef30: draw the next slice of the bar if the bar is behind its target
    void Step();

    BOOL Load(int which);                      // @44ea30 show the picture for a level: 0 = current ini file, 1 = place, 2 = region
    void Hide();                               // @44ee40 finish the bar, free the picture
    int  IsShown() const { return shown; }     // @44eea0
    void Flush();                              // @44ef00 draw the slices still owed (step times) and reset
    void Show();                               // @44edd0 draw the picture on the primary surface
    void Reset() { step = 0; }                 // @44f080

    void SetFile(int which);                   // @44eb20 work out the picture's file name into file
    void Pump();                               // @44eeb0 take the window's pending message, repaint and restart the bar

    void  **vtbl;          // 0x00 0x49ed34
    Bitmap *picture;       // 0x04 the full-screen picture, made on demand
    Bitmap *bar;           // 0x08 one slice of the bar (ui\loadbar.bmp)
    int     step;          // 0x0c slices drawn so far
    int     steps;         // 0x10 slices in the whole bar (445 pixels / slice width)
    int     barX;          // 0x14 where the bar starts on screen (98)
    int     barY;          // 0x18 (362)
    int     shown;         // 0x1c the picture has been loaded
    char    file[0x3c];    // 0x20 picture path, built by SetFile
    int     which;         // 0x5c what the picture was loaded for (-1 = none)
    float   target;        // 0x60 fraction the bar is heading for (1.0 = full)
};
static_assert(sizeof(Splash) == 0x64, "Splash layout");

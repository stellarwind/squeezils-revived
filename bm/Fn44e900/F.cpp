// Splash::Splash (0x44e900): loads the load-bar bitmap and derives the horizontal scale from its width.
#include "Bitmap2/Bitmap2.h"
extern Renderer *g_renderer;                         // 0x4c75dc
struct SplashC {
    virtual void v0();
    Bitmap *picture;     // 0x04
    Bitmap *bar;         // 0x08
    int step;            // 0x0c
    int scale;           // 0x10
    int a14, a18;        // 0x14, 0x18
    int shown;           // 0x1c
    char pad[0x5c - 0x20];
    int a5c;             // 0x5c
    float a60;           // 0x60
    SplashC();
};
SplashC::SplashC()
{
    picture = 0;
    shown = 0;
    bar = 0;
    step = 0;
    a14 = 0x62;
    a18 = 0x16a;
    bar = new Bitmap(g_renderer);
    if (bar)
        bar->Load(".\\ui\\loadbar.bmp");
    int w = bar->GetWidth();
    a5c = -1;
    a60 = 1.0f;
    scale = 0x1bd / w;
}

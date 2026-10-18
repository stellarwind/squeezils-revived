// Panel::DrawBackground, SQZ.EXE 0x42dc50.
// Spare declarations preserve VC++ 4.1 register allocation.
void BackgroundPad1();
void BackgroundPad2();
#include "Panel/Panel.h"
#include "Bitmap/Bitmap.h"

struct PanelRenderer : Renderer {
    void GetScreenRect(long *rect); // 0x474fe0
};
extern PanelRenderer *gPanelRenderer; // 0x4c75dc

int Panel::DrawBackground()
{
    IDirectDrawSurface *screen = gPanelRenderer->primary;
    if (!screen)
        return 0;
    long rect[4];
    gPanelRenderer->GetScreenRect(rect);
    originY = rect[3];
    originX = rect[0];
    if (background)
        background->DrawToSurfaceAt(screen, backgroundX + originX,
            backgroundY + originY, 0, 0, 1);
    return 1;
}

// CompassRuler::SetCompass (SQZ.EXE 0x414fb0, ret 8): build the strip for this heading and draw it on the panel.
#include <math.h>
#include "CompassC.h"
struct RendererC { char pad[0x994]; SurfaceC *primary; };
extern RendererC *g_renderer;                   // 0x4c75dc
struct PanelC { void GetScreenOrigin(int *x, int *y); };   // 0x42da30
PanelC *GetPanel();                             // 0x4206c0

void CompassC::SetCompass(float angle, float roll)
{
    SurfaceC *screen = g_renderer->primary;
    if (!screen)
        return;
    double whole;
    double fraction = modf((double)angle / (double)quarter, &whole);
    int s = (int)whole;
    int n = sectorCount;
    sector = s;
    if (sector >= n)
        sector = 0;
    if (sector < 0)
        sector = n - 1;
    s = sector;
    int other = s + 1;
    if (!((s + 1) * eighth < angle)) {
        other = s - 1;
        if (!(quarter > angle))
            other = s;
    }
    if (other >= n)
        other = 0;
    if (other < 0)
        other = n - 1;
    Compose(s, other, (float)fabs(fraction), roll);
    int ox, oy;
    GetPanel()->GetScreenOrigin(&ox, &oy);
    picture[11]->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
}

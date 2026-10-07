// Splash: see Splash.h.
#include <string.h>
#include <stdio.h>
#include <new>
#include "Splash.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"

static const float kOne = 1.0f;
// The two simulation objects SetFile reads: the place being loaded (index into the place name list and the level number) and
// the region (index into the same list). They are big game classes, so only these fields are modelled.
struct PlaceSimulationFields  { char pad[0x24]; int placeIndex; int level; };
struct RegionSimulationFields { char pad[0x24]; int placeIndex; };

Splash::Splash()
{
    vtbl = (void **)kSplashVtable;
    picture = NULL;
    shown = 0;
    bar = NULL;
    step = 0;
    barX = 0x62;
    barY = 0x16a;
    void *m = Operator_New(sizeof(Bitmap));
    bar = m ? new (m) Bitmap(GetRenderer()) : NULL;
    if (bar) bar->Load(".\\ui\\loadbar.bmp");
    which = -1;
    target = kOne;
    steps = 0x1bd / bar->GetWidth();
}

Splash::~Splash()
{
    vtbl = (void **)kSplashVtable;
    if (picture) {
        picture->~Bitmap();
        Operator_Delete(picture);
    }
    if (bar) {
        bar->~Bitmap();
        Operator_Delete(bar);
    }
    shown = 0;
}

void Splash::Pump()
{
    Renderer *r = GetRenderer();
    MSG msg;
    if (PeekMessageA(&msg, r->hwnd, 0, 0, PM_REMOVE)) {
        ValidateRect(GetRenderer()->hwnd, NULL);
        Show();
        step = 0;
    }
}

void Splash::Show()
{
    if (!picture) return;
    shown = 1;
    POINT pt = { 0, 0 };
    Renderer *r = GetRenderer();
    if (r->fullscreen == 0)
        ClientToScreen(r->hwnd, &pt);
    picture->DrawToSurfaceAt(GetRenderer()->primary, pt.x, pt.y, 0, 0, 0);
}

void Splash::Hide()
{
    target = kOne;
    if (bar) {
        while (step < steps) {
            Step();
            if (!bar) break;
        }
    }
    if (picture) {
        picture->~Bitmap();
        Operator_Delete(picture);
        picture = NULL;
    }
    shown = 0;
    Reset();
    which = -1;
}

BOOL Splash::Load(int w)
{
    target = kOne;
    if (picture && which != w)
        Hide();
    if (!picture) {
        void *m = Operator_New(sizeof(Bitmap));
        picture = m ? new (m) Bitmap(GetRenderer()) : NULL;
        if (!picture) {
            which = w;
            return TRUE;
        }
    }
    if (!shown) {
        SetFile(w);
        if (!picture->Load(file))
            return FALSE;
    }
    if (which != w)
        Show();
    which = w;
    return TRUE;
}

void Splash::Flush()
{
    int n = step;
    step = 0;
    for (; n > 0; n--)
        Step();
}

void Splash::Step()
{
    if (!bar) return;
    int i = step;
    if (i >= steps) return;
    if (which == -1) {
        step = steps;
        return;
    }
    if (GetRenderer()->fullscreen == 0)
        Pump();
    if (!((double)step / (double)steps < (double)target))      // the original compares in x87 precision
        return;
    POINT pt = { 0, 0 };
    Renderer *r = GetRenderer();
    if (r->fullscreen == 0)
        ClientToScreen(r->hwnd, &pt);
    LPDIRECTDRAWSURFACE primary = GetRenderer()->primary;
    if (step > i) return;
    do {
        bar->DrawToSurfaceAt(primary, pt.x + bar->GetWidth() * step + barX, barY + pt.y, 0, 0, 1);
        step++;
    } while (i >= step);
}

void Splash::SetProgress(float p)
{
    unsigned tb, ob;
    memcpy(&tb, &target, 4);
    memcpy(&ob, &kOne, 4);
    bool behind = target < p;
    if (!behind && (int)tb < (int)ob) {
        Step();
        return;
    }
    if (tb != ob) {
        while ((double)step / (double)steps < (double)target)
            Step();
    }
    target = p;
    Step();
}

void Splash::SetFile(int w)
{
    char path[256];
    if (w == 0) {
        strcpy(path, kCurrentIniPath);
    } else if (w == 1) {
        PlaceSimulationFields *sim = (PlaceSimulationFields *)DynamicCast(GetCurrentSimulation(), kLocationSimulationType, kPlaceSimulationType);
        strcpy(path, GetPlaceName(sim->placeIndex));
        strcat(path, "\\place");
        char num[36];
        sprintf(num, "%02d", sim->level);
        strcat(path, num);
        strcat(path, "\\place.ini");
    } else if (w == 2) {
        RegionSimulationFields *sim = (RegionSimulationFields *)DynamicCast(GetCurrentSimulation(), kLocationSimulationType, kRegionSimulationType);
        strcpy(path, GetPlaceName(sim->placeIndex));
        strcat(path, "\\region\\region.ini");
    } else {
        path[0] = 0;                                  // the original leaves the buffer unset for any other value
    }
    strcpy(file, ".\\ui\\");
    IniReader ini(path);
    ini.SetSection(kDefaultSectionName);
    const char *name = ini.GetString("Splash");
    if (strlen(name) == 0)
        name = "splash";
    strcat(file, name);
    strcat(file, ".bmp");
}

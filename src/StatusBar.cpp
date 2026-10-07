// StatusBar: see StatusBar.h.
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include <new>
#include "StatusBar.h"
#include "Panel.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"

static Bitmap *NewBitmap()
{
    void *m = Operator_New(sizeof(Bitmap));
    return m ? new (m) Bitmap(GetRenderer()) : NULL;
}

BOOL StatusBar::Init()
{
    amo.Init("AmoNut", 25, 20, 0);
    amoCount = 0;
    amoShown = -1;
    food.Init("FoodNut", 510, 10, 1);
    foodCount = 0;
    foodShown = -1;
    if (!goodies.Init(410, 20))
        return FALSE;
    if (!face.Load(150, 14))
        return FALSE;
    deadMessage = NewBitmap();
    messageX = 405;
    messageY = 12;
    wonMessage = NewBitmap();
    return LoadAll() != 0;
}

BOOL StatusBar::LoadAll()
{
    amo.Load();
    food.Load();
    goodies.Load();
    IniReader ini;
    ini.SetSection("Interface");
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    char name[100], path[100];
    strcpy(name, ini.GetString("DeadMessage"));
    if (deadMessage) {
        strcpy(path, dir);
        strcat(path, name);
        strcat(path, ".bmp");
        if (!deadMessage->Load(path))
            return FALSE;
    }
    strcpy(name, ini.GetString("WonMessage"));
    if (wonMessage) {
        strcpy(path, dir);
        strcat(path, name);
        strcat(path, ".bmp");
        if (!wonMessage->Load(path))
            return FALSE;
    }
    return TRUE;
}

static void FreeBitmap(Bitmap *b)
{
    if (b) {
        b->~Bitmap();
        Operator_Delete(b);
    }
}

StatusBar::StatusBar()
{
    foodGoal = 0;
    goodieStep = goodieSteps = goodieInterval = goodieFlip = faceState = 0;
    goodieTick = 0;
}

StatusBar::~StatusBar()
{
    FreeBitmap(deadMessage);
    FreeBitmap(wonMessage);
}

int StatusBar::UpdateFace(int force, Bitmap *panel)
{
    faceState = face.Draw(force, panel);
    return faceState;
}

BOOL StatusBar::UpdateAmo(int force)
{
    if (!GoodieCount("SpitNutGoodie", &amoCount))
        return FALSE;
    if (force || amoCount != amoShown)
        amo.Draw(amoCount, 0);
    BOOL changed = amoShown != amoCount;
    amoShown = amoCount;
    return changed;
}

void StatusBar::SetFoodGoal(int value)
{
    foodGoal = value;
}

BOOL StatusBar::UpdateFood(int force)
{
    if (!GoodieCount("FoodNutGoodie", &foodCount))
        return FALSE;
    if (force || foodCount != foodShown) {
        food.Draw(foodCount, foodGoal);
        StoreSetting("PlayerFoodNuts", &foodCount, -1);
    }
    BOOL changed = foodShown != foodCount;
    foodShown = foodCount;
    return changed;
}

BOOL StatusBar::Update(int force, int *event, Bitmap *panel)
{
    UpdateFace(force, panel);
    UpdateGoodies(force, event, panel);
    UpdateAmo(force);
    UpdateFood(force);
    if (!faceState) {
        *kMessageShown = 0;
        return TRUE;
    }
    if (!force && *kMessageShown)
        return TRUE;
    *kMessageShown = 1;
    int x, y;
    GetScreenOrigin(&x, &y);
    LPDIRECTDRAWSURFACE screen = GetRenderer()->primary;
    if (faceState == 1) {
        ((Panel *)GetPanel())->DrawMessage(kEmptyString);
        deadMessage->DrawToSurfaceAt(screen, messageX + x, messageY + y, 0, 0, 1);
    } else {
        wonMessage->DrawToSurfaceAt(screen, messageX + x, messageY + y, 0, 0, 1);
    }
    return TRUE;
}

void StatusBar::SetRound(int steps, float seconds)
{
    goodieSteps = steps;
    goodieStep = 0;
    goodieInterval = (int)((double)seconds * 1000.0);
}

BOOL StatusBar::UpdateGoodies(int force, int *event, Bitmap *panel)
{
    BOOL due = FALSE;
    DWORD now = 0;
    if (*event == 1) {
        now = timeGetTime();
        if (now - goodieTick > (unsigned)goodieInterval)
            due = TRUE;
    }
    static String lastName(kEmptyString);               // the goodie shown last time
    String current;
    if (!CurrentGoodieName(&current))
        return FALSE;
    if (StringCompare(&current, &lastName))
        force = 1;
    lastName = current;
    if (force || due) {
        goodies.Draw(goodieFlip, current, panel);
        if (due) {
            goodieFlip = goodieFlip == 0;
            goodieStep++;
            if (goodieSteps <= goodieStep) {                // the round is over
                goodieStep = 0;
                goodieTick = 0;
                goodieFlip = 0;
                *event = 0;
            }
            goodieTick = now;
        }
    }
    return TRUE;
}

// StatusBar: see StatusBar.h.
#include <string.h>
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
    amoA = 0;
    amoB = -1;
    food.Init("FoodNut", 510, 10, 1);
    foodA = 0;
    foodC = -1;
    if (!GoodieListInit(goodies, 410, 20))
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
    GoodieListLoad(goodies);
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
    GoodieListCtor(goodies);
    foodB = 0;
    goodieStep = goodieSteps = goodieInterval = goodieFlip = faceState = 0;
    goodieTick = 0;
}

StatusBar::~StatusBar()
{
    FreeBitmap(deadMessage);
    FreeBitmap(wonMessage);
    GoodieListDtor(goodies);
}

int StatusBar::UpdateFace(int force, Bitmap *panel)
{
    faceState = face.Draw(force, panel);
    return faceState;
}

BOOL StatusBar::UpdateAmo(int force)
{
    if (!GoodieCount("SpitNutGoodie", &amoA))
        return FALSE;
    if (force || amoA != amoB)
        amo.Draw(amoA, 0);
    BOOL changed = amoB != amoA;
    amoB = amoA;
    return changed;
}

void StatusBar::SetFoodB(int value)
{
    foodB = value;
}

BOOL StatusBar::UpdateFood(int force)
{
    if (!GoodieCount("FoodNutGoodie", &foodA))
        return FALSE;
    if (force || foodA != foodC) {
        food.Draw(foodA, foodB);
        StoreSetting("PlayerFoodNuts", &foodA, -1);
    }
    BOOL changed = foodC != foodA;
    foodC = foodA;
    return changed;
}

BOOL StatusBar::Update(int force, int *event, Bitmap *panel)
{
    UpdateFace(force, panel);
    GoodiesUpdate(this, force, event, panel);
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

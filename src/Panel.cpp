// Panel: see Panel.h.
#include <string.h>
#include <stdio.h>
#include <new>
#include "Panel.h"
#include "IniReader.h"
#include "Original.h"

BOOL Panel::LoadBitmaps()
{
    IniReader ini;
    ini.SetSection("Interface");
    strcpy(backgroundFile, ini.GetString("Panel"));
    GameLog("reading Panel bitmaps \n");
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    if (background) {
        char path[100];
        strcpy(path, dir);
        strcat(path, backgroundFile);
        if (!background->Load(path))
            return FALSE;
    }
    if (!RegionScreenLoad(regionScreen))
        return FALSE;
    for (int i = 0; i < 10; i++) {
        char path[100];
        sprintf(path, "%snumb_%d.bmp", dir, i);
        if (!digits[i]->Load(path))
            return FALSE;
    }
    GameLog("Panel bitmaps Read\n");
    return TRUE;
}

static Bitmap *NewBitmap()
{
    void *m = Operator_New(sizeof(Bitmap));
    return m ? new (m) Bitmap(GetRenderer()) : NULL;
}

static void FreeBitmap(Bitmap *b)
{
    if (b) {
        b->~Bitmap();
        Operator_Delete(b);
    }
}

static float FloatFromBits(unsigned bits)
{
    float f;
    memcpy(&f, &bits, sizeof f);
    return f;
}

Panel::Panel()
{
    // status and quest are built first (their constructors run before this body), then the region screen
    RegionScreenCtor(regionScreen);
    background = NewBitmap();
    backgroundX = 0;
    backgroundY = 0;
    RegionScreenInit(regionScreen);
    for (int i = 0; i < 10; i++) {
        digits[i] = 0;
        digits[i] = NewBitmap();
    }
    GetScreenOrigin(&originX, &originY);
    prevRow2.x = prevRow2.y = prevRow2.z = -5.0f;
    fullTurn = FloatFromBits(0x40c90fd0);
    turnThreshold = 0.01f;
    IniReader ini;
    ini.SetSection("Compass");
    verticalChange = ini.GetFloat("VerticalChange", 1.0f);
    verticalThreshold = ini.GetFloat("VerticalThreshold", 0.1f);
    mode = 2;
    lastMode = 2;
    dirty = 0;
    event = 0;
    questKey = -1;
    font = NULL;
    PanelLoadFonts(this);
    message[0] = 0;
    note[0] = 0;
}

Panel::~Panel()
{
    FreeBitmap(background);
    for (int i = 0; i < 10; i++)
        FreeBitmap(digits[i]);
    if (font)
        DeleteObject(font);
    RegionScreenDtor(regionScreen);        // then quest and status are destroyed, in that order
}

BOOL Panel::Load()
{
    if (!LoadBitmaps())
        return FALSE;
    if (!status.Init())
        return FALSE;
    return QuestPanelLoad(&quest) != 0;
}

BOOL Panel::DrawBackground()
{
    Renderer *r = GetRenderer();
    LPDIRECTDRAWSURFACE screen = r->primary;
    if (!screen)
        return FALSE;
    LONG rect[4];
    r->GetScreenRect(rect);
    originX = rect[0];
    originY = rect[3];
    if (background)
        background->DrawToSurfaceAt(screen, backgroundX + originX, backgroundY + originY, 0, 0, 1);
    return TRUE;
}

void Panel::ShowQuest(int key)
{
    if (questKey != key) {
        quest.items.FreeCombined();
        quest.items.Init(30, 5, key);
        dirty = 1;
    }
    questKey = key;
    mode = 1;
    ClearMessage();
    ClearNote();
}

void Panel::ShowStatus(int foodGoal)
{
    mode = 0;
    status.SetFoodGoal(foodGoal);
    dirty = 1;
    ClearMessage();
    ClearNote();
}

BOOL Panel::Update(int force)
{
    if (GetRenderer()->field_078)
        return TRUE;
    int redraw;
    if (lastMode != mode) {
        redraw = 1;
        lastMode = mode;
    } else {
        redraw = force;
    }
    if (dirty)
        redraw = 1;
    dirty = 0;
    if (*kPanelDrawn && !redraw) {
        UpdateCompass(0);
    } else {
        DrawBackground();
        UpdateCompass(1);
        *kPanelDrawn = 1;
    }
    if (mode == 0) {
        int before = event;
        status.Update(redraw, &event, background);
        if (before == 1 && event == 0)
            ClearEvent();
        if (redraw) {
            if (!status.faceState) {
                GameLog("SETTING : %s \n", message);
                DrawMessage(message);
            }
            if (strlen(note))
                DrawNote(note);
        }
    } else {
        QuestPanelDraw(&quest, redraw, questKey);
        if (redraw)
            DrawMessage(message);
    }
    return TRUE;
}

void Panel::StartGoodieAnimation(int steps, float seconds)
{
    event = 1;
    status.SetRound(steps, seconds);
}

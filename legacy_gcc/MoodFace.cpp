// MoodFace: see MoodFace.h.
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>
#include "MoodFace.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"
#include <mmsystem.h>

MoodFace::MoodFace()
{
    for (int a = 0; a < MAX_ANIMATIONS; a++)
        for (int f = 0; f < MAX_FRAMES; f++)
            animation[a].frames[f] = NULL;
    frame = -1;
    current = -1;
    lastMood = -1;
    canvas = NULL;
    background = NULL;
    width = 0;
    height = 0;
    lastTick = 0;
}

MoodFace::~MoodFace()
{
    for (int a = 0; a < MAX_ANIMATIONS; a++) {
        for (int f = 0; f < MAX_FRAMES; f++) {
            Bitmap *b = animation[a].frames[f];
            if (b) {
                b->~Bitmap();
                Operator_Delete(b);
            }
        }
    }
    if (canvas) {
        canvas->~Bitmap();
        Operator_Delete(canvas);
    }
    if (background) {
        background->~Bitmap();
        Operator_Delete(background);
    }
}

static Bitmap *NewBitmap()
{
    void *m = Operator_New(sizeof(Bitmap));
    return m ? new (m) Bitmap(GetRenderer()) : NULL;
}

BOOL MoodFace::Load(int px, int py)
{
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    IniReader ini;
    ini.SetSection("Interface");
    float rate = ini.GetFloat("FrameRate", 0.5f);
    frameMs = (float)((long double)rate * 1000.0f);
    animationCount = atoi(ini.GetString("Animations"));
    for (int n = 1; n <= animationCount && n <= MAX_ANIMATIONS; n++) {
        Animation &an = animation[n - 1];
        char key[100], file[100], num[20];
        strcpy(key, "Animation");
        sprintf(num, "%d", n);
        strcat(key, num);
        an.frameCount = atoi(ini.GetString(key));
        char base[100], nameKey[100];
        strcpy(base, key);
        strcpy(nameKey, base);
        strcat(nameKey, "Name");
        strcpy(an.name, ini.GetString(nameKey));
        for (int f = 0; f < an.frameCount && f < MAX_FRAMES; f++) {
            an.frames[f] = NewBitmap();
            strcpy(key, base);
            strcat(key, "_");
            sprintf(num, "%d", f + 1);
            strcat(key, num);
            strcpy(file, dir);
            strcat(file, ini.GetString(key));
            strcat(file, ".bmp");
            if (!an.frames[f]->Load(file))
                return FALSE;
            if (an.frames[f]->GetWidth() > width) width = an.frames[f]->GetWidth();
            if (an.frames[f]->GetHeight() > height) height = an.frames[f]->GetHeight();
        }
    }
    x = px;
    y = py;
    return TRUE;
}

// The animation a mood plays, by name. Moods the table leaves out play "undefined".
static const char *AnimationName(int mood)
{
    switch (mood) {
    case 0: case 10:                       return "Ambivalent";
    case 1: case 22:                       return "Sleeping";
    case 2: case 3:                        return "Alert";
    case 4: case 9: case 17:               return "Happy";
    case 6: case 8: case 11: case 19:      return "Surprise";
    case 7: case 20: case 21:              return "Anger";
    case 12: case 13: case 14:             return "Jumping";
    case 16:                               return "Spitting";
    default:                               return "undefined";      // 5, 15, 18 and anything above 22
    }
}

int MoodFace::FindAnimation(int mood)
{
    const char *want = AnimationName(mood);
    for (int i = 0; i < animationCount; i++)
        if (strcmp(animation[i].name, want) == 0)
            return i;
    return 0;
}

int MoodFace::Draw(int force, Bitmap *panel)
{
    if (GetRenderer()->field_078) return 0;
    void *soul = GetPlayerSoul();
    if (!soul) return 0;
    int mood = SoulMood(soul);
    int asleep = mood == 22;
    if (mood == lastMood && animation[current].frameCount == 1 && !force)
        return asleep;
    if (mood == lastMood && animation[current].frameCount > 1) {
        long long elapsed = (long long)(timeGetTime() - lastTick);
        if (!((double)elapsed > (double)frameMs))
            return asleep;
        frame++;
        if (animation[current].frameCount <= frame)
            frame = 0;
    } else {
        frame = 0;
        int row = FindAnimation(mood);
        if (row == current && animation[current].frameCount == 1 && !force)
            return asleep;
        current = row;
    }
    LPDIRECTDRAWSURFACE primary = GetRenderer()->primary;
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    if (!canvas) {
        canvas = NewBitmap();
        canvas->CreateSurfaceSized(width, height);
        canvas->CaptureFrom(panel->GetSurface(), x, y);
        background = NewBitmap();
        background->CreateSurfaceSized(width, height);
        background->CaptureFrom(panel->GetSurface(), x, y);
    }
    background->DrawToSurfaceAt(canvas->GetSurface(), 0, 0, 0, 0, 0);
    animation[current].frames[frame]->DrawToSurfaceAt(canvas->GetSurface(), 0, 0, 0, 0, 1);
    canvas->DrawToSurfaceAt(primary, x + ox, y + oy, 0, 0, 0);
    lastTick = timeGetTime();
    lastMood = mood;
    return asleep;
}

void GetScreenOrigin(int *px, int *py)
{
    LONG r[4];
    GetRenderer()->GetScreenRect(r);
    *px = r[0];
    *py = r[3];
}

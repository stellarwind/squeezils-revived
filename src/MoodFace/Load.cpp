// SQZ.EXE 0x405520: load the configured named face animation bitmaps.
#include <string.h>
#include <stdlib.h>
#include "MoodFace.h"
#include "Bitmap2/Bitmap2.h"
#include "IniReader/IniReader.h"
extern Renderer *g_renderer;                       // 0x4c75dc
extern char g_gameDirectory[];                     // 0x4c73d8

int MoodFace::Load(int px, int py)
{
    char dir[100];
    strcpy(dir, g_gameDirectory);
    strcat(dir, "\\ui\\");
    IniReader ini;
    ini.SetSection("Interface");
    frameMs = ini.GetFloat("FrameRate", 0.5f);
    frameMs *= 1000.0f;
    animationCount = atoi(ini.GetString("Animations"));
    for (int n=0; n<animationCount; n++) {
        char key[32], number[8], base[52], nameKey[20], file[100];
        strcpy(key, "");
        strcat(key, "Animation");
        _itoa(n+1, number, 10);
        strcat(key, number);
        Animation &an = animation[n];
        an.frameCount = atoi(ini.GetString(key));
        strcpy(base, key);
        strcpy(nameKey, base);
        strcat(nameKey, "Name");
        strcpy(an.name, ini.GetString(nameKey));
        for (int f=0; f<an.frameCount; f++) {
            an.frames[f] = new Bitmap(g_renderer);
            strcat(key, "_");
            _itoa(f+1, number, 10);
            strcat(key, number);
            const char *name = ini.GetString(key);
            strcpy(file, dir);
            strcat(file, name);
            strcat(file, ".bmp");
            if (!an.frames[f]->Load(file)) return 0;
            strcpy(key, base);
            if (an.frames[f]->GetWidth()>width) width=an.frames[f]->GetWidth();
            if (an.frames[f]->GetHeight()>height) height=an.frames[f]->GetHeight();
        }
    }
    x=px; y=py;
    return 1;
}

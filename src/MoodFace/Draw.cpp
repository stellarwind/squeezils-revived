#include "Runtime/DynamicCast.h"
// SQZ.EXE 0x4059e0: compose the mood animation over the panel and copy it to the screen.
#include "MoodFace.h"
#include "Bitmap2/Bitmap2.h"
#include <mmsystem.h>
extern Renderer *g_renderer;                       // 0x4c75dc
struct MoodSimulation { void *GetSoul(int id); };   // 0x414080
MoodSimulation *GetSimulation();                   // 0x420690
struct MoodPanel { void GetScreenOrigin(int *x, int *y); }; // 0x42da30
MoodPanel *GetPanel();                             // 0x4206c0
extern const char kMoodSoulType[], kMoodPlayerSoulType[]; // 0x4b93e8, 0x4b9490
struct MoodSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual int Mood();                            // +0x64
};

int MoodFace::Draw(int force, Bitmap *panel)
{
    if (g_renderer->field_078) return 0;
    void *soul = GetSimulation()->GetSoul(0);
    if (!soul) return 0;
    MoodSoul *player = (MoodSoul *)SqzRTDynamicCast(soul, 0, kMoodSoulType, kMoodPlayerSoulType, 0);
    int mood = player->Mood();
    int asleep = mood == 22;
    if (mood==lastMood && animation[current].frameCount==1 && !force) return asleep;
    if (mood==lastMood && animation[current].frameCount>1) {
        __int64 elapsed = (unsigned)(timeGetTime()-lastTick);
        if (!((double)elapsed > frameMs)) return asleep;
        frame++;
        if (animation[current].frameCount<=frame) frame=0;
    } else {
        frame=0;
        int row=FindAnimation(mood);
        if (row==current && animation[current].frameCount==1 && !force) return asleep;
        current=row;
    }
    LPDIRECTDRAWSURFACE primary=g_renderer->primary;
    int ox, oy;
    GetPanel()->GetScreenOrigin(&ox, &oy);
    if (!canvas) {
        canvas=new Bitmap(g_renderer);
        canvas->CreateSurfaceSized(width,height);
        canvas->CaptureFrom(panel->GetSurface(),x,y);
        background=new Bitmap(g_renderer);
        background->CreateSurfaceSized(width,height);
        background->CaptureFrom(panel->GetSurface(),x,y);
    }
    background->DrawToSurfaceAt(canvas->GetSurface(),0,0,0,0,0);
    animation[current].frames[frame]->DrawToSurfaceAt(canvas->GetSurface(),0,0,0,0,1);
    canvas->DrawToSurfaceAt(primary,x+ox,y+oy,0,0,0);
    lastTick=timeGetTime();
    lastMood=mood;
    return asleep;
}

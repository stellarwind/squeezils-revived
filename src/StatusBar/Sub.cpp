// SQZ.EXE 0x42f050 (ret 0xc): redraw the goodie strip when the inventory summary changed or the blink timer ran out.
#include <windows.h>
#include <mmsystem.h>
#include "GoodieList/GoodieList.h"

struct SubInv { String Summary(); };                     // 0x42f230
struct SubOwner { SubInv *Inventory(); };                // 0x448990
struct SubSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual SubOwner *Owner();                           // slot 18 (+0x48)
};
struct SubSim { SubSoul *GetSoul(int id); };             // 0x414080
SubSim *GetSimulation();                                 // 0x420690

struct StatusT {
    char pad0[0x6c];
    GoodieList list;                                     // 0x6c
    char pad1[0x364 - 0x6c - sizeof(GoodieList)];
    int step;                                            // 0x364
    int steps;                                           // 0x368
    unsigned period;                                     // 0x36c
    unsigned lastTime;                                   // 0x370
    int flip;                                            // 0x374
    void Reset();                                        // 0x42f2a0
    int Sub(int force, int *event, Bitmap *panel);
};

int StatusT::Sub(int force, int *event, Bitmap *panel)
{
    int timeUp = 0;
    DWORD now;
    if (*event == 1) {
        now = timeGetTime();
        if (now - lastTime > period)
            timeUp = 1;
    }
    static String last("");
    SubSoul *soul = GetSimulation()->GetSoul(0);
    if (!soul)
        return 0;
    SubOwner *owner = soul->Owner();
    if (!owner)
        return 0;
    String text = owner->Inventory()->Summary();
    if (StringCompare(&text, &last))
        force = 1;
    last = text;
    if (force || timeUp) {
        list.Draw(flip, text, panel);
        if (timeUp) {
            flip = !flip;
            step++;
            if (steps <= step) {
                Reset();
                *event = 0;
            }
            lastTime = now;
        }
    }
    return 1;
}

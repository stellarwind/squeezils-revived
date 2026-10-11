// Menu layout used by MenuDraw.cpp / MenuMouse.cpp.
#pragma once
#include <windows.h>
#include <ddraw.h>

struct Bm {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual int DrawAt(LPDIRECTDRAWSURFACE dst, int x, int y, int a, int b, int c);   // slot 4
    int GetWidth();       // 0x4739b0
    int GetHeight();      // 0x4739c0
};

struct Item {
    Bm *normal, *lit;
    int left, top, right, bottom;
    int action, key;
    int highlighted;
};

struct MenuD {
    virtual int SetSelected(int index);   // slot 0
    int itemCount;
    int left, top, itemHeight, spacing;
    Item items[10];
    int anchorX, anchorY;
    void SaveAnchor();                    // 0x406b80
    int HitTest(int index, int x, int y);   // 0x406a60
    void DrawItem(int index, int lit);
    void Relayout();
    int OnMouseMove(int x, int y);
};

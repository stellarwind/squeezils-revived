// Menu::SetSelected (0x406a10), DrawItem (0x406c20), Relayout (0x406ba0).
#include <windows.h>
#include <ddraw.h>

struct Renderer { char pad[0x18]; HWND hwnd; char pad2[0x78 - 0x1c]; int field_078; char pad3[0x994 - 0x7c]; LPDIRECTDRAWSURFACE primary; };
extern Renderer *g_app;        // 0x4c75dc

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

struct RawItem {
    char pad[0x18];
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
    void DrawItem(int index, int lit);
    void Relayout();
};

int MenuD::SetSelected(int index)
{
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].highlighted) {
            if (i == index)
                DrawItem(i, 1);
        } else if (i != index) {
            DrawItem(i, 0);
        }
    }
    return 0;
}

void MenuD::DrawItem(int index, int lit)
{
    if (g_app->field_078)
        return;
    SaveAnchor();
    LPDIRECTDRAWSURFACE primary = g_app->primary;
    RawItem &it = *(RawItem *)((char *)this + index * 0x24);
    POINT pt = { it.left, it.top };
    ClientToScreen(g_app->hwnd, &pt);
    Bm *b = !lit ? it.normal : it.lit;
    b->DrawAt(primary, pt.x, pt.y, 0, 0, 1);
    it.highlighted = lit;
}

void MenuD::Relayout()
{
    SaveAnchor();
    for (int i = 0; i < itemCount; i++) {
        int x = anchorX;
        int y = (spacing + itemHeight) * i + anchorY;
        if (i < 10) {
            Item *p = &items[i];
            p->left = x;
            p->right = x + p->normal->GetWidth();
            p->top = y;
            p->bottom = y + p->normal->GetHeight();
            DrawItem(i, 0);
        }
    }
    SetSelected(0);
}

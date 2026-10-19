// Ruler row draw (SQZ.EXE 0x430440, ret 4): draw the panel backdrop, then the button of each place whose state in this row is 1.
struct SurfaceR;
struct DrawBitmap {
    virtual void d0(); virtual void d1(); virtual void d2(); virtual void d3();
    virtual int DrawToSurfaceAt(SurfaceR *dest, int x, int y, int a, int b, int c);   // slot 4 (+0x10)
};
struct RendererR { char pad[0x994]; SurfaceR *primary; };
extern RendererR *g_renderer;                   // 0x4c75dc
struct PanelR { void GetScreenOrigin(int *x, int *y); };   // 0x42da30
PanelR *GetPanel();                             // 0x4206c0
struct RowInv { int pad; int state[1]; };
struct RowOwner { RowInv *Inventory(); };       // 0x448990
struct RowSoul { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
               virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
               virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
               virtual RowOwner *Owner(); };   // slot 18 (+0x48)
struct RowSim { RowSoul *GetSoul(int id); };    // 0x414080
RowSim *GetCurrentSimulation();                 // 0x420690

struct RowPanel {
    DrawBitmap *back;                           // 0x00
    char pad0[0x14];
    DrawBitmap *b0; char pad1[0x14];            // 0x18
    DrawBitmap *b1; char pad2[0x14];            // 0x30
    DrawBitmap *b2; char pad3[0x14];            // 0x48
    int x, y;                                   // 0x60
    void DrawRow(int row);
};

void RowPanel::DrawRow(int row)
{
    int i;
    int ox, oy;
    GetPanel()->GetScreenOrigin(&ox, &oy);
    SurfaceR *screen = g_renderer->primary;
    back->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
    DrawBitmap *btn = 0;
    RowSoul *soul = GetCurrentSimulation()->GetSoul(0);
    if (!soul)
        return;
    RowOwner *owner = soul->Owner();
    if (!owner)
        return;
    for (i = 0; i < 3; i++) {
        if (owner->Inventory()->state[row * 3 + i] == 1) {
            switch (i) {
            case 0: btn = b0; break;
            case 1: btn = b1; break;
            case 2: btn = b2; break;
            }
            btn->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
        }
    }
}

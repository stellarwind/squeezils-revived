// StatusBar::Update (SQZ.EXE 0x42ed70, ret 0xc): refresh the counters, then draw the face for the current answer.
struct SurfaceS;
struct FaceBitmap {
    virtual void d0(); virtual void d1(); virtual void d2(); virtual void d3();
    virtual int DrawToSurfaceAt(SurfaceS *dest, int x, int y, int a, int b, int c);   // slot 4 (+0x10)
};
struct RendererS { char pad[0x994]; SurfaceS *primary; };
extern RendererS *g_renderer;                   // 0x4c75dc
extern char kEmptyString[];                     // 0x4b9488
extern int g_faceDrawn;                         // 0x4bbea4
struct PanelS {
    void GetScreenOrigin(int *x, int *y);       // 0x42da30
    void DrawMessage(const char *text);         // 0x42dd30
};
PanelS *GetPanel();                             // 0x4206c0

struct StatusS {
    char pad[0x378];
    int answer;                                 // 0x378
    FaceBitmap *faceA;                          // 0x37c
    FaceBitmap *faceB;                          // 0x380
    int x, y;                                   // 0x384
    void Remember(int a, int b);                // 0x42ee90
    void Sub(int a, int *b, int c);             // 0x42f050
    int UpdateAmo(int force);                   // 0x42eec0
    int UpdateFood(int force);                  // 0x42ef50
    int Update(int redraw, int *event, int background);
};

int StatusS::Update(int redraw, int *event, int background)
{
    Remember(redraw, background);
    Sub(redraw, event, background);
    UpdateAmo(redraw);
    UpdateFood(redraw);
    if (answer) {
        if (redraw || !g_faceDrawn) {
            int ox, oy;
            g_faceDrawn = 1;
            GetPanel()->GetScreenOrigin(&ox, &oy);
            SurfaceS *screen = g_renderer->primary;
            if (answer == 1) {
                GetPanel()->DrawMessage(kEmptyString);
                faceA->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
            } else {
                faceB->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
            }
        }
    } else {
        g_faceDrawn = 0;
    }
    return 1;
}

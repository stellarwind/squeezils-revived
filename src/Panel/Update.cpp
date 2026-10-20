// Panel::Update (SQZ.EXE 0x42db10, ret 4): once a frame, redraw what changed (everything when forced or dirty).
int DebugLogAll(const char *format, ...);   // 0x419290
struct Renderer { char pad[0x78]; int suspended; };
extern Renderer *g_app;                   // 0x4c75dc
extern int g_panelDrawn;                  // 0x4bbe90
extern const char kSetting[];             // 0x4bbecc

struct StatusU { void Update(int redraw, int *event, void *background); };   // 0x42ed70
struct QuestU { int Draw(int force, int key); };                              // 0x42f690

struct PanelU {
    int originX, originY;
    StatusU status;                       // 0x008
    char pad9[0x380 - 9];
    int faceState;                        // 0x380
    char pad384[0x394 - 0x384];
    QuestU quest;                         // 0x394
    char pad395[0x450 - 0x395];
    int mode, lastMode, questKey;         // 0x450
    int dirty;                            // 0x45c
    void *background;                     // 0x460
    char pad464[0x614 - 0x464];
    int event;                            // 0x614
    char pad618[0x630 - 0x618];
    char message[100];                    // 0x630
    char note[20];                        // 0x694

    int Update(int force);
    void ClearEvent();                    // 0x42ea60
    int DrawBackground();                 // 0x42dc50
    void UpdateCompass(int force);        // 0x42e2b0
    void DrawMessage(const char *text);   // 0x42dd30
    void DrawNote(const char *text);      // 0x42e0e0
};
extern "C" unsigned strlen(const char *);

int PanelU::Update(int force)
{
    if (g_app->suspended)
        return 1;
    int before;
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
    if (g_panelDrawn && !redraw) {
        UpdateCompass(0);
    } else {
        DrawBackground();
        UpdateCompass(1);
        g_panelDrawn = 1;
    }
    if (mode == 0) {
        int *pEvent = &event;
        before = *pEvent;
        status.Update(redraw, pEvent, background);
        if (before == 1 && *pEvent == 0)
            ClearEvent();
        if (redraw) {
            if (!faceState) {
                DebugLogAll(kSetting, message);
                DrawMessage(message);
            }
            if (strlen(note))
                DrawNote(note);
        }
    } else {
        quest.Draw(redraw, questKey);
        if (redraw)
            DrawMessage(message);
    }
    return 1;
}

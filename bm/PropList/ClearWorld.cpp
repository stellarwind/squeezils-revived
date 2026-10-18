// ClearWorldObjects (SQZ.EXE 0x42d010): free the level's prop and path tables, flush queued removals, and empty the destroy/update lists.
struct Prop { virtual void VDestroy(int del); };
struct PathObj { ~PathObj(); };                // 0x41b940

struct PropTable {                             // keys (count at +8) and values (data at +0x24)
    int pad0, pad4;
    int count;                                 // 0x08
    char pad[0x18];
    Prop **values;                             // 0x24
    void RemoveAt(int i);                      // 0x468a50
};
struct PathTable {
    int pad0, pad4;
    int count;                                 // 0x08
    char pad[0x18];
    PathObj **values;                          // 0x24
    void RemoveAt(int i);                      // 0x468e90
};
struct PropArray {
    int pad0;
    int count;                                 // 0x04
    int pad8;
    Prop **data;                               // 0x0c
    void RemoveAt(int i);                      // 0x468fd0
};
extern PropTable g_thirdProps;                 // 0x4c72c0
extern PathTable g_thirdPaths;                 // 0x4c71d0
extern PropTable g_secondProps;                // 0x4c7230
extern PathTable g_secondPaths;                // 0x4c7258
extern PropArray g_destroy;                    // 0x4c72e8
extern PropArray g_updating;                   // 0x4c71f8
void FlushRemovedProps();                      // 0x42d2b0

static inline void EmptyPropsLoop(PropTable &t)
{
    int count = t.count;
    int i = 0;
    for (;;) {
        if (i >= count)
            break;
        Prop *p = t.values[i];
        if (p)
            p->VDestroy(1);
        i++;
    }
    for (int j = 0; j < count; j++)
        t.RemoveAt(count - j - 1);
}

static inline void EmptyProps(PropTable &t)
{
    int count = t.count;
    for (int i = 0; i < count; i++) {
        Prop *p = t.values[i];
        if (p)
            p->VDestroy(1);
    }
    for (int j = 0; j < count; j++)
        t.RemoveAt(count - j - 1);
}

static inline void EmptyPaths(PathTable &t)
{
    int count = t.count;
    int i = 0;
    for (;;) {
        if (i >= count)
            break;
        PathObj *p = t.values[i];
        if (p)
            delete p;
        i++;
    }
    for (int j = 0; j < count; j++)
        t.RemoveAt(count - j - 1);
}

void ClearWorldObjects()
{
    EmptyPropsLoop(g_thirdProps);
    EmptyPaths(g_thirdPaths);
    EmptyProps(g_secondProps);
    EmptyPaths(g_secondPaths);
    FlushRemovedProps();
    for (int i = g_destroy.count - 1; i >= 0; i--) {
        Prop *p = g_destroy.data[i];
        if (p)
            p->VDestroy(1);
        g_destroy.RemoveAt(i);
    }
    for (int j = g_updating.count - 1; j >= 0; j--)
        g_updating.RemoveAt(j);
}

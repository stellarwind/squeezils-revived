// NavProp::Update (0x429c60): move the shown navigation surface with its frame.
struct UpdateMatrix {
    float m[16];
    UpdateMatrix() {}
    ~UpdateMatrix(); // 0x402310
};
struct UpdatePlacement {
    int instance, sub;
    UpdateMatrix matrix;
    UpdatePlacement() { instance = -1; sub = 0; }
    ~UpdatePlacement(); // 0x4031e0
};
struct UpdateTables { void Move(int instance, UpdateMatrix *matrix); }; // 0x410310
struct UpdatePlace { UpdateTables *GetSurfaces(); }; // 0x439b60
struct UpdateScene {
    void *GetSceneChild(); // 0x477290
    // The original caller also loads ecx with the scene before this callee-pop call.
    void Transform(void *frame, void *relative, UpdateMatrix *out); // 0x477120
};
extern UpdateScene *g_updateScene; // 0x4c75e0
void *GetUpdateSimulation(); // 0x420690
void *UpdateDynamicCast(void *, int, const void *, const void *, int);
extern const char kUpdateSimType[], kUpdatePlaceType[];
struct UpdateFrame {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual UpdateFrame *GetOwner();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13();
    virtual int IsShown();
    int HasBody(); // 0x427f40
    void Update(float seconds); // 0x427740
    void *GetPartFrame(); // 0x4276b0
};
struct NavUpdate : UpdateFrame {
    char pad[0x94 - 4];
    void *body;
    char pad2[0xf8 - 0x98];
    int instance;
    void Update(float seconds);
};
void NavUpdate::Update(float seconds)
{
    UpdateFrame::Update(seconds);
    if (IsShown()) {
        UpdatePlacement placement;
        placement.instance = instance;
        if (!body) {
            UpdateFrame *owner = GetOwner();
            while (owner && !owner->HasBody())
                owner = owner->GetOwner();
            if (!owner)
                return;
        }
        UpdateMatrix matrix;
        g_updateScene->Transform(GetPartFrame(), g_updateScene->GetSceneChild(), &matrix);
        ((UpdatePlace *)UpdateDynamicCast(GetUpdateSimulation(), 0,
            kUpdateSimType, kUpdatePlaceType, 0))->GetSurfaces()->Move(instance, &matrix);
    }
}

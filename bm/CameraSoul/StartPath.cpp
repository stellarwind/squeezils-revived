// 0x409a50: freeze the player and blend the camera onto an animation path.
struct PathPart { char data[0x10]; PathPart(); }; // 0x41d8a0
struct PathClip;
struct PathAnimation {
    virtual void AddPart(const char *name, PathPart *part);
    virtual void Build(void *parent);
    virtual void v2();
    virtual void SetCycle(PathClip *clip, int blend);
    virtual void v4(); virtual void v5(); virtual void v6();
    virtual void SetLoopMode(int mode);
    virtual void v8(); virtual void v9(); virtual void v10();
    virtual void v11(); virtual void v12(); virtual void v13();
    virtual void SetTime(float time);
    char data[0x50 - 4];
    PathAnimation(); // 0x41d9e0
};
struct PathScene { void *GetSceneChild(); }; // 0x477290
extern PathScene *g_pathScene;
struct PathActor {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void Freeze();
};
struct PathMatrix {
    float m[16];
    PathMatrix &operator=(const PathMatrix &source) {
        m[0] = source.m[0]; m[1] = source.m[1]; m[2] = source.m[2]; m[3] = source.m[3];
        m[4] = source.m[4]; m[5] = source.m[5]; m[6] = source.m[6]; m[7] = source.m[7];
        m[8] = source.m[8]; m[9] = source.m[9]; m[10] = source.m[10]; m[11] = source.m[11];
        m[12] = source.m[12]; m[13] = source.m[13]; m[14] = source.m[14]; m[15] = source.m[15];
        return *this;
    }
};
struct PathCamera {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual PathMatrix *GetMatrix();
};
void CameraPathLog(const char *, ...);
struct PathSoul {
    char pad[8];
    PathCamera *camera;
    void *target;
    PathActor *targetSoul;
    int enabled;
    char pad18[0x88 - 0x18];
    PathMatrix fromMatrix;
    char padc8[0x148 - 0xc8];
    int lookMode;
    float blendTime, blendDuration;
    int blending, blendPosition, justStarted;
    PathAnimation *path;
    int pathMode;
    void StartPath(PathClip *clip, int loopMode, int mode, float duration);
};
void PathSoul::StartPath(PathClip *clip, int loopMode, int mode, float duration)
{
    if (clip) {
        targetSoul->Freeze();
        PathPart *part = new PathPart;
        path = new PathAnimation;
        path->AddPart("part", part);
        path->Build(g_pathScene->GetSceneChild());
        path->SetCycle(clip, 0);
        path->SetTime(0);
        path->SetLoopMode(loopMode);
        pathMode = mode;
        enabled = 0;
        blendTime = 0;
        blendDuration = duration;
        fromMatrix = *camera->GetMatrix();
        blending = 1;
        blendPosition = 1;
        lookMode = 1;
    } else {
        CameraPathLog("Path is not valid.\n");
    }
}

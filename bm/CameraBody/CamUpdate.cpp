// CameraBody::Update (SQZ.EXE 0x40b990): orthonormalize the body's matrix, store it back and hand it to the scene camera.
struct CMat {                                            // 64 bytes, empty out-of-line destructor 0x402310
    float v[16];
    CMat() {}
    CMat(const CMat &o)
    {
        v[0] = o.v[0];
        v[1] = o.v[1];
        v[2] = o.v[2];
        v[3] = o.v[3];
        v[4] = o.v[4];
        v[5] = o.v[5];
        v[6] = o.v[6];
        v[7] = o.v[7];
        v[8] = o.v[8];
        v[9] = o.v[9];
        v[10] = o.v[10];
        v[11] = o.v[11];
        v[12] = o.v[12];
        v[13] = o.v[13];
        v[14] = o.v[14];
        v[15] = o.v[15];
    }
    ~CMat();
    void Orthonormalize();                               // 0x47e350
};
struct CScene { void SetCamera(CMat *m); };              // 0x476e10
extern CScene *g_scene;                                  // 0x4c75e0

struct CBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual CMat *GetMatrix();                           // slot 3 (+0xc)
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(CMat *m);                     // slot 9 (+0x24)
    void Update();
};

void CBody::Update()
{
    CMat m = *GetMatrix();
    m.Orthonormalize();
    SetMatrix(&m);
    g_scene->SetCamera(&m);
}

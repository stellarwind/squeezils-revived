// 0x459c00: launch a goodie on the next of sixteen spray courses.
struct SprayVector {
    float x, y, z;
    SprayVector(float a, float b, float c) : x(a), y(b), z(c) {}
    ~SprayVector() {}
    float Normalize();                         // 0x47d630
};

struct SprayMatrix {
    float m[16];
    SprayMatrix() {}
    SprayMatrix(const SprayVector &p) {
        m[0]=1; m[1]=0; m[2]=0; m[3]=0;
        m[4]=0; m[5]=1; m[6]=0; m[7]=0;
        m[8]=0; m[9]=0; m[10]=1; m[11]=0;
        m[12]=p.x; m[13]=p.y; m[14]=p.z; m[15]=1;
    }
    ~SprayMatrix() {}
    SprayMatrix &TurnX(float radians);          // 0x47dcd0
    SprayMatrix &TurnY(float radians);          // 0x47dd20
    SprayMatrix operator*(const SprayMatrix &other) const; // 0x47dea0
};

struct SprayBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(SprayMatrix *matrix); // +0x24
    virtual void SetFacing(SprayVector *direction, float speed); // +0x28
};

struct SpraySettings {
    char pad[0x34];
    float angle, speed, offset;                 // +0x34, +0x38, +0x3c
};
extern SpraySettings *gGoodieFlightSettings;    // 0x4c7bb4
extern int gSprayCounter;                      // 0x4bf358

struct GoodieSoul {
    void **vtbl, **grantVtbl;
    SprayBody *body;                           // +0x08
    void *name;
    int kind, thrown, bounces;
    float flightTime;                          // +0x1c
    SprayVector direction;                    // +0x20
    void Spray(SprayMatrix from);
};

void GoodieSoul::Spray(SprayMatrix from)
{
    SprayVector offset(0.0f, 0.0f, gGoodieFlightSettings->offset);
    SprayMatrix shift(offset), turnY, turnX;
    float angle;
    switch (gSprayCounter % 8) {
    case 0: angle=0.0f; break;
    case 1: angle=180.0f; break;
    case 2: angle=90.0f; break;
    case 3: angle=270.0f; break;
    case 4: angle=45.0f; break;
    case 5: angle=225.0f; break;
    case 6: angle=135.0f; break;
    case 7: angle=315.0f; break;
    }
    if (gSprayCounter > 7) angle += 22.5f;
    turnY.TurnY(angle * 0.017453292f);
    turnX.TurnX((float)(360.0 - gGoodieFlightSettings->angle) * 0.017453292f);
    SprayMatrix placed = shift * turnX * turnY * from;
    body->SetMatrix(&placed);
    direction.x=placed.m[8]; direction.y=placed.m[9]; direction.z=placed.m[10];
    direction.Normalize();
    ++gSprayCounter;
    if (gSprayCounter == 16) gSprayCounter=0;
    body->SetFacing(&direction, gGoodieFlightSettings->speed);
    flightTime=0.0f;
}

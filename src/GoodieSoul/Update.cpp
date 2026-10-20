// SQZ.EXE 0x459f80: ballistic flight, surface bounces and delayed rebudding.
#define V3_DTOR_OUTOFLINE
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"

struct FlightMatrix {
    float m[16];
    FlightMatrix() {}
    FlightMatrix(const FlightMatrix &o) {
        m[0]=o.m[0]; m[1]=o.m[1]; m[2]=o.m[2]; m[3]=o.m[3];
        m[4]=o.m[4]; m[5]=o.m[5]; m[6]=o.m[6]; m[7]=o.m[7];
        m[8]=o.m[8]; m[9]=o.m[9]; m[10]=o.m[10]; m[11]=o.m[11];
        m[12]=o.m[12]; m[13]=o.m[13]; m[14]=o.m[14]; m[15]=o.m[15];
    }
    FlightMatrix(const Vector3f &r, const Vector3f &u, const Vector3f &f, const Vector3f &p) {
        m[0]=r.x; m[1]=r.y; m[2]=r.z; m[3]=0;
        m[4]=u.x; m[5]=u.y; m[6]=u.z; m[7]=0;
        m[8]=f.x; m[9]=f.y; m[10]=f.z; m[11]=0;
        m[12]=p.x; m[13]=p.y; m[14]=p.z; m[15]=1;
    }
    ~FlightMatrix();                           // 0x402310
    Vector3f *Position();                       // 0x41faf0
    FlightMatrix &SetTranslation(const Vector3f &p); // 0x47e330
};
struct FlightContact {
    int face, second;
    FlightMatrix matrix;
    FlightContact() : face(-1), second(0) {}
    ~FlightContact();                          // 0x4031e0
};
struct FlightBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual FlightMatrix *Matrix();            // +0x0c
    virtual Vector3f *Facing();                // +0x10
    virtual float Speed();                     // +0x14
    virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(FlightMatrix *m);   // +0x24
    virtual void SetFacing(Vector3f *v, float speed); // +0x28
    virtual void v11(); virtual void v12(); virtual void v13();
    virtual void SetContact(FlightContact *c); // +0x38
};
struct FlightLevel { void Forget(int id); };   // 0x44e020
struct FlightSurfaces {
    int FlightHitsSurface(const Vector3f *pos, const Vector3f *right, const Vector3f *up,
        const Vector3f *forward, Vector3f *direction, float distance, Vector3f *hitPos,
        Vector3f *newRight, Vector3f *newUp, Vector3f *newForward, FlightContact *contact); // 0x4108e0
    int FlyThrough(FlightContact *c);          // 0x40f8d0
    float RebudGoodies(FlightContact *c);      // 0x40fb20
};
struct FlightSim {
    int FindIdBySoul(void *soul);              // 0x4140b0
    void AddId(int id);                        // 0x414180
};
struct FlightPlace {
    FlightLevel *GetLevel();                   // 0x439b50
    FlightSurfaces *GetSurfaces();             // 0x439b60
};
struct FlightClock { unsigned Milliseconds(); }; // 0x41ec00
FlightSim *GetSimulation();                    // 0x420690
FlightClock *GetFlightClock();                 // 0x4206d0
void *FlightDynamicCast(void *p, int, const void *, const void *, int); // 0x48f060
extern const char kFlightLocationType[], kFlightPlaceType[]; // 0x4ba1d0, 0x4bae38
struct FlightFace { char pad[0xc]; Vector3f normal; char rest[0x10]; }; // 0x28
struct FlightMesh { char pad[0xc]; FlightFace *faces; };
struct FlightWorld { FlightMesh *Surface(int face); }; // 0x4365a0
FlightWorld *GetFlightWorld();                  // 0x4206a0
struct FlightSettings {
    float gravity[3]; char pad[0x24 - 12];
    float gravityFactor; int bounceCount; float bounceDamping;
};
extern FlightSettings *gGoodieFlightSettings;   // 0x4c7bb4
extern float gGoodieRebudPlane;                 // 0x4c7bb0
void PlayEffectAt(String name, Vector3f position); // 0x44a250

struct GoodieSoul {
    void **vtbl, **grantVtbl;
    FlightBody *body;
    void *name;
    int kind, thrown, bounces;
    float flightTime;
    Vector3f direction;
    float rebudTimer, rebudDelay;
    void UpdateGoodie();
};

void GoodieSoul::UpdateGoodie()
{
    FlightSim *sim = GetSimulation();
    FlightPlace *place = (FlightPlace *)FlightDynamicCast(sim, 0, kFlightLocationType, kFlightPlaceType, 0);
    float seconds = (float)GetFlightClock()->Milliseconds() * 0.001f;
    if (kind) body->SetFacing(body->Facing(), 0.0f);
    if (*(unsigned *)&rebudTimer != 0xbf800000U) {
        rebudTimer += seconds;
        if (!(rebudTimer < rebudDelay)) {
            rebudTimer = -1.0f;
            rebudDelay = 0.0f;
            place->GetLevel()->Forget(sim->FindIdBySoul(this));
            sim->AddId(sim->FindIdBySoul(this));
            return;
        }
    }
    if (direction.Length() <= 0.1f) return;
    Vector3f unused;
    flightTime += seconds;
    if ((*(unsigned *)&flightTime & 0x7fffffffU) == 0) return;
    Vector3f step;
    step = direction * body->Speed() * seconds;
    step.y = ((flightTime * flightTime - (flightTime - seconds) * (flightTime - seconds)) *
        (gGoodieFlightSettings->gravityFactor * gGoodieFlightSettings->gravity[1])) * 0.5f + step.y;
    Vector3f course;
    course = step;
    float distance = course.Length();
    course.Normalize();
    body->SetFacing(&course, body->Speed());
    Vector3f hitPos, newRight, newUp, newForward;
    FlightContact contact;
    FlightBody *flightBody = body;
    FlightMatrix *forwardMatrix = flightBody->Matrix();
    FlightMatrix *upMatrix = flightBody->Matrix();
    FlightMatrix *rightMatrix = flightBody->Matrix();
    FlightMatrix *positionMatrix = flightBody->Matrix();
    int hit = place->GetSurfaces()->FlightHitsSurface(
        (Vector3f *)(positionMatrix->m + 12), (Vector3f *)(rightMatrix->m),
        (Vector3f *)(upMatrix->m + 4), (Vector3f *)(forwardMatrix->m + 8),
        flightBody->Facing(), distance, &hitPos, &newRight, &newUp, &newForward, &contact);
    if (hit && place->GetSurfaces()->FlyThrough(&contact)) hit = 0;
    if (hit) {
        ++bounces;
        if (!(gGoodieFlightSettings->bounceCount > bounces)) {
            Vector3f zero(0.0f, 0.0f, 0.0f);
            body->SetFacing(&zero, 0.0f);
            direction.x = direction.y = direction.z = 0.0f;
            body->SetContact(&contact);
            thrown = 0;
            bounces = 0;
            float delay = place->GetSurfaces()->RebudGoodies(&contact);
            if (delay != 0.0f) {
                rebudTimer = 0.0f;
                rebudDelay = delay;
            }
        } else {
            float speed = body->Speed() * gGoodieFlightSettings->bounceDamping;
            Vector3f d;
            d = *body->Facing();
            Vector3f n;
            n = GetFlightWorld()->Surface(contact.face)->faces[contact.second].normal;
            Vector3f reflected = d - n * ((d.y*n.y + d.z*n.z + d.x*n.x) * 2.0f);
            reflected.Normalize();
            body->SetFacing(&reflected, speed);
            direction = reflected;
            flightTime = 0.0f;
        }
        FlightMatrix placed(newRight, newUp, newForward, hitPos);
        body->SetMatrix(&placed);
        PlayEffectAt(String("bounceEffect"), *body->Matrix()->Position());
    } else {
        hitPos = *(Vector3f *)(body->Matrix()->m + 12) + step;
        // VC4 rewrites an ordinary comparison with the opposite unordered result.
        // Preserve the original x87 C0/C3 test, including its NaN behavior.
        unsigned char planeFlags;
        __asm {
            fld DWORD PTR gGoodieRebudPlane
            fcomp DWORD PTR hitPos.y
            fnstsw ax
            mov planeFlags, ah
        }
        if (!(planeFlags & 0x41)) {
            place->GetLevel()->Forget(sim->FindIdBySoul(this));
            sim->AddId(sim->FindIdBySoul(this));
            return;
        }
        FlightMatrix moved = *body->Matrix();
        moved.SetTranslation(hitPos);
        body->SetMatrix(&moved);
    }
}

// SQZ.EXE 0x410330: carry supported bodies and their heading with a moving surface.
#include "NavSurface/NavSurfaceList.h"

struct CarryVector {
    float x, y, z;
    ~CarryVector(); // 0x401660
    CarryVector &operator=(const CarryVector &v) { x=v.x; y=v.y; z=v.z; return *this; }
    CarryVector operator*(float scale) const; // 0x401b60
    CarryVector operator+(const CarryVector &v) const; // 0x403070
    CarryVector operator-(const CarryVector &v) const; // 0x401ae0
    float Normalize(); // 0x47d630
};
struct CarryMatrix {
    float m[16];
    CarryMatrix(const float *v) {
        m[0]=v[0]; m[1]=v[1]; m[2]=v[2]; m[3]=v[3];
        m[4]=v[4]; m[5]=v[5]; m[6]=v[6]; m[7]=v[7];
        m[8]=v[8]; m[9]=v[9]; m[10]=v[10]; m[11]=v[11];
        m[12]=v[12]; m[13]=v[13]; m[14]=v[14]; m[15]=v[15];
    }
    ~CarryMatrix(); // 0x402310
    const CarryVector &Position() const { return *(const CarryVector *)(m+12); }
};
struct CarryBody {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual const CarryMatrix *GetMatrix(); // slot 3
    virtual const CarryVector *GetFacing(); // slot 4
    virtual float GetFacingValue(); // slot 5
    virtual void slot6();
    virtual Contact *GetContact(); // slot 7
    virtual int IsOnSurface(); // slot 8
    virtual void SetMatrix(const CarryMatrix &matrix); // slot 9
    virtual void SetFacing(const CarryVector &facing, float speed); // slot 10
};
struct CarryCreature { void *soul; CarryBody *body; };
struct CarryCreatures {
    char pad00[8];
    int count;
    char pad0c[0x24-0x0c];
    CarryCreature **data;
};
struct CarrySimulation { CarryCreatures *GetCreatures(); }; // 0x414150
CarrySimulation *GetCarrySimulation(); // 0x420690
struct CarrySurface {
    int Changed(); // 0x40c860
    CarryMatrix MatrixToWorld(const CarryMatrix &matrix); // 0x4105f0
    CarryVector ToLocalBefore(const CarryVector &point); // 0x410660
    CarryVector ToWorld(const CarryVector &point); // 0x40f850
};

void NavSurfaceList::MoveCarried()
{
    Array<CarrySurface *> &surfaces = *(Array<CarrySurface *> *)((char *)this + 0x134);
    for (int i=0; ; ++i) {
        if (GetCarrySimulation()->GetCreatures()->count <= i)
            break;
        CarryBody *body = GetCarrySimulation()->GetCreatures()->data[i]->body;
        if (!body->IsOnSurface())
            continue;
        if (!surfaces.Get(body->GetContact()->face)->Changed())
            continue;
        CarryMatrix relative(body->GetContact()->matrix);
        CarrySurface *surface = surfaces.Get(body->GetContact()->face);
        CarryMatrix world = surface->MatrixToWorld(relative);
        float speed = body->GetFacingValue();
        const CarryVector &position = body->GetMatrix()->Position();
        CarryVector reached = *body->GetFacing() * speed + position;
        CarryVector before = surface->ToLocalBefore(reached);
        CarryVector direction = surface->ToWorld(before);
        direction = direction - world.Position();
        float length = direction.Normalize();
        body->SetMatrix(world);
        body->SetFacing(direction, length);
    }
}

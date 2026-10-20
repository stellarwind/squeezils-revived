// SQZ.EXE 0x40bf90: initialize one placed surface and overlay its property bindings.
#include <string.h>
#include "Ini/IniFile.h"

struct InstanceCtorVector {
    float x, y, z;
    InstanceCtorVector() {}
    InstanceCtorVector(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    ~InstanceCtorVector(); // 0x401660
    InstanceCtorVector &operator=(const InstanceCtorVector &v) {
        x=v.x; y=v.y; z=v.z; return *this;
    }
};
struct InstanceCtorMatrix {
    float v[16];
    InstanceCtorMatrix() {}
    ~InstanceCtorMatrix(); // 0x402310
    InstanceCtorMatrix &operator=(const InstanceCtorMatrix &other) {
        v[0]=other.v[0]; v[1]=other.v[1]; v[2]=other.v[2]; v[3]=other.v[3];
        v[4]=other.v[4]; v[5]=other.v[5]; v[6]=other.v[6]; v[7]=other.v[7];
        v[8]=other.v[8]; v[9]=other.v[9]; v[10]=other.v[10]; v[11]=other.v[11];
        v[12]=other.v[12]; v[13]=other.v[13]; v[14]=other.v[14]; v[15]=other.v[15];
        return *this;
    }
    void Identity() {
        v[0]=1; v[1]=0; v[2]=0; v[3]=0;
        v[4]=0; v[5]=1; v[6]=0; v[7]=0;
        v[8]=0; v[9]=0; v[10]=1; v[11]=0;
        v[12]=0; v[13]=0; v[14]=0; v[15]=1;
    }
};
struct InstanceCtorLoaded {
    char pad00[0x14];
    StringList typeNames;
};
struct InstanceCtorKeys { String KeyAt(int index); }; // 0x40ee50
struct InstancedSurfaceCtor {
    InstanceCtorLoaded *surface;
    InstanceCtorMatrix matrix;
    int identity;
    InstanceCtorMatrix previous;
    int previousIdentity, moveDirty, changed;
    void *prop;
    IniValueTable properties;
    PrimitiveArray<int, 1> flags;
    InstanceCtorVector position, previousPosition;
    InstancedSurfaceCtor(void *owner, InstanceCtorLoaded *loaded,
                          const InstanceCtorMatrix &start, IniValueTable *given);
    InstanceCtorVector ToWorld(const InstanceCtorVector &point); // 0x40f850
    void SetPositions(const InstanceCtorVector &point) {
        position=point;
        previousPosition=point;
    }
};

InstancedSurfaceCtor::InstancedSurfaceCtor(void *owner, InstanceCtorLoaded *loaded,
    const InstanceCtorMatrix &start, IniValueTable *given)
{
    surface=loaded;
    prop=owner;
    matrix=start;
    previous=matrix;
    InstanceCtorMatrix identityMatrix;
    identityMatrix.Identity();
    identity=memcmp(matrix.v, identityMatrix.v, 0x40)==0;
    previousIdentity=identity;
    {
        InstanceCtorVector origin(0.0f, 0.0f, 0.0f);
        SetPositions(ToWorld(origin));
    }
    moveDirty=0;
    changed=0;
    for (int i=0; i<loaded->typeNames.count; ++i) {
        properties.Insert(loaded->typeNames.Get(i), String("none"), -1);
        flags.Insert(1, -1);
    }
    for (int j=0; j<given->keys.count; ++j) {
        String key=((InstanceCtorKeys *)given)->KeyAt(j);
        String value=given->At(j);
        properties.Insert(key, value, -1);
    }
}

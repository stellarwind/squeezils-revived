#include "Runtime/DynamicCast.h"
// SQZ.EXE 0x4577e0: restore orientation, position and the surface-relative contact.
#include "SqueezilBody/SqueezilBody.h"

struct PlacementVector {
    float x, y, z;
    ~PlacementVector(); // 0x401660
    PlacementVector Cross(const PlacementVector &other) const; // 0x41a350
    float Normalize(); // 0x47d630
};
struct PlacementMatrix {
    float m[16];
    ~PlacementMatrix(); // 0x402310
    PlacementMatrix &operator=(const PlacementMatrix &other); // 0x402340
    void Orthonormalize(); // 0x47e350
    void CopyInline(const PlacementMatrix &other) {
        m[0]=other.m[0]; m[1]=other.m[1]; m[2]=other.m[2]; m[3]=other.m[3];
        m[4]=other.m[4]; m[5]=other.m[5]; m[6]=other.m[6]; m[7]=other.m[7];
        m[8]=other.m[8]; m[9]=other.m[9]; m[10]=other.m[10]; m[11]=other.m[11];
        m[12]=other.m[12]; m[13]=other.m[13]; m[14]=other.m[14]; m[15]=other.m[15];
    }
};
struct BodyPlacement {
    int unknown00;
    float matrix[16];
    Contact contact; // +0x44
    PlacementVector position, up, direction; // +0x8c, +0x98, +0xa4
    BodyPlacement(const BodyPlacement &other); // 0x4020b0
    ~BodyPlacement(); // 0x402250
};
struct PlacementFace {
    PlacementMatrix Relative(const PlacementMatrix &matrix); // 0x4110b0
};
struct PlacementSurfaces {
    PlacementFace *FaceOf(const Contact *contact); // 0x412bf0
};
struct PlacementSimulation {
    PlacementSurfaces *GetSurfaces(); // 0x439b60
};
void *GetSimulation(); // 0x420690
extern const char kWorldSimulationType[]; // 0x4ba1d0
extern const char kWorldPlaceType[]; // 0x4bae38

void SqueezilBody::Place(BodyPlacement placement)
{
    PlacementVector right = placement.up.Cross(placement.direction);
    right.Normalize();
    matrix[0]=right.x; matrix[1]=right.y; matrix[2]=right.z;
    matrix[4]=placement.up.x; matrix[5]=placement.up.y; matrix[6]=placement.up.z;
    matrix[8]=placement.direction.x; matrix[9]=placement.direction.y; matrix[10]=placement.direction.z;
    matrix[12]=placement.position.x; matrix[13]=placement.position.y;
    matrix[3]=0.0f; matrix[7]=0.0f; matrix[11]=0.0f; matrix[15]=1.0f;
    matrix[14]=placement.position.z;
    ((PlacementMatrix *)matrix)->Orthonormalize();
    contact.face=placement.contact.face;
    contact.second=placement.contact.second;
    *(PlacementMatrix *)contact.matrix = *(PlacementMatrix *)placement.contact.matrix;
    ((PlacementMatrix *)contact.matrix)->CopyInline(
        ((PlacementSimulation *)SqzRTDynamicCast(GetSimulation(), 0,
            kWorldSimulationType, kWorldPlaceType, 0))->GetSurfaces()->FaceOf(
                &placement.contact)->Relative(*(PlacementMatrix *)matrix));
}

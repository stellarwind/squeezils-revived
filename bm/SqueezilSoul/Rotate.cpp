// SqueezilSoul::RotateOnSurface (0x4517e0): self-contained (matrix and vector values with out-of-line dtors).
#include "compat41.h"

struct Matrix16;

struct Vector3f {
    float x, y, z;
    Vector3f(const Matrix16 *m, int row);
    ~Vector3f();                            // 0x401660
    Vector3f &operator=(const Vector3f &o) { x = o.x; y = o.y; z = o.z; return *this; }
    void SetRow(const Matrix16 *m, int row);
};

struct Matrix16 {                           // 64 bytes
    float v[16];
    Matrix16() {}
    ~Matrix16();                            // 0x402310
    Matrix16 &operator=(const Matrix16 &o)
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
        return *this;
    }
    void SetRotation(const Vector3f &axis, float radians);     // 0x47ddc0
    Matrix16 operator*(const Matrix16 &o);                   // 0x47dea0
    void SetTranslation(const Vector3f &p);                  // 0x47e330
};

inline Vector3f::Vector3f(const Matrix16 *m, int row) { x = m->v[row * 4]; y = m->v[row * 4 + 1]; z = m->v[row * 4 + 2]; }
inline void Vector3f::SetRow(const Matrix16 *m, int row) { x = m->v[row * 4]; y = m->v[row * 4 + 1]; z = m->v[row * 4 + 2]; }

Vector3f Transform(const Vector3f &p, const Matrix16 &m);    // 0x47e1e0 (cdecl)

struct Contact {                            // 0x48 bytes
    int a, b;
    Matrix16 frame;                         // 0x8
};

struct SurfaceFace {
    Matrix16 Relative(const Matrix16 &m);                    // 0x4110b0
};

struct NavSurfaces {
    SurfaceFace *FaceOf(Contact *c);                         // 0x412bf0
};

struct Place {
    NavSurfaces *Surfaces();                                 // 0x439b60
};

struct SqueezilSoul {
    virtual void s0();
    char pad04[0x70 - 4];
    Place *place;                           // 0x70

    void RotateOnSurface(Matrix16 *m, Vector3f *v, Contact *contact, float radians);
};

// @004517e0 Turns the matrix about its own up axis (the position stays), keeps the contact's matrix relative to the face
// and turns v the same way.
void SqueezilSoul::RotateOnSurface(Matrix16 *m, Vector3f *v, Contact *contact, float radians)
{
    Matrix16 turn, turned;
    Vector3f position(m, 3);
    Vector3f up(m, 1);
    turn.SetRotation(up, radians);
    *m = *m * turn;
    m->SetTranslation(position);
    position.SetRow(&contact->frame, 3);
    contact->frame = place->Surfaces()->FaceOf(contact)->Relative(*m);
    contact->frame.SetTranslation(position);
    *v = Transform(*v, turn);
}

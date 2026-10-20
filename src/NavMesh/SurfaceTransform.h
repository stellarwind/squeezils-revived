#pragma once
#include "compat41.h"

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    Vector3f(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    ~Vector3f(); // 0x401660
    Vector3f operator+(const Vector3f &other) const; // 0x403070
    Vector3f operator-(const Vector3f &other) const; // 0x401ae0
    Vector3f &operator=(const Vector3f &other) {
        x=other.x; y=other.y; z=other.z; return *this;
    }
};
struct Matrix16 {
    float v[16];
    Matrix16() {
        v[0]=1; v[1]=0; v[2]=0; v[3]=0;
        v[4]=0; v[5]=1; v[6]=0; v[7]=0;
        v[8]=0; v[9]=0; v[10]=1; v[11]=0;
        v[12]=0; v[13]=0; v[14]=0; v[15]=1;
    }
    Matrix16(const Matrix16 &other) {
        v[0]=other.v[0]; v[1]=other.v[1]; v[2]=other.v[2]; v[3]=other.v[3];
        v[4]=other.v[4]; v[5]=other.v[5]; v[6]=other.v[6]; v[7]=other.v[7];
        v[8]=other.v[8]; v[9]=other.v[9]; v[10]=other.v[10]; v[11]=other.v[11];
        v[12]=other.v[12]; v[13]=other.v[13]; v[14]=other.v[14]; v[15]=other.v[15];
    }
    ~Matrix16(); // 0x402310
    Matrix16 &operator=(const Matrix16 &other) {
        v[0]=other.v[0]; v[1]=other.v[1]; v[2]=other.v[2]; v[3]=other.v[3];
        v[4]=other.v[4]; v[5]=other.v[5]; v[6]=other.v[6]; v[7]=other.v[7];
        v[8]=other.v[8]; v[9]=other.v[9]; v[10]=other.v[10]; v[11]=other.v[11];
        v[12]=other.v[12]; v[13]=other.v[13]; v[14]=other.v[14]; v[15]=other.v[15];
        return *this;
    }
    Matrix16 &SetTranslation(const Vector3f &position); // 0x47e330
    Matrix16 &Transpose(); // 0x47dbf0
    const Vector3f &Position() const { return *(const Vector3f *)(v+12); }
};
// The source placement converts inline; a returned owning matrix uses 0x4099e0.
struct SurfaceWorldMatrix {
    float v[16];
    SurfaceWorldMatrix(const Matrix16 &other) {
        v[0]=other.v[0]; v[1]=other.v[1]; v[2]=other.v[2]; v[3]=other.v[3];
        v[4]=other.v[4]; v[5]=other.v[5]; v[6]=other.v[6]; v[7]=other.v[7];
        v[8]=other.v[8]; v[9]=other.v[9]; v[10]=other.v[10]; v[11]=other.v[11];
        v[12]=other.v[12]; v[13]=other.v[13]; v[14]=other.v[14]; v[15]=other.v[15];
    }
    SurfaceWorldMatrix(const SurfaceWorldMatrix &other); // 0x4099e0
    ~SurfaceWorldMatrix(); // 0x402310
    SurfaceWorldMatrix &operator=(const SurfaceWorldMatrix &other); // 0x402340
    SurfaceWorldMatrix operator*(const Matrix16 &other) const; // 0x47dea0
    SurfaceWorldMatrix &SetTranslation(const Vector3f &position); // 0x47e330
};
struct InstancedSurface {
    void *surface;
    Matrix16 matrix; // +0x04
    int identity; // +0x44
    Matrix16 previous;
    int previousIdentity;
    int moveDirty; // +0x8c
    char pad90[0xdc-0x90];
    Vector3f position, previousPosition; // +0xdc, +0xe8
    void Move(const Matrix16 &next); // 0x40c470
    Vector3f ToWorld(const Vector3f &point); // 0x40f850
    Vector3f PointToWorld(const Vector3f &point, const Matrix16 &matrix,
                          int identity, const Matrix16 *turn); // 0x40ce30
    Vector3f ToWorldBefore(const Vector3f &point); // 0x4128a0
    Vector3f ToLocal(const Vector3f &point); // 0x40f6f0
    Vector3f Carried(const Vector3f &point); // 0x40d370
    SurfaceWorldMatrix MatrixToWorld(const Matrix16 &local, const Matrix16 &matrix,
                                     int identity); // 0x40d070
    Vector3f PointToLocal(const Vector3f &point, const Matrix16 &matrix,
                          int identity, const Matrix16 *inverse); // 0x40c8a0
    SurfaceWorldMatrix MatrixToLocal(const Matrix16 &world, const Matrix16 &matrix,
                                     int identity); // 0x40cb30
};

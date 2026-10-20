// SQZ.EXE 0x42bfd0: prefixed placement keys, Euler rotation or normalized direction/up.
#include <string.h>
#include "IniReader/IniReader.h"

struct PlacementVector {
    float x, y, z;
    PlacementVector() {}
    PlacementVector(float a, float b, float c) : x(a), y(b), z(c) {}
    ~PlacementVector();                              // 0x401660
    float Normalize();                               // 0x47d630
};
struct PlacementMatrix {
    float m[16];
    PlacementMatrix() {}
    ~PlacementMatrix();                              // 0x402310
    PlacementMatrix &operator=(const PlacementMatrix &o) {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
        return *this;
    }
    PlacementMatrix &TurnX(float angle);              // 0x47dcd0
    PlacementMatrix &TurnY(float angle);              // 0x47dd20
    PlacementMatrix &TurnZ(float angle);              // 0x47dd70
    PlacementMatrix operator*(const PlacementMatrix &o) const; // 0x47dea0
};
void GameLog(const char *format, ...);                // 0x419290

int ReadPlacement(IniReader *ini, const char *prefix, PlacementVector *position,
                  PlacementVector *scale, PlacementVector *direction, PlacementVector *up)
{
    char key[100];
    strcpy(key, prefix ? prefix : "");
    strcat(key, "Position");
    if (strlen(ini->GetString(key)) == 0) {
        GameLog("WARNING: no position for prop.\n");
        *position = PlacementVector(0.0f, 0.0f, 0.0f);
    } else ini->GetFloats(key, (float *)position);

    strcpy(key, prefix ? prefix : "");
    strcat(key, "Scale");
    if (strlen(ini->GetString(key)) == 0)
        *scale = PlacementVector(1.0f, 1.0f, 1.0f);
    else ini->GetFloats(key, (float *)scale);

    strcpy(key, prefix ? prefix : "");
    strcat(key, "Rotation");
    if (strlen(ini->GetString(key)) != 0) {
        PlacementVector angles;
        ini->GetFloats(key, (float *)&angles);
        PlacementMatrix m, rx, ry, rz;
        m.m[0] = 1.0f; m.m[1] = 0.0f; m.m[2] = 0.0f; m.m[3] = 0.0f;
        m.m[4] = 0.0f; m.m[5] = 1.0f; m.m[6] = 0.0f; m.m[7] = 0.0f;
        m.m[8] = 0.0f; m.m[9] = 0.0f; m.m[10] = 1.0f; m.m[11] = 0.0f;
        m.m[12] = 0.0f; m.m[13] = 0.0f; m.m[14] = 0.0f; m.m[15] = 1.0f;
        rx.TurnX(angles.x * 0.017453292f);
        m = m * rx;
        ry.TurnY(angles.y * 0.017453292f);
        m = m * ry;
        rz.TurnZ(angles.z * 0.017453292f);
        m = m * rz;
        direction->x = m.m[8]; direction->y = m.m[9]; direction->z = m.m[10];
        up->x = m.m[4]; up->y = m.m[5]; up->z = m.m[6];
    } else {
        strcpy(key, prefix ? prefix : "");
        strcat(key, "RotationZdir");
        if (strlen(ini->GetString(key)) == 0)
            *direction = PlacementVector(0.0f, 0.0f, 1.0f);
        else {
            ini->GetFloats(key, (float *)direction);
            direction->Normalize();
        }
        strcpy(key, prefix ? prefix : "");
        strcat(key, "RotationYup");
        if (strlen(ini->GetString(key)) == 0)
            *up = PlacementVector(0.0f, 1.0f, 0.0f);
        else {
            ini->GetFloats(key, (float *)up);
            up->Normalize();
        }
    }
    return 1;
}

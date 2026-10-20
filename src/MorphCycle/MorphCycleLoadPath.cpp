// MorphCycle::LoadPath (SQZ.EXE 0x41c530): build a one-part clip (part "part") from an INI path section. Every key of
// the section is TimeN, PositionN, RotZdirN/RotationZdirN, RotYupN/RotationYupN or RotationN (euler degrees, X then Y
// then Z). A new N starts a sample that copies the previous one (the first gets time 0, position 0 and the orientation
// (dir 0,0,1, up 0,1,0)); the key then overwrites its field of the newest sample. TimeN is the step to the next
// sample: the clip's times are the running sums. Returns 0 for an empty section, else 1.
#include <stdio.h>
#include <string.h>
#define MORPH_CYCLE_PATH_INTERFACE
#include "MorphCycle/MorphCycle.h"
#include "PartCycle/PartCycle.h"
#include "IniReader/IniReader.h"
#include "Container/ContainerTable.h"
#include "String/String.h"

typedef Table<String, PartCycle *, Array<PartCycle *>, 0> PartCycleTable;
typedef PrimitiveArray<float, 2> TimeList;
typedef PrimitiveArray<Vector3f, 4> PositionList;
void DebugLog(const char *format, ...);               // 0x419290

// SIZE 0x40
struct PathMatrix {
    float m[16];                                      // 0x00
    PathMatrix()
    {
        m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
        m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f;
        m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
        m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
    }
    PathMatrix &operator=(const PathMatrix &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
        return *this;
    }
    ~PathMatrix();                                    // 0x402310
    PathMatrix &TurnX(float angle);                   // 0x47dcd0
    PathMatrix &TurnY(float angle);                   // 0x47dd20
    PathMatrix &TurnZ(float angle);                   // 0x47dd70
    PathMatrix operator*(const PathMatrix &o) const;  // 0x47dea0
};

static const float kDegToRad = 0.017453292f;          // 0x49d780

int MorphCycle::LoadPath(IniReader *ini, const char *name)
{
    ini->SetSection(name);
    TimeList sampleTimes;
    PositionList positions;
    Array<PartOrientation> orientations;
    StringList *keys = ini->GetKeys();
    if (keys->count == 0) {
        delete keys;
        return 0;
    }
    int last = -1;
    const char *names[7] = { "Time", "Position", "RotZdir", "RotationZdir", "RotYup", "RotationYup", "Rotation" };
    const char *formats[7] = { "Time%d", "Position%d", "RotZdir%d", "RotationZdir%d", "RotYup%d", "RotationYup%d",
                               "Rotation%d" };
    for (int i = 0; i < keys->count; i++) {
        // The original keeps the text of a String temporary that is already gone.
        const char *key = keys->Get(i).c_str();
        for (int k = 0; k < 7; k++) {
            if (strstr(key, names[k])) {
                int index;
                sscanf(keys->Get(i).c_str(), formats[k], &index);
                if (index != last) {
                    last = index;
                    float time;
                    if (sampleTimes.count <= 1)
                        time = i == 0 ? 0.0f : 1.0f;
                    else
                        time = sampleTimes.data[sampleTimes.count - 1];
                    sampleTimes.Insert(time, -1);
                    positions.Insert(positions.count > 0 ? positions.Get(positions.count - 1) : Vector3f(0, 0, 0), -1);
                    PartOrientation orientation;
                    if (orientations.count > 0) {
                        orientation = orientations.Get(orientations.count - 1);
                    } else {
                        orientation.direction = Vector3f(0, 0, 1);
                        orientation.up = Vector3f(0, 1, 0);
                    }
                    orientations.Insert(orientation, -1);
                }
                break;
            }
        }
        if (strstr(keys->Get(i).c_str(), "Time")) {
            float step = ini->GetFloat(keys->Get(i).c_str(), 1.0f);
            sampleTimes.Replace(sampleTimes.count - 1, step);
        } else if (strstr(keys->Get(i).c_str(), "Position")) {
            Vector3f position;
            ini->GetFloats(keys->Get(i).c_str(), &position.x);
            positions.Replace(positions.count - 1, position);
        } else if (strstr(keys->Get(i).c_str(), "RotZdir") || strstr(keys->Get(i).c_str(), "RotationZdir")) {
            Vector3f direction;
            ini->GetFloats(keys->Get(i).c_str(), &direction.x);
            direction.Normalize();
            PartOrientation orientation;
            orientation.direction = direction;
            orientation.up = orientations.Get(orientations.count - 1).up;
            orientations.Replace(orientations.count - 1, orientation);
        } else if (strstr(keys->Get(i).c_str(), "RotYup") || strstr(keys->Get(i).c_str(), "RotationYup")) {
            Vector3f up;
            ini->GetFloats(keys->Get(i).c_str(), &up.x);
            up.Normalize();
            PartOrientation orientation;
            orientation.direction = orientations.Get(orientations.count - 1).direction;
            orientation.up = up;
            orientations.Replace(orientations.count - 1, orientation);
        } else if (strstr(keys->Get(i).c_str(), "Rotation")) {
            Vector3f angles;
            ini->GetFloats(keys->Get(i).c_str(), &angles.x);
            PathMatrix rotation;
            PathMatrix turnX;
            PathMatrix turnY;
            PathMatrix turnZ;
            turnX.TurnX(angles.x * kDegToRad);
            rotation = rotation * turnX;
            turnY.TurnY(angles.y * kDegToRad);
            rotation = rotation * turnY;
            turnZ.TurnZ(angles.z * kDegToRad);
            rotation = rotation * turnZ;
            PartOrientation orientation;
            orientation.direction.x = rotation.m[8];
            orientation.direction.y = rotation.m[9];
            orientation.direction.z = rotation.m[10];
            orientation.up.x = rotation.m[4];
            orientation.up.y = rotation.m[5];
            orientation.up.z = rotation.m[6];
            orientations.Replace(orientations.count - 1, orientation);
        } else {
            const char *bad = keys->Get(i).c_str();
            DebugLog("WARNING: couldn't parse path entry named %s.\n", bad);
        }
    }
    delete keys;
    PartCycle *track = new PartCycle(positions.count, positions.Data(), orientations.count, orientations.Data(), 0, 0,
                                     this);
    ((PartCycleTable *)parts)->Insert(String("part"), track, -1);
    float total = 0.0f;
    if (sampleTimes.count > 1) {
        for (int j = 0; j < sampleTimes.count; j++) {
            total += sampleTimes.data[j];
            ((TimeList *)times)->Insert(total, -1);
        }
    }
    duration = total;
    if (total > 0)
        period = ((TimeList *)times)->Get(1) - ((TimeList *)times)->Get(0);
    else
        period = 1.0f;
    startTime = 0.0f;
    firstSample = 0;
    sampleCount = sampleTimes.count;
    return 1;
}

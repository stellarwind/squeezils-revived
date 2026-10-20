// Animation: see Animation.h. Reconstructed from SQZ.EXE 0x419530-0x41bd5a.
#include "Animation.h"

// The containers have no virtual destructor in the original either (Container.h), and the objects deleted here are exactly the
// classes the original deletes, so the warning does not apply.
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

// ---- PartPose ----

// @004193c0
PartPose::PartPose(const Vector3f &p, const Vector3f &d, const Vector3f &u, const WeightList *w)
    : position(p), direction(d), up(u)
{
    if (w) {
        for (int i = 0; i < w->count; i++)
            weights.Insert(w->At(i), -1);
    }
}

PartPose::PartPose(const PartPose &o)
    : position(o.position), direction(o.direction), up(o.up), weights(o.weights)
{
}

// @00419530 The weights must already be there: it replaces them one by one and never makes room.
void PartPose::Assign(const PartPose &o)
{
    position = o.position;
    direction = o.direction;
    up = o.up;
    for (int i = 0; i < o.weights.count; i++)
        weights.Replace(i, o.weights.At(i));
}

// @004195b0 The direction is taken as it is; "up" is squared up against it so the three rows are at right angles.
void PartPose::ToMatrix(float m[16]) const
{
    Vector3f right = Cross(up, direction);
    Normalize(right);
    Vector3f squaredUp = Cross(direction, right);
    Normalize(squaredUp);

    m[0] = right.x;       m[1] = right.y;       m[2] = right.z;       m[3] = 0.0f;
    m[4] = squaredUp.x;   m[5] = squaredUp.y;   m[6] = squaredUp.z;   m[7] = 0.0f;
    m[8] = direction.x;   m[9] = direction.y;   m[10] = direction.z;  m[11] = 0.0f;
    m[12] = position.x;   m[13] = position.y;   m[14] = position.z;   m[15] = 1.0f;
}

// ---- PartCycle ----

// @00419750
PartCycle::PartCycle(int positionCount, const Vector3f *positionData, int orientationCount, const PartOrientation *orientationData,
                     int morphCount, WeightList **morphData, MorphCycle *owner)
    : positions(positionCount, positionData),
      orientations(orientationCount, orientationData),
      morph(morphCount, morphData),
      cycle(owner),
      weightCount(-1)
{
}

// @00419880
PartCycle::~PartCycle()
{
    for (int i = 0; i < morph.count; i++) {
        WeightList *weights = morph.At(i);
        if (weights)
            delete weights;
    }
}

// @0041b2a0
int PartCycle::FindSegment(float time, int *hint)
{
    if (!(cycle->TimeAt(0) < time)) {
        *hint = 0;
        return -1;
    }
    int last = cycle->SampleCount() - 1;
    if (cycle->TimeAt(last) <= time) {
        *hint = last;
        return -1;
    }
    if (*hint != -1 && cycle->TimeAt(*hint) <= time) {
        // the old interval is at or before the time: walk forward
        while (cycle->TimeAt(*hint + 1) <= time)
            (*hint)++;
        return *hint;
    }
    // no usable hint (none yet, or the time went backwards): search from the first sample
    for (int i = 0; i <= last; i++) {
        if (!(cycle->TimeAt(i) <= time)) {
            *hint = i - 1;
            return *hint;
        }
    }
    return *hint;
}

// @00419960
int PartCycle::Sample(PartPose *out, float time, int hint)
{
    int segment = FindSegment(time, &hint);
    int known = morph.count > 0 ? morph.At(0)->count : -1;     // how many weights the samples have

    if (segment != -1) {
        // between sample hint and hint + 1, f of the way
        float t0 = cycle->TimeAt(hint);
        float t1 = cycle->TimeAt(hint + 1);
        float f = (time - t0) / (t1 - t0);
        float rest = 1.0f - f;

        if (positions.count > 0)
            out->position = positions.At(hint) * rest + positions.At(hint + 1) * f;
        else
            out->position = Vector3f(0.0f, 0.0f, 0.0f);

        if (orientations.count > 0)
            out->direction = orientations.data[hint].direction * rest + orientations.data[hint + 1].direction * f;
        else
            out->direction = Vector3f(0.0f, 0.0f, 1.0f);
        Normalize(out->direction);

        if (orientations.count > 0)
            out->up = orientations.data[hint].up * rest + orientations.data[hint + 1].up * f;
        else
            out->up = Vector3f(0.0f, 1.0f, 0.0f);
        Normalize(out->up);

        if (morph.count > 0) {
            for (int k = 0; k < out->weights.count; k++) {
                float w;
                if (known > 0 && k < known)
                    w = morph.At(hint)->At(k) * rest + morph.At(hint + 1)->At(k) * f;
                else
                    w = 0.0f;
                out->weights.Replace(k, w);
            }
        }
        return hint;
    }

    // outside the samples: the first or last one as it is
    if (positions.count > 0)
        out->position = positions.At(hint);
    else
        out->position = Vector3f(0.0f, 0.0f, 0.0f);
    if (orientations.count > 0)
        out->direction = orientations.data[hint].direction;
    else
        out->direction = Vector3f(0.0f, 0.0f, 1.0f);
    if (orientations.count > 0)
        out->up = orientations.data[hint].up;
    else
        out->up = Vector3f(0.0f, 1.0f, 0.0f);

    if (morph.count > 0) {
        for (int k = 0; k < out->weights.count; k++) {
            float w = k < known ? morph.At(hint)->At(k) : 0.0f;
            out->weights.Replace(k, w);
        }
    }
    return hint;
}

// @0041a3f0
PartPose *PartCycle::NewPose(float time, int hint)
{
    Vector3f zero(0.0f, 0.0f, 0.0f);
    WeightList *sizing = NULL;
    bool ownsSizing = false;

    if (weightCount == -1) {
        // not known yet: the first sample's list gives the size (and the pose copies its values, which Sample then replaces)
        weightCount = morph.count > 0 ? morph.At(0)->count : 0;
        sizing = weightCount > 0 ? morph.At(0) : NULL;
    } else {
        sizing = new WeightList;
        ownsSizing = true;
        for (int i = 0; i < weightCount; i++)
            sizing->Insert(0.0f, -1);
    }

    PartPose *pose = new PartPose(zero, zero, zero, sizing);
    if (ownsSizing && sizing)
        delete sizing;

    Sample(pose, time, hint);
    return pose;
}

// @0041a5d0
int PartCycle::Blend(PartPose *pose, float time, float weight, int hint)
{
    int segment = FindSegment(time, &hint);
    int known = morph.count > 0 ? morph.At(0)->count : -1;
    float keep = 1.0f - weight;                 // how much of the pose that is already there stays

    if (segment != -1) {
        float t0 = cycle->TimeAt(hint);
        float t1 = cycle->TimeAt(hint + 1);
        float f = (time - t0) / (t1 - t0);
        float rest = 1.0f - f;

        Vector3f samplePosition;
        if (positions.count > 0)
            samplePosition = positions.At(hint) * rest + positions.At(hint + 1) * f;
        else
            samplePosition = Vector3f(0.0f, 0.0f, 0.0f);
        pose->position = pose->position * keep + samplePosition * weight;

        Vector3f sampleDirection;
        if (orientations.count > 0)
            sampleDirection = orientations.data[hint].direction * rest + orientations.data[hint + 1].direction * f;
        else
            sampleDirection = Vector3f(0.0f, 0.0f, 1.0f);
        pose->direction = pose->direction * keep + sampleDirection * weight;
        Normalize(pose->direction);

        Vector3f sampleUp;
        if (orientations.count > 0)
            sampleUp = orientations.data[hint].up * rest + orientations.data[hint + 1].up * f;
        else
            sampleUp = Vector3f(0.0f, 1.0f, 0.0f);
        pose->up = pose->up * keep + sampleUp * weight;
        Normalize(pose->up);

        if (morph.count > 0) {
            for (int k = 0; k < pose->weights.count; k++) {
                float sample;
                if (known > 0 && k < known)
                    sample = morph.At(hint)->At(k) * rest + morph.At(hint + 1)->At(k) * f;
                else
                    sample = 0.0f;
                pose->weights.Replace(k, pose->weights.At(k) * keep + weight * sample);
            }
        }
        return hint;
    }

    Vector3f samplePosition = positions.count > 0 ? positions.At(hint) : Vector3f(0.0f, 0.0f, 0.0f);
    pose->position = pose->position * keep + samplePosition * weight;

    Vector3f sampleDirection = orientations.count > 0 ? orientations.data[hint].direction : Vector3f(0.0f, 0.0f, 1.0f);
    pose->direction = pose->direction * keep + sampleDirection * weight;
    Normalize(pose->direction);

    Vector3f sampleUp = orientations.count > 0 ? orientations.data[hint].up : Vector3f(0.0f, 1.0f, 0.0f);
    pose->up = pose->up * keep + sampleUp * weight;
    Normalize(pose->up);

    if (morph.count > 0) {
        for (int k = 0; k < pose->weights.count; k++) {
            // Unlike the case above, a weight the track does not have is set to 0 rather than blended towards 0.
            float w = k < known ? morph.At(hint)->At(k) * weight + pose->weights.At(k) * keep : 0.0f;
            pose->weights.Replace(k, w);
        }
    }
    return hint;
}

// ---- PoseTable ----

// @0041b3f0
PoseTable::PoseTable()
{
    UseOriginalVtables(static_cast<PoseTableBase *>(this));     // the derived constructor would otherwise leave the compiler's own vtable
}

// @0041b440
PoseTable::PoseTable(const PoseTable &o)
{
    for (int i = 0; i < o.keys.count; i++) {
        PartPose *copy = new PartPose(*o.values.data[i]);
        Insert(o.keys.data[i], copy, -1);
    }
    UseOriginalVtables(static_cast<PoseTableBase *>(this));
}

// @0041b5f0
PoseTable::~PoseTable()
{
    for (int i = 0; i < values.count; i++) {
        PartPose *pose = values.data[i];
        if (pose)
            delete pose;
    }
}

// @0041b6e0 No check that the part is there.
PartPose *PoseTable::Find(const char *partName)
{
    return Get(String(partName));
}

// @0041b760
void PoseTable::Assign(const PoseTable &o)
{
    for (int i = 0; i < o.keys.count; i++)
        values.data[i]->Assign(*o.values.data[i]);
}

// ---- MorphCycle ----

// @0041b7b0
MorphCycle::MorphCycle(int noLoopFlag, float playSpeed, float samplePeriod)
{
    noLoop = noLoopFlag;
    times = new WeightList;
    parts = new PartCycleTable;
    period = samplePeriod;
    speed = playSpeed;
    sharesData = 0;
}

// @0041b8b0
MorphCycle::MorphCycle(const MorphCycle &whole, int first, int last, int noLoopFlag, float playSpeed, float samplePeriod)
{
    noLoop = noLoopFlag;
    period = samplePeriod == -1.0f ? whole.period : samplePeriod;
    firstSample = first;
    sampleCount = last - first + 1;
    speed = playSpeed;
    startTime = (float)first * period;
    duration = (float)(last - first) * period;
    times = whole.times;
    sharesData = 1;
    parts = whole.parts;
}

// @0041b940 A sub-clip owns nothing.
MorphCycle::~MorphCycle()
{
    if (sharesData)
        return;
    for (int i = 0; i < parts->values.count; i++) {
        PartCycle *track = parts->values.data[i];
        if (track)
            delete track;
    }
    delete parts;
    delete times;
}

// @0041ba70
PoseTable *MorphCycle::NewPoseTable()
{
    PoseTable *table = new PoseTable;
    for (int i = 0; i < parts->keys.count; i++) {
        PartPose *pose = parts->values.data[i]->NewPose(startTime, -1);
        table->Insert(parts->keys.data[i], pose, -1);
    }
    return table;
}

// @0041bbf0 Every part starts its search from the hint it was given; the one that comes back is the last part's (they all share the same
// sample times, so it is the same for each).
int MorphCycle::SamplePose(PoseTable *pose, float time, int hint)
{
    int result = hint;
    for (int i = 0; i < parts->keys.count; i++)
        result = parts->values.data[i]->Sample(pose->values.data[i], startTime + time, hint);
    return result;
}

// @0041bcf0
int MorphCycle::BlendPose(PoseTable *pose, float time, float weight, int hint)
{
    int result = hint;
    for (int i = 0; i < parts->keys.count; i++)
        result = parts->values.data[i]->Blend(pose->values.data[i], startTime + time, weight, hint);
    return result;
}

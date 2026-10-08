// Animation: the data a character animation is made of, and how a time in the animation becomes a pose. Reconstructed from
// SQZ.EXE 0x419530-0x41bd5a. The class names are the original's (RTTI shows them inside the container names).
//
//   MorphCycle   one animation clip ("walk", "idle" ...): a list of sample times and, for every body part, a PartCycle
//   PartCycle    one body part's track in a clip: its position, direction/up vectors and morph weights at every sample time
//   PartPose     where one body part is at one moment: position, direction, up and the morph weights (a BodyPart applies it)
//   PoseTable    the PartPoses of every part of a body, by part name: the pose a character is in right now
//
// Sampling a clip at a time means finding the two samples around the time and blending their values (position and weights
// linearly, directions linearly and then back to length 1). Blending (BlendPose) moves the pose
// that is already there part of the way towards a clip's values, which is how a character changes from one clip to the next.
//
// The sub-clips: the game cuts one long clip (a "cycle file" with many frames) into named pieces, each a MorphCycle that
// shares the long clip's times and tracks (sharesData 1) and only says which frames it covers. The PartCycles keep a pointer
// to the clip that owns the times, so a sub-clip is sampled at its start time plus the time asked for.
//
// Objects of all these classes are also made, used and freed by original code, so they are the same sizes and layouts and
// live on the game's heap (GameHeapObject, or the same operators spelled out). They have no virtual functions of their own (PoseTable is a
// Table, which has the original's).
#pragma once
#include "Vector3f.h"
#include "Container.h"

typedef PrimitiveArray<float, 2> WeightList;     // morph weights, one per morph target of a part's model

// A direction and an up vector (RTTI _tag_PartOrientation, 24 bytes).
struct PartOrientation {
    Vector3f direction;
    Vector3f up;
};

struct PartPose : GameHeapObject {
    Vector3f   position;            // 0x00
    Vector3f   direction;           // 0x0c
    Vector3f   up;                  // 0x18
    WeightList weights;             // 0x24 (0x14 bytes)

    PartPose(const Vector3f &position, const Vector3f &direction, const Vector3f &up, const WeightList *weights);   // @004193c0 weights may be NULL
    PartPose(const PartPose &o);                                  // member by member, inlined where poses are copied
    void Assign(const PartPose &o);                               // @00419530 copies the vectors and replaces this pose's weights one by one
    void ToMatrix(float m[16]) const;                             // @004195b0 rows: right (up x direction), up (direction x right), direction, position
};

struct MorphCycle;

struct PartCycle {
    GAME_HEAP_OPERATORS

    PrimitiveArray<Vector3f, 4> positions;       // 0x00 one per sample time (may be empty: the part does not move)
    Array<PartOrientation>      orientations;    // 0x14 one per sample time (may be empty)
    Array<WeightList *>         morph;           // 0x24 one weight list per sample time (may be empty)
    MorphCycle *cycle;                           // 0x34 the clip that owns the sample times
    int         weightCount;                     // 0x38 how many weights a sample has; -1 until NewPose finds out

    PartCycle(int positionCount, const Vector3f *positionData, int orientationCount, const PartOrientation *orientationData,
              int morphCount, WeightList **morphData, MorphCycle *owner);   // @00419750 copies the three lists
    ~PartCycle();                                                 // @00419880 frees the weight lists it owns

    // The sample interval around a time. Returns the index of the sample at or before the time, or -1 when the time is outside
    // the clip's samples (then *hint is the first or last sample). *hint is the previous answer: searching goes on from there.
    int  FindSegment(float time, int *hint);                      // @0041b2a0
    // Fills a pose with this part's values at a time and returns the new hint (the interval found).
    int  Sample(PartPose *out, float time, int hint);             // @00419960
    PartPose *NewPose(float time, int hint);                      // @0041a3f0 a pose of this part at a time, its weights sized like the track's
    // Moves a pose part of the way (weight 0 .. 1) to this part's values at a time; returns the new hint.
    int  Blend(PartPose *pose, float time, float weight, int hint);   // @0041a5d0
};
static_assert(sizeof(PartOrientation) == 0x18, "PartOrientation size");
static_assert(sizeof(PartPose) == 0x38, "PartPose size");
static_assert(sizeof(PartCycle) == 0x3c, "PartCycle size");

// The current pose of a whole body: a PartPose for every part, in the order of the part names.
struct PoseTable : PoseTableBase {
    PoseTable();                                                  // @0041b3f0 empty
    PoseTable(const PoseTable &o);                                // @0041b440 a copy with a copy of every pose
    ~PoseTable();                                                 // @0041b5f0 frees the poses
    PartPose *Find(const char *partName);                         // @0041b6e0
    PartPose *At(int index) { return values.data[index]; }        // @0041b750
    int       Count() { return keys.count; }                      // @0041b6d0
    void      Assign(const PoseTable &o);                         // @0041b760 pose by pose (same parts, same order)
};
static_assert(sizeof(PoseTable) == 0x28, "PoseTable size");

struct MorphCycle : GameHeapObject {
    float duration;                 // 0x00 seconds from the first to the last frame of this clip
    float period;                   // 0x04 seconds between two samples
    float startTime;                // 0x08 where this clip starts in the shared samples (0 for a whole clip)
    int   firstSample;              // 0x0c index of its first frame in the shared samples
    int   noLoop;                   // 0x10 nonzero: playing past the end stops instead of looping
    int   sampleCount;              // 0x14 number of frames in this clip
    float speed;                    // 0x18 playing time is divided by this
    int   sharesData;               // 0x1c 1 for a sub-clip: the times and tracks belong to another clip
    PrimitiveArray<float, 2> *times;    // 0x20 the sample times (seconds)
    PartCycleTable *parts;          // 0x24 the tracks by part name

    MorphCycle(int noLoop, float speed, float period);            // @0041b7b0 empty clip with its lists made; the loaders fill duration, count ...
    MorphCycle(const MorphCycle &whole, int first, int last, int noLoop, float speed, float period);   // @0041b8b0 frames first..last of another clip (period -1: its period)
    ~MorphCycle();                                                // @0041b940

    float Duration() { return duration; }                         // @0041ba00
    float Period()   { return period; }                           // @0041ba10
    float Speed()    { return speed; }                            // @0041ba20
    int   SampleCount() { return sampleCount; }                   // @0041ba30
    int   NoLoop()   { return noLoop; }                           // @0041ba40
    float TimeAt(int sample) { return times->At(firstSample + sample); }   // @0041ba50

    PoseTable *NewPoseTable();                                    // @0041ba70 a pose table holding every part's pose at the start of the clip
    int  SamplePose(PoseTable *pose, float time, int hint);       // @0041bbf0 every part of the table set to the clip's values at a time; returns the hint
    int  BlendPose(PoseTable *pose, float time, float weight, int hint);   // @0041bcf0 the same, but only moving the table part of the way
};
static_assert(sizeof(MorphCycle) == 0x28, "MorphCycle size");

// MorphCycle: a named animation clip over shared samples (SQZ.EXE 0x41b7b0-0x41c000), VC++ 4.1 dialect.
// Layout from Repo/legacy_gcc/Animation.h. This unit holds the five trivial getters (0x41ba00-0x41ba40); the
// larger members live in their own files (MorphCycleLoad.cpp, MorphCycleLoadPath.cpp, ...).
#pragma once
#include "compat41.h"
#ifdef MORPH_CYCLE_SAMPLE_INTERFACE
struct PoseTable;
#endif

struct MorphCycle {
    float duration;                 // 0x00
    float period;                   // 0x04
    float startTime;                // 0x08
    int   firstSample;              // 0x0c
    int   noLoop;                   // 0x10
    int   sampleCount;              // 0x14
    float speed;                    // 0x18
    int   sharesData;               // 0x1c
    void *times;                    // 0x20 PrimitiveArray<float,2>*
    void *parts;                    // 0x24 PartCycleTable*

    MorphCycle(int noLoop, float speed, float period);   // 0x41b7b0
    MorphCycle(const MorphCycle &whole, int first, int last, int noLoop, float speed, float period);   // 0x41b8b0
    ~MorphCycle();                  // 0x41b940
    float Duration();               // 0x41ba00
    float Period();                 // 0x41ba10
    float Speed();                  // 0x41ba20
    int   SampleCount();            // 0x41ba30
    int   NoLoop();                 // 0x41ba40
    float TimeAt(int sample);       // 0x41ba50
#ifdef MORPH_CYCLE_SAMPLE_INTERFACE
    int SamplePose(PoseTable *pose, float time, int flag); // 0x41bbf0
#endif
#ifdef MORPH_CYCLE_LOAD_INTERFACE
    // Opt in to preserve the symbol count in existing byte-match units.
    int Load(const char *filename); // 0x41bd60
#endif
#ifdef MORPH_CYCLE_PATH_INTERFACE
    int LoadPath(class IniReader *ini, const char *name); // 0x41c530
#endif
};

// MorphCycle::Load (SQZ.EXE 0x41bd60): read a clip from a binary ".sqa" file. Layout: the header "#SQA V1.0 binary" with
// its 0, then part count, duration, time-sample count and the time samples, then per part: name length, name, flags
// (1 positions, 2 orientations, 4 morph weights) and one record of each kind present per time sample. A clip with more
// than one sample gets one extra sample that repeats the first, one sample step after the last, so it can loop.
// Returns 1, or 0 with a log line when the file cannot be opened or is short.
#include <stdio.h>
#include <string.h>
#pragma intrinsic(strlen, strcmp)
#define MORPH_CYCLE_LOAD_INTERFACE
#include "MorphCycle/MorphCycle.h"
#include "PartCycle/PartCycle.h"
#include "Container/ContainerTable.h"
#include "String/String.h"

typedef Table<String, PartCycle *, Array<PartCycle *>, 0> PartCycleTable;
typedef PrimitiveArray<float, 2> TimeList;
void *GameAlloc(unsigned n);                          // 0x448350
void GameFree(void *memory);                          // 0x448360
int DebugLogAll(const char *format, ...);               // 0x419290

int MorphCycle::Load(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return 0;
    char header[20];
    fread(header, strlen("#SQA V1.0 binary") + 1, 1, f);
    if (ferror(f)) {
        DebugLogAll("Can't read file header\n");
        fclose(f);
        return 0;
    }
    if (strcmp((char *)header, "#SQA V1.0 binary")) {
        DebugLogAll("File header is wrong for SQA file %s", filename);
        fclose(f);
        return 0;
    }
    int partCount;
    fread(&partCount, 4, 1, f);
    if (ferror(f)) {
        DebugLogAll("Can't read part count\n");
        fclose(f);
        return 0;
    }
    fread(&duration, 4, 1, f);
    if (ferror(f)) {
        DebugLogAll("Can't read duration\n");
        fclose(f);
        return 0;
    }
    int count;
    fread(&count, 4, 1, f);
    if (ferror(f)) {
        DebugLogAll("Can't read time sample count\n");
        fclose(f);
        return 0;
    }
    int total = count;
    int wrap = 0;
    if (count > 1) {
        wrap = 1;
        total = count + 1;
    }
    float *samples = (float *)GameAlloc(total * 4);
    fread(samples, 4, count, f);
    if (ferror(f)) {
        DebugLogAll("Can't read time samples\n");
        fclose(f);
        return 0;
    }
    if (wrap) {
        float step = samples[1] - samples[0];
        samples[count] = samples[count - 1] + step;
        duration = step + duration;
    }
    float *sample = samples;
    for (int n = total; n > 0; n--, sample++)
        ((TimeList *)times)->Insert(*sample, -1);
    GameFree(samples);
    if (duration > 0.0f)
        period = ((TimeList *)times)->Get(1) - ((TimeList *)times)->Get(0);
    startTime = 0.0f;
    firstSample = 0;
    sampleCount = total;

    Vector3f *positions = (Vector3f *)GameAlloc(total * 0xc);
    PartOrientation *orientations = (PartOrientation *)GameAlloc(total * 0x18);
    WeightList **morphs = (WeightList **)GameAlloc(total * 4);
    for (int part = 0; part < partCount; part++) {
        unsigned nameLength;
        char name[128];
        unsigned flags;
        fread(&nameLength, 4, 1, f);
        if (ferror(f)) {
            DebugLogAll("Can't read part string header\n");
            fclose(f);
            return 0;
        }
        fread(name, nameLength, 1, f);
        if (ferror(f)) {
            DebugLogAll("Can't read part string name\n");
            fclose(f);
            return 0;
        }
        fread(&flags, 4, 1, f);
        if (ferror(f)) {
            DebugLogAll("Can't read part flags\n");
            fclose(f);
            return 0;
        }
        int positionCount = 0;
        int orientationCount = 0;
        int morphCount = 0;
        if (flags & 1) {
            positionCount = count;
            fread(positions, 0xc, count, f);
            if (ferror(f)) {
                DebugLogAll("Can't read part positions\n");
                fclose(f);
                return 0;
            }
            if (wrap) {
                positionCount++;
                positions[count] = positions[0];
            }
        }
        if (flags & 2) {
            orientationCount = count;
            fread(orientations, 0x18, count, f);
            if (ferror(f)) {
                DebugLogAll("Can't read part orientations\n");
                fclose(f);
                return 0;
            }
            if (wrap) {
                orientations[count] = orientations[0];
                orientationCount++;
            }
        }
        if (flags & 4) {
            morphCount = count;
            unsigned weightCount;
            fread(&weightCount, 4, 1, f);
            if (ferror(f)) {
                DebugLogAll("Can't read morph vector count\n");
                fclose(f);
                return 0;
            }
            float weights[128];
            WeightList **morph = morphs;
            for (int i = 0; i < count; i++) {
                fread(weights, 4, weightCount, f);
                if (ferror(f)) {
                    DebugLogAll("Can't read morph value data\n");
                    fclose(f);
                    return 0;
                }
                *morph++ = new WeightList(weightCount, weights);
            }
            if (wrap) {
                morphCount++;
                morphs[count] = new WeightList(weightCount, morphs[0]->Data());
            }
        }
        PartCycle *track = new PartCycle(positionCount, positions, orientationCount, orientations, morphCount, morphs, this);
        ((PartCycleTable *)parts)->Insert(String(name), track, -1);
    }
    GameFree(positions);
    GameFree(orientations);
    GameFree(morphs);
    fclose(f);
    return 1;
}

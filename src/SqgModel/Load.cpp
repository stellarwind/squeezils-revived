// SQZ.EXE 0x472560: read a binary SQG model and optionally mirror its X axis.
#include "SqgModel.h"
#include <stdio.h>
#include <string.h>
#pragma intrinsic(strlen, strcmp)

extern const char *kSqgHeader;                 // 0x4c0f74
extern const char kSqgReadMode[];              // 0x4baa40
extern void *(*Alloc)(size_t n);               // 0x4c0f78
void SqgLog(const char *format, ...);          // 0x473710
extern const char kSqgReadHeader[], kSqgBadHeader[], kSqgReadTextures[], kSqgReadNameLength[],
    kSqgReadName[], kSqgReadVerticesCount[], kSqgNoVertices[], kSqgReadVertices[], kSqgReadIndicesCount[],
    kSqgNoFaces[], kSqgReadIndices[], kSqgReadGroupsCount[], kSqgNoGroups[], kSqgReadGroups[],
    kSqgReadDeltaLists[], kSqgReadDeltaSize[], kSqgReadDeltas[];
    // 0x4baa28, 0x4c1160, 0x4c1144, 0x4c1124, 0x4c1108, 0x4c10ec, 0x4c10d4, 0x4c10bc,
    // 0x4c10a4, 0x4c1090, 0x4c1074, 0x4c105c, 0x4c1044, 0x4c1030, 0x4c1010, 0x4c0fe4, 0x4c0fbc
struct SqgLoadVertex { float x, y, z, rest[6]; }; // 0x24
struct SqgLoadDelta { int vertex; float x, y, z; }; // 0x10
struct SqgLoadDeltaList { int count; SqgLoadDelta *data; }; // 8

#define SQG_READ_ERROR(message) \
    if (ferror(f)) { SqgLog(message); fclose(f); return 0; }

int SqgModel::Load(const char *path, int mirror)
{
    FILE *f = fopen(path, kSqgReadMode);
    if (!f) return 0;
    char header[20];
    fread(header, strlen(kSqgHeader) + 1, 1, f);
    SQG_READ_ERROR(kSqgReadHeader)
    if (strcmp(header, kSqgHeader)) {
        SqgLog(kSqgBadHeader, path);
        fclose(f);
        return 0;
    }
    fread(&textureCount, 4, 1, f);
    SQG_READ_ERROR(kSqgReadTextures)
    int i;
    size_t size;
    if (textureCount >= 1) {
        textureNames = (char **)Alloc(textureCount * 4);
        for (i = 0; i < textureCount; ++i) {
            fread(&size, 4, 1, f);
            SQG_READ_ERROR(kSqgReadNameLength)
            textureNames[i] = (char *)Alloc(size);
            fread(textureNames[i], size, 1, f);
            if (ferror(f)) {
                SqgLog(kSqgReadName, i);
                fclose(f);
                return 0;
            }
        }
    }
    fread(&vertexCount, 4, 1, f);
    SQG_READ_ERROR(kSqgReadVerticesCount)
    if (vertexCount <= 0) {
        SqgLog(kSqgNoVertices);
        fclose(f);
        return 0;
    }
    vertices = Alloc(vertexCount * 0x24);
    fread(vertices, 0x24, vertexCount, f);
    SQG_READ_ERROR(kSqgReadVertices)
    fread(&faceCount, 4, 1, f);
    SQG_READ_ERROR(kSqgReadIndicesCount)
    if (faceCount <= 0) {
        SqgLog(kSqgNoFaces);
        fclose(f);
        return 0;
    }
    fread(&indexCount, 4, 1, f);
    SQG_READ_ERROR(kSqgReadIndicesCount)
    indices = (int *)Alloc(indexCount * 4);
    fread(indices, 4, indexCount, f);
    SQG_READ_ERROR(kSqgReadIndices)
    fread(&groupCount, 4, 1, f);
    SQG_READ_ERROR(kSqgReadGroupsCount)
    if (groupCount <= 0) {
        SqgLog(kSqgNoGroups);
        fclose(f);
        return 0;
    }
    groups = Alloc(groupCount * 0x24);
    fread(groups, 0x24, groupCount, f);
    SQG_READ_ERROR(kSqgReadGroups)
    if (mirror) {
        i = 0;
        while (i < indexCount) {
            int count = indices[i++];
            int swaps = count / 2;
            if (swaps > 0) {
                int front = i;
                int back = i + count;
                do {
                    --back;
                    int temp = indices[front];
                    ++front;
                    indices[front - 1] = indices[back];
                    indices[back] = temp;
                } while (--swaps);
            }
            i += count;
        }
        for (i = 0; i < vertexCount; ++i)
            ((SqgLoadVertex *)vertices)[i].x = -((SqgLoadVertex *)vertices)[i].x;
    }
    fread(&deltaListCount, 4, 1, f);
    SQG_READ_ERROR(kSqgReadDeltaLists)
    if (!deltaListCount) {
        fclose(f);
        return 1;
    }
    deltaLists = Alloc(deltaListCount * 8);
    for (i = 0; i < deltaListCount; ++i) {
        fread(&size, 4, 1, f);
        if (ferror(f)) {
            SqgLog(kSqgReadDeltaSize, i);
            fclose(f);
            return 0;
        }
        ((SqgLoadDeltaList *)deltaLists)[i].count = size;
        ((SqgLoadDeltaList *)deltaLists)[i].data = (SqgLoadDelta *)Alloc(size * 0x10);
        fread(((SqgLoadDeltaList *)deltaLists)[i].data, 0x10, size, f);
        if (ferror(f)) {
            SqgLog(kSqgReadDeltas, i);
            fclose(f);
            return 0;
        }
        if (mirror) {
            for (int j = 0; j < (int)size; ++j) {
                SqgLoadDelta *delta = &((SqgLoadDeltaList *)deltaLists)[i].data[j];
                delta->x = -delta->x;
            }
        }
    }
    fclose(f);
    return 1;
}
#undef SQG_READ_ERROR

// 0x472b90 (ret 4): SqgModel::Save - write the model as a binary .sqg file: header, texture names, vertices,
// face indices, groups and the delta records. 1 on success; 0 (error logged, file closed) on failure.
#include <stdio.h>
#include <string.h>
#pragma intrinsic(strlen)

struct SqgDelta { int count; void *data; };   // 0x10-byte entries
struct SqgSaveModel {
    int textureCount;       // 0x00
    char **textureNames;    // 0x04
    int vertexCount;        // 0x08
    void *vertices;         // 0x0c 0x24 bytes each
    int faceCount;          // 0x10
    int indexCount;         // 0x14
    int *indices;           // 0x18
    int groupCount;         // 0x1c
    void *groups;           // 0x20 0x24 bytes each
    int deltaCount;         // 0x24
    SqgDelta *deltas;       // 0x28
    int Save(const char *file);
};
void SqgLog(const char *format, ...);           // 0x473710
extern const char *kSqgHeader;                  // 0x4c0f74
extern const char kSqgWriteMode[];              // 0x4c1328
extern const char kSqgCantOpen[], kSqgCantWriteHeader[], kSqgCantWriteTextureCount[], kSqgCantWriteTexture[],
    kSqgCantWriteVertexCount[], kSqgCantWriteVertices[], kSqgCantWriteFaceCount[], kSqgCantWriteIndexCount[],
    kSqgCantWriteIndices[], kSqgCantWriteGroupCount[], kSqgCantWriteGroups[], kSqgCantWriteDeltaCount[],
    kSqgCantWriteDeltaSize[], kSqgCantWriteDelta[];
    // 0x4c1304 0x4c12e8 0x4c12cc 0x4c12ac 0x4c1290 0x4c1278 0x4c1260 0x4c1244 0x4c122c 0x4c1210 0x4c11fc 0x4c11dc
    // 0x4c11b0 0x4c1188

int SqgSaveModel::Save(const char *file)
{
    FILE *f = fopen(file, kSqgWriteMode);
    if (!f) {
        SqgLog(kSqgCantOpen, file);
        return 0;
    }
    fwrite(kSqgHeader, strlen(kSqgHeader) + 1, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteHeader);
        fclose(f);
        return 0;
    }
    fwrite(&textureCount, 4, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteTextureCount);
        fclose(f);
        return 0;
    }
    int i;
    if (textureCount >= 1) {
        for (i = 0; i < textureCount; i++) {
            int length = strlen(textureNames[i]) + 1;
            fwrite(&length, 4, 1, f);
            fwrite(textureNames[i], length, 1, f);
            if (ferror(f)) {
                SqgLog(kSqgCantWriteTexture, i);
                fclose(f);
                return 0;
            }
        }
    }
    fwrite(&vertexCount, 4, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteVertexCount);
        fclose(f);
        return 0;
    }
    fwrite(vertices, 0x24, vertexCount, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteVertices);
        fclose(f);
        return 0;
    }
    fwrite(&faceCount, 4, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteFaceCount);
        fclose(f);
        return 0;
    }
    fwrite(&indexCount, 4, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteIndexCount);
        fclose(f);
        return 0;
    }
    fwrite(indices, 4, indexCount, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteIndices);
        fclose(f);
        return 0;
    }
    fwrite(&groupCount, 4, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteGroupCount);
        fclose(f);
        return 0;
    }
    fwrite(groups, 0x24, groupCount, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteGroups);
        fclose(f);
        return 0;
    }
    fwrite(&deltaCount, 4, 1, f);
    if (ferror(f)) {
        SqgLog(kSqgCantWriteDeltaCount);
        fclose(f);
        return 0;
    }
    for (i = 0; i < deltaCount; i++) {
        fwrite(&deltas[i].count, 4, 1, f);
        if (ferror(f)) {
            SqgLog(kSqgCantWriteDeltaSize, i);
            fclose(f);
            return 0;
        }
        fwrite(deltas[i].data, 0x10, deltas[i].count, f);
        if (ferror(f)) {
            SqgLog(kSqgCantWriteDelta, i);
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;
}

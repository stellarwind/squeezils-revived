// 0x472480: parse a WRL file and require at least one new vertex.
#include "SqgModel/SqgModel.h"

struct WrlLoadContext {
    virtual void Dispatch();
    int ok;                         // +0x04
    char state[0x304];
    SqgModel *model;                 // +0x30c
    int flag;                       // +0x310
    WrlLoadContext(const char *, const char *, const char *, SqgModel *, int); // 0x471ab0
    ~WrlLoadContext();               // 0x472550
};
void SetWrlContext(WrlLoadContext *); // 0x47a770
void ParseWrl(const char *);         // 0x47a990
void ClearWrlContext();              // 0x47a780
void WrlError(const char *);         // 0x47a7b0
extern const char kNoWrlVertices[];  // 0x4c0f8c

int SqgModel::LoadWrl(const char *path, int flag)
{
    int oldCount = vertexCount;
    WrlLoadContext context(path, path, path, this, flag);
    SetWrlContext(&context);
    ParseWrl(path);
    ClearWrlContext();
    if (!context.ok)
        return 0;
    if (vertexCount == oldCount) {
        WrlError(kNoWrlVertices);
        return 0;
    }
    return 1;
}

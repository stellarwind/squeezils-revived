// More small leaves: indexed record store (SQZ.EXE 0x46a540), COM-holder release (0x471990), list bulk add (0x44e070),
// zeroing constructor of a 5-field record (0x4719d0).
#include "compat41.h"

struct Rec6 { int w[6]; };
struct RecArray {
    int pad[3];
    Rec6 *data;                              // 0x0c
    void Set(int i, const Rec6 *r);
};
void RecArray::Set(int i, const Rec6 *r)
{
    Rec6 *d = &data[i];
    d->w[0] = r->w[0];
    d->w[1] = r->w[1];
    d->w[2] = r->w[2];
    d->w[3] = r->w[3];
    d->w[4] = r->w[4];
    d->w[5] = r->w[5];
}

typedef void (*FreeFn)(void *);
extern FreeFn g_free;                        // 0x4c0f80
struct Com {
    virtual long __stdcall Q();
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
};
struct ModelParts {
    void *a, *b;
    Com *c;
    void Free();
};
void ModelParts::Free()
{
    if (a)
        g_free(a);
    if (b)
        g_free(b);
    if (c)
        c->Release();
}

struct IdList { virtual int Get(int i); int count; };
struct Sink {
    void Add(int id);                        // 0x44e020
    void AddAll(IdList *l);
};
void Sink::AddAll(IdList *l)
{
    if (l) {
        for (int i = 0; i < l->count; i++)
            Add(l->Get(i));
    }
}


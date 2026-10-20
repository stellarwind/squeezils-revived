// Three small C-style helpers: a buffered byte reader (SQZ.EXE 0x45beb0), a recursive node release (0x47ae10)
// and a typed read that stores through an out pointer (0x47c770).
#include "compat41.h"

struct ByteSrc {
    int pad0, pad4;
    int left;            // 0x08 bytes remaining in the buffer
    int pad0c;
    unsigned char *ptr;  // 0x10
};
int Refill(ByteSrc *s);                       // 0x45b7a0

int GetByte(ByteSrc *s, unsigned char *out)
{
    if (s->left == 0 && Refill(s))
        return 1;
    *out = *s->ptr;
    s->ptr++;
    s->left--;
    return 0;
}

struct Node8 {
    char pad[0x20];
    Node8 *kids[8];
    void Clear();                             // 0x47ae90
    void Release();
};

void Node8::Release()
{
    Clear();
    for (int i = 0; i < 8; i++) {
        Node8 *k = kids[i];
        if (k) {
            k->Release();
            delete k;
            kids[i] = 0;
        }
    }
}

int ReadInt(void *src, float *tmp);           // 0x47c6f0 (4-byte read; the float type gives SQZ.EXE's load order)
int ReadIntTo(void *src, float *out)
{
    float v;
    if (ReadInt(src, &v) != 1)
        return 0;
    *out = v;
    return 1;
}

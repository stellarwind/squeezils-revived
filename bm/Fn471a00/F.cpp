// 0x471a00: frees a model-part table: a pointer array, three buffers and an array of 8-byte records (pointer at +4).
typedef void (*FreeFn)(void *);
extern FreeFn g_free;                        // 0x4c0f80
struct Rec { int tag; void *data; };
struct PartTable {
    int n1;                // 0x00
    void **a1;             // 0x04
    int pad8[1];
    void *p1;              // 0x0c
    int pad10[2];
    void *p2;              // 0x18
    int pad1c;
    void *p3;              // 0x20
    int n2;                // 0x24
    Rec *a2;               // 0x28
    void Free();
};
void PartTable::Free()
{
    if (a1) {
        for (int i = 0; i < n1; i++)
            g_free(a1[i]);
        g_free(a1);
    }
    if (p1) g_free(p1);
    if (p2) g_free(p2);
    if (p3) g_free(p3);
    if (a2) {
        for (int j = 0; j < n2; j++)
            g_free(a2[j].data);
        g_free(a2);
    }
}

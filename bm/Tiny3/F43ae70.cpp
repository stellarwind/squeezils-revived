// 0x43ae70 (ret 8): look up an entry by name in the table at this+4, build it on demand.
struct Ent43 { void *obj; char pad[0x40]; };
struct P43 {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual int FindIndex(const char *name);     // slot 3 (+0xc)
    char pad[0x4c - 4];
    Ent43 ents[1];                               // 0x4c, stride 0x44
    int M43db40(int idx);
    void *M43d660(int idx, int v);
};
int __stdcall S43d750(int idx, int v, int a);
struct T43ae70 {
    int f0;
    P43 *p;     // 0x04
    void *F43ae70(const char *name, int a);
    void M43c7b0(int idx, const char *name);
};
void *T43ae70::F43ae70(const char *name, int a)
{
    void *r = 0;
    int idx = p->FindIndex(name);
    int v = p->M43db40(idx);
    if (idx != -1) {
        r = p->ents[idx].obj;
        if (!r)
            return 0;
        r = p->M43d660(idx, S43d750(idx, v, a));
    }
    if (r)
        M43c7b0(idx, name);
    return r;
}

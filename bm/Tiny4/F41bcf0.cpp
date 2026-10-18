// 4 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
// 0x41bcf0 (ret 0x10): apply the same call to every element of this->group against the matching element of `other`.
struct Elem41 { int Apply(Elem41 *other, float x, int a, int b); };   // 0x41a5d0
struct Group41 {
    char pad[8];
    int count;              // 0x08
    char pad2[0x24 - 0x0c];
    Elem41 **items;         // 0x24
};
struct Owner41 {
    char pad[8];
    float offset;           // 0x08
    char pad2[0x24 - 0x0c];
    Group41 *group;         // 0x24
    int Apply(Group41 *other, float dx, int a, int b);
};
int Owner41::Apply(Group41 *other, float dx, int a, int b)
{
    int r;
    for (int i = 0; i < group->count; i++)
        r = group->items[i]->Apply(other->items[i], offset + dx, a, b);
    return r;
}

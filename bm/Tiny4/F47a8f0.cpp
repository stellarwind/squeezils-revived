// 0x47a8f0 (base, three path strings) and 0x479b10 (derived, adds three ints).
#include <string.h>
struct Paths {
    virtual void v0();
    int one;                // 0x04
    char a[0x100];          // 0x08
    char b[0x100];          // 0x108
    char c[0x100];          // 0x208
    int f308;
    Paths(const char *pa, const char *pb, const char *pc);
};
Paths::Paths(const char *pa, const char *pb, const char *pc)
{
    one = 1;
    strcpy(a, pa);
    strcpy(b, pb);
    strcpy(c, pc);
    f308 = 0;
}
struct PathsEx : Paths {
    int g30c, g310, g314, g318, g31c;
    PathsEx(const char *pa, const char *pb, const char *pc, int x, int y, int z);
};
PathsEx::PathsEx(const char *pa, const char *pb, const char *pc, int x, int y, int z) : Paths(pa, pb, pc)
{
    g30c = x;
    g310 = y;
    g314 = z;
    g318 = z;
    g31c = 0;
}

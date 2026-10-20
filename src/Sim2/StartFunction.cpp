// SqueezilRegionSoul::StartFunction (SQZ.EXE 0x436a60): run the function named by the spot's "Function" key, with the soul as its caller.
#include <string.h>
struct StrF { const char *c_str() const; };               // 0x438530
struct SpotF {
    char pad[0x14];
    StrF function;                                        // 0x14
    char pad2[0x34 - 0x18];
    int prop;                                             // 0x34
};
struct RegionF { SpotF *GetPlace(int id); };              // 0x418140
struct CallerF { char pad[0x18]; CallerF(void *soul, int id, int zero); };   // 0x420bc0
void PostFunction(const char *name, CallerF *caller, int a, int b);   // 0x422ce0 (cdecl)

struct RegionSoulF {
    char pad[0x88];
    RegionF *region;                                      // 0x88
    int location;                                         // 0x8c
    int location2;                                        // 0x90
    void StartFunction();
};

void RegionSoulF::StartFunction()
{
    if (strlen(region->GetPlace(location2)->function.c_str()) != 0) {
        CallerF caller(this, 0, region->GetPlace(location2)->prop);
        const char *n = region->GetPlace(location2)->function.c_str();
        PostFunction(n, &caller, 0, 0);
    }
}

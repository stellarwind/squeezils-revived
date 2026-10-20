// Wrappers that forward an id plus the object's embedded sub-object and +0x44 field to a helper which returns a
// class with a destructor by value (SQZ.EXE 0x4105f0, 0x4110b0, 0x40f6f0).
#include "compat41.h"

struct Item { int w[4]; ~Item(); };
// HelperA/B/C are Source members: ecx stays this across the call.

struct Source {
    char pad[4];
    char sub[0x40];                                  // 0x04
    int field44;                                     // 0x44
    Item GetA(int id);
    Item GetB(int id);
    Item GetC(int id);
    Item HelperA(int id, void *sub, int x);           // 0x40d070
    Item HelperB(int id, void *sub, int x);           // 0x40cb30
    Item HelperC(int id, void *sub, int x, int zero); // 0x40c8a0
};

Item Source::GetA(int id)
{
    return HelperA(id, sub, field44);
}
Item Source::GetB(int id)
{
    return HelperB(id, sub, field44);
}
Item Source::GetC(int id)
{
    return HelperC(id, sub, field44, 0);
}

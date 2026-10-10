// Soul-hierarchy destructors: a class with a virtual dtor + a base with a virtual dtor, no members.
// The scalar-deleting-destructor inlines "vtable store + call base dtor" inside an EH frame.
#include "compat41.h"

struct MMap {
    virtual ~MMap();
};
MMap::~MMap() {}   // out-of-line trivial: mov [ecx],vtbl ; ret

struct Soul : MMap {
    virtual ~Soul() {}
};

Soul *ForceSoul()
{
    return new Soul;
}

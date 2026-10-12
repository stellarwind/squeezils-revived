// Complete destructor of a class whose only content is one member with an out-of-line destructor
// (SQZ.EXE 0x414330 and 8 identical copies): EH frame, the member destroyed through the cleanup funclet.
#include "compat41.h"

struct Member { int pad; ~Member(); };
struct Holder {
    Member m;
    ~Holder();
};

Holder::~Holder() {}

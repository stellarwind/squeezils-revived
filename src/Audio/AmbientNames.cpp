// 0x449500: dynamic initialization of the ambient-sound name array (0x4c77b0).
#include "String/String.h"
#define BUFFEROBJECT_SIGNATURES
#include "Container/ContainerSet.h"

// Expand only the wrapper; Array<String>'s allocation stays out of line.
inline PrimitiveArray<String, 3>::PrimitiveArray() {}

typedef PrimitiveArray<String, 3> AmbientNames;
extern AmbientNames gAmbientNames; // storage at 0x4c77b0

// Guards/Init.cpp calls this entry and separately registers G449580.
void Thunk449500()
{
    gAmbientNames.AmbientNames::PrimitiveArray(); // explicit ctor call: no placement-new null test
}

extern unsigned char f_4c780c; // shared audio shutdown guards

// 0x449580: the explicit shutdown entry may also be invoked by atexit.
void G449580()
{
    if (!(f_4c780c & 8)) {
        f_4c780c |= 8;
        gAmbientNames.~AmbientNames();
    }
}

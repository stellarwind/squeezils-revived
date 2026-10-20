// CreatePropFunction::~CreatePropFunction (0x46f8c0).
#include "FunctionProp/Prop.h"

CreatePropFunction::~CreatePropFunction()
{
    delete name;
    delete position;
    delete direction;
}

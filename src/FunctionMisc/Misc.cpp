// No-operand script commands: Create is just `new`, Print the command name.
#include "FunctionMisc/Misc.h"

Function *GoToRegionFunction::Create(ArgList *list) { return new GoToRegionFunction; }
void GoToRegionFunction::Print() { DebugLog("%s", Name()); }
const char *GoToRegionFunction::Name() { return "GoToRegion"; }

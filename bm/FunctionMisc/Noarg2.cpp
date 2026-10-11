// ClearSenseTargets / ClearMessage / Abort: Create, Print and Name (SQZ.EXE 0x423cf0, 0x440590, 0x444620).
#include "FunctionMisc/Noarg2.h"

Function *ClearSenseTargetsFunction::Create(ArgList *list) { return new ClearSenseTargetsFunction; }
void ClearSenseTargetsFunction::Print() { DebugLog("%s", Name()); }
const char *ClearSenseTargetsFunction::Name() { return "ClearSenseTargets"; }

Function *ClearMessageFunction::Create(ArgList *list) { return new ClearMessageFunction; }
void ClearMessageFunction::Print() { DebugLog("%s", Name()); }
const char *ClearMessageFunction::Name() { return "ClearMessage"; }

Function *AbortFunction::Create(ArgList *list) { return new AbortFunction; }
void AbortFunction::Print() { DebugLog("%s", Name()); DebugLog("\n"); }
const char *AbortFunction::Name() { return "Abort"; }

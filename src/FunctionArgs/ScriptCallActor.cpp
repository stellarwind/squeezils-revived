// Script-call actor accessor, original 0x420c20: mov eax,[ecx+8]; ret.
#include "FunctionArgs/FunctionArgs.h"

Target *ScriptCall::Actor() { return target; }

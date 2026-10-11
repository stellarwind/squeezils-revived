// SetToFunction::Execute (0x444340).
#include "FunctionSet/Set.h"

int SetToFunction::Execute(ScriptCall *args, int extra)
{
    Variables *vars = &g_vars;
    int v = value->Get(extra);
    vars->Set(&name, &v, -1);
    return 1;
}

// AddToFunction::Execute (0x444590).
#include "FunctionSet/Set.h"

int AddToFunction::Execute(ScriptCall *args, int extra)
{
    Variables *has = &g_vars;
    if (!has->Contains(&name)) {
        Variables *set = &g_vars;
        int v = value->Get(extra);
        set->Set(&name, &v, -1);
    } else {
        int cur = g_vars.Get(&name);
        int v = cur + value->Get(extra);
        Variables *set = &g_vars;
        set->Set(&name, &v, -1);
    }
    return 1;
}

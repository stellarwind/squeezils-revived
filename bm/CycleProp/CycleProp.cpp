// CycleProp::SetCurrentThin (0x429370): forwards the arg to SetCurrent (0x428fd0).
#include "CycleProp/CycleProp.h"

void CycleProp::SetCurrentThin(int child)
{
    SetCurrent(child);
}

void CycleProp::SetStep(int newStep)
{
    step = newStep;
}

void CycleProp::SetMode(int newMode)
{
    mode = newMode;
}

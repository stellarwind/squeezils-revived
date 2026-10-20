// Panel::StartGoodieAnimation, SQZ.EXE 0x42e6f0.
#include "Panel/Panel.h"

void Panel::StartGoodieAnimation(int steps, float seconds)
{
    event = 1;
    status.SetRound(steps, seconds);
}

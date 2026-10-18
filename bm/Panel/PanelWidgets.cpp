// Panel widget loading and status-mode selection, SQZ.EXE 0x42d9f0 / 0x42dac0.
#include "Panel/Panel.h"

int Panel::Load()
{
    if (!LoadBitmaps())
        return 0;
    if (!status.Init())
        return 0;
    return quest.Load() != 0;
}

void Panel::ShowStatus(int foodGoal)
{
    mode = 0;
    status.SetFoodGoal(foodGoal);
    dirty = 1;
    ClearMessage();
    ClearNote();
}

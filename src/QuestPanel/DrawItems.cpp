// QuestPanel::DrawItems, SQZ.EXE 0x42f760.
#include "QuestPanel/QuestPanel.h"

int QuestPanel::DrawItems(int force, int place)
{
    if (force)
        items.Draw(place);
    return 0;
}

// QuestPanel: see QuestPanel.h.
#include "QuestPanel.h"
#include "Original.h"

BOOL QuestPanel::Load()
{
    if (!places.Init("PlacesCompleted", 525, 10, 1))
        return FALSE;
    places.Load();
    return TRUE;
}

int QuestPanel::DrawPlaces(int force)
{
    if (!force)
        return 0;
    int gold = 0, diamond = 0;
    GoodieCount("GoldPlacesCompleted", &gold);
    GoodieCount("DiamondPlacesCompleted", &diamond);
    places.Draw(gold, diamond);
    return 0;
}

BOOL QuestPanel::Draw(int force, int place)
{
    DrawPlaces(force);
    if (force)
        items.Draw(place);
    return TRUE;
}

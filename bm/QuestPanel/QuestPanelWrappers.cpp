// QuestPanel wrappers, SQZ.EXE 0x42f660-0x42f6d0.
#include "QuestPanel/QuestPanel.h"

int QuestPanel::Load()
{
    if (!places.Init("PlacesCompleted", 525, 10, 1))
        return 0;
    return LoadPlaces() != 0;
}

int QuestPanel::Draw(int force, int place)
{
    DrawPlaces(force);
    DrawItems(force, place);
    return 1;
}

int QuestPanel::LoadPlaces()
{
    places.Load();
    return 1;
}

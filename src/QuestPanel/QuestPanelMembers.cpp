// The QuestPanel member ctors (QPQuestItems / QPNutCounter), out of line so QuestPanel's ctor calls them.
#include "QuestPanel/QuestPanel.h"

QPQuestItems::QPQuestItems()
{
    combined = 0;
    left = 0;
    middle = 0;
    right = 0;
}

QPQuestItems::~QPQuestItems()
{
}

QPNutCounter::QPNutCounter()
{
    icon = 0;
    original = 0;
}

QPNutCounter::~QPNutCounter()
{
}

// StatusBar ctor (0x42ea70): build the widgets, then zero the count/state fields.
#include "StatusBarCtor/StatusBarCtor.h"

SB_NutCounter::SB_NutCounter() {}
SB_NutCounter::~SB_NutCounter() {}
SB_GoodieList::SB_GoodieList() {}
SB_GoodieList::~SB_GoodieList() {}
SB_MoodFace::SB_MoodFace() {}
SB_MoodFace::~SB_MoodFace() {}

StatusBarCtor::StatusBarCtor()
{
    foodGoal = 0;
    goodieStep = 0;
    goodieTick = 0;
    goodieInterval = 0;
    goodieSteps = 0;
    goodieFlip = 0;
    faceState = 0;
}

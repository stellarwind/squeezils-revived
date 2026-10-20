// QuestPanel: the part of the in-game panel that shows how far the player has got: the quest item pictures (the three key pieces
// and the combined stone) and the "places completed" counter. A sub object of the panel (at Panel + 0x394, 0x94 bytes).
// Reconstructed from SQZ.EXE 0x42f580 (constructor), 0x42f5f0 (destructor), 0x42f660 (Load), 0x42f690 (Draw) with its two helpers
// 0x42f6d0 and 0x42f760.
#pragma once
#include "QuestItems.h"
#include "NutCounter.h"

struct QuestPanel {
    QuestItems items;          // 0x00
    NutCounter places;         // 0x68 the PlacesCompleted counter (rockcnt.bmp)

    QuestPanel() {}            // @42f580 builds items, then places (their constructors do the work)
    ~QuestPanel() {}           // @42f5f0 destroys places, then items

    BOOL Load();                           // @42f660 set up and load the places counter (the quest item pictures load later, per place)
    BOOL Draw(int force, int place);       // @42f690 on a full redraw: the places counter and the quest items of place
    int  DrawPlaces(int force);            // @42f6d0 (a full redraw only) the counter shows the gold and diamond places completed
};
static_assert(sizeof(QuestPanel) == 0x94, "QuestPanel layout");

// QuestPanel: the part of the in-game panel that shows how far the player has got: the quest item pictures (the three key pieces
// and the combined stone) and the "places completed" counter. A sub object of the panel (at Panel + 0x394, 0x94 bytes).
// Reconstructed from SQZ.EXE 0x42f580 (constructor) and 0x42f5f0 (destructor); what it draws and when is still original code.
#pragma once
#include "QuestItems.h"
#include "NutCounter.h"

struct QuestPanel {
    QuestItems items;          // 0x00
    NutCounter places;         // 0x68 the PlacesCompleted counter (rockcnt.bmp)

    QuestPanel() {}            // @42f580 builds items, then places (their constructors do the work)
    ~QuestPanel() {}           // @42f5f0 destroys places, then items
};
static_assert(sizeof(QuestPanel) == 0x94, "QuestPanel layout");

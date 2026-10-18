// Panel::ShowQuest, SQZ.EXE 0x42da60.
// Seven spare declarations preserve VC++ 4.1 register allocation.
void ShowQuestPad1();
void ShowQuestPad2();
void ShowQuestPad3();
void ShowQuestPad4();
void ShowQuestPad5();
void ShowQuestPad6();
void ShowQuestPad7();
#include "Panel/Panel.h"

void Panel::ShowQuest(int key)
{
    if (questKey != key) {
        quest.items.FreeCombined();
        quest.items.Init(30, 5, key);
        dirty = 1;
    }
    questKey = key;
    mode = 1;
    ClearMessage();
    ClearNote();
}

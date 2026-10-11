// Menu::SetActions (0x406600): item n gets action n.
struct MenuItem { char pad[0x18]; int (*action)(); char pad2[0x24 - 0x1c]; };
struct MenuS {
    void *vtbl;
    int itemCount;
    char pad[0x18 - 8];
    MenuItem items[10];
    void SetActions();
};
int ActionStart();        // 0x406950
int ActionLoadGame();     // 0x406760
int ActionDialog81();     // 0x406730
int ActionHelp1();        // 0x406670
int ActionHelp2();        // 0x406690
int ActionDialog80();     // 0x406920

void MenuS::SetActions()
{
    for (int i = 0; i < itemCount; i++) {
        switch (i) {
        case 0: items[i].action = ActionStart; break;
        case 1: items[i].action = ActionLoadGame; break;
        case 2: items[i].action = ActionDialog81; break;
        case 3: items[i].action = ActionHelp1; break;
        case 4: items[i].action = ActionHelp2; break;
        case 5: items[i].action = ActionDialog80; break;
        }
    }
}

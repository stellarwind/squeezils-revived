// Menu leaves (0x406950, 0x406b80).
#include "Menu/Menu.h"

int Menu::ActionStart()
{
    return 2;
}

void Menu::SaveAnchor()
{
    anchorX = left;
    anchorY = top;
}

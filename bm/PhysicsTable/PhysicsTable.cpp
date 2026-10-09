// Input-control binding table initializer (0x43df10).
#include <string.h>
#include "PhysicsTable/PhysicsTable.h"

int PTTable::Init(int index)
{
    switch (index) {
    case 0:
        strcpy(entries[index].name, "Move Forward");
        entries[index].f_c = 0.0f;
        entries[index].f_10 = 1.0f;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 1:
        strcpy(entries[index].name, "Move Backward");
        entries[index].f_c = 0.0f;
        entries[index].f_10 = -1.0f;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 2:
        strcpy(entries[index].name, "Run");
        entries[index].i_1c = 2;
        ((PTTail *)&entries[index])->i_48 = 1;
        break;
    case 3:
        strcpy(entries[index].name, "Jump/Hop");
        entries[index].i_1c = 1;
        ((PTTail *)&entries[index])->i_48 = 1;
        break;
    case 4:
        strcpy(entries[index].name, "Spit");
        ((PTTail *)&entries[index])->i_48 = 1;
        break;
    case 5:
        strcpy(entries[index].name, "Rotate Left");
        entries[index].f_c = -1.0f;
        entries[index].f_10 = 0.0f;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 6:
        entries[index].f_c = 1.0f;
        entries[index].f_10 = 0.0f;
        strcpy(entries[index].name, "Rotate Right");
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 7:
        strcpy(entries[index].name, "Fly");
        ((PTTail *)&entries[index])->i_48 = 1;
        entries[index].i_20 = 3;
        ((PTTail *)&entries[index])->i_4c = 0;
        break;
    case 8:
        strcpy(entries[index].name, "Cannonball/Slide");
        ((PTTail *)&entries[index])->i_48 = 1;
        break;
    case 9:
        strcpy(entries[index].name, "Camera Mode");
        entries[index].i_1c = 3;
        ((PTTail *)&entries[index])->i_48 = 3;
        break;
    case 10:
        entries[index].f_c = -1.0f;
        entries[index].f_10 = 0.0f;
        entries[index].i_20 = 1;
        strcpy(entries[index].name, "Rotate Camera Left");
        ((PTTail *)&entries[index])->i_48 = 3;
        break;
    case 11:
        entries[index].f_c = 1.0f;
        entries[index].f_10 = 0.0f;
        entries[index].i_20 = 1;
        strcpy(entries[index].name, "Rotate Camera Right");
        ((PTTail *)&entries[index])->i_48 = 3;
        break;
    case 12:
        entries[index].f_c = 0.0f;
        entries[index].f_10 = 1.0f;
        entries[index].i_20 = 1;
        strcpy(entries[index].name, "Rotate Camera Up");
        ((PTTail *)&entries[index])->i_48 = 3;
        break;
    case 13:
        entries[index].f_c = 0.0f;
        entries[index].f_10 = -1.0f;
        entries[index].i_20 = 1;
        strcpy(entries[index].name, "Rotate Camera Down");
        ((PTTail *)&entries[index])->i_48 = 3;
        break;
    case 14:
        strcpy(entries[index].name, "Move Camera Base");
        entries[index].i_1c = 4;
        ((PTTail *)&entries[index])->i_48 = 3;
        break;
    case 15:
        entries[index].f_c = 0.0f;
        entries[index].f_10 = 0.0f;
        entries[index].f_14 = -0.05f;
        strcpy(entries[index].name, "Move Camera Out");
        ((PTTail *)&entries[index])->i_48 = 3;
        entries[index].i_20 = 4;
        entries[index].i_18 = 999;
        break;
    case 16:
        entries[index].f_c = 0.0f;
        entries[index].f_10 = 0.0f;
        entries[index].f_14 = 0.05f;
        strcpy(entries[index].name, "Move Camera POV");
        ((PTTail *)&entries[index])->i_48 = 3;
        entries[index].i_20 = 4;
        entries[index].i_18 = 998;
        break;
    case 17:
        entries[index].f_c = 0.1f;
        entries[index].f_10 = 0.0f;
        entries[index].f_14 = 0.0f;
        strcpy(entries[index].name, "Increase Leash");
        ((PTTail *)&entries[index])->i_48 = 3;
        entries[index].i_20 = 4;
        entries[index].i_18 = 997;
        break;
    case 18:
        entries[index].f_c = -0.1f;
        entries[index].f_10 = 0.0f;
        entries[index].f_14 = 0.0f;
        strcpy(entries[index].name, "Decrease Leash");
        ((PTTail *)&entries[index])->i_48 = 3;
        entries[index].i_18 = 996;
        entries[index].i_20 = 4;
        break;
    case 19:
        strcpy(entries[index].name, "AutoLeash Toggle");
        ((PTTail *)&entries[index])->i_48 = 3;
        entries[index].i_18 = 995;
        entries[index].i_20 = 4;
        break;
    case 20:
        strcpy(entries[index].name, "Return Toggle");
        ((PTTail *)&entries[index])->i_48 = 3;
        entries[index].i_18 = 994;
        entries[index].i_20 = 4;
        break;
    case 21:
        strcpy(entries[index].name, "Enter Tree/Portal");
        ((PTTail *)&entries[index])->i_48 = 2;
        break;
    case 22:
        strcpy(entries[index].name, "Restart Level");
        ((PTTail *)&entries[index])->i_48 = 1;
        ((PTTail *)&entries[index])->i_4c = 0;
        break;
    case 23:
        entries[index].i_18 = 27;
        strcpy(entries[index].name, "Menu Activation");
        ((PTTail *)&entries[index])->i_48 = 0;
        ((PTTail *)&entries[index])->i_4c = 0;
        break;
    }
    switch (index) {
    case 24:
        entries[index].i_18 = 13;
        strcpy(entries[index].name, "Jump to Place");
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 26:
        entries[index].i_18 = 89;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 27:
        entries[index].i_18 = 84;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 28:
        entries[index].i_18 = 88;
        entries[index].i_20 = 2;
        strcpy(entries[index].name, "Fart");
        ((PTTail *)&entries[index])->i_48 = 1;
        break;
    case 29:
        entries[index].i_18 = 83;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 30:
        entries[index].i_18 = 112;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 31:
        entries[index].i_18 = 113;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 32:
        entries[index].i_18 = 114;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 33:
        entries[index].i_18 = 115;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 34:
        entries[index].i_18 = 116;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 35:
        entries[index].i_18 = 117;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 36:
        entries[index].i_18 = 80;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 37:
        entries[index].i_18 = 119;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 38:
        entries[index].i_18 = 120;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 39:
        entries[index].i_18 = 122;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 40:
        entries[index].i_18 = 123;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 41:
        entries[index].i_18 = 85;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 42:
        entries[index].i_18 = 68;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 43:
        entries[index].i_18 = 69;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 44:
        entries[index].i_18 = 87;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 45:
        entries[index].i_18 = 56;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 46:
        entries[index].i_18 = 57;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 47:
        entries[index].i_18 = 52;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 48:
        entries[index].i_18 = 53;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 49:
        entries[index].i_18 = 54;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    case 50:
        entries[index].i_18 = 55;
        entries[index].i_20 = 2;
        ((PTTail *)&entries[index])->i_48 = 0;
        break;
    }
    return 0;
}

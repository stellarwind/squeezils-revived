// Menu::~Menu (0x405f90): frees the normal/lit bitmaps of the first itemCount items.
#include <windows.h>

struct Bitmap { ~Bitmap(); };   // 0x473890

struct Item {
    Bitmap *normal, *lit;
    int left, top, right, bottom;
    int action, key;
    int highlighted;
};

struct MenuK {
    virtual int SetSelected(int index);
    int itemCount;
    int left, top, itemHeight, spacing;
    Item items[10];
    ~MenuK();
};

MenuK::~MenuK()
{
    for (int i = 0; i < itemCount; i++) {
        if (items[i].normal && i < 10) {
            delete items[i].normal;
            delete items[i].lit;
        }
    }
}

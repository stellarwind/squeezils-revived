// Menu::OnMessage (SQZ.EXE 0x406420, ret 0x10): key, mouse and close handling.
struct ItemM {
    void *normal, *lit;
    int left, top, right, bottom;
    int (*action)();
    int key;
    int highlighted;
};
int ActionDialog80();                   // 0x406920
struct MenuM {
    virtual int SetSelected(int index);            // slot 0
    virtual int OnMouseMove(int x, int y);         // slot 1
    int itemCount;                                 // 0x04
    int left, top, itemHeight, spacing;
    ItemM items[10];                               // 0x18
    int HitTest(int index, int x, int y);          // 0x406a60
    int OnMessage(unsigned msg, unsigned wParam, unsigned lParam, unsigned *lastKey);
};

int MenuM::OnMessage(unsigned msg, unsigned wParam, unsigned lParam, unsigned *lastKey)
{
    int result = 0;
    switch (msg) {
    case 0x104:
    case 0x112:
        if ((wParam & 0xfff0) == 0xf060 || wParam == 0x73)
            return ActionDialog80();
        return 0;
    case 0x100: {
        unsigned key = wParam;
        *lastKey = key;
        for (int i = 0; i < itemCount; i++) {
            if (items[i].key == key) {
                result = items[i].action();
                break;
            }
        }
        int selected = -1;
        int step = 1;
        if (key == 0x26 || key == 0x28) {
            if (key == 0x26)
                step = -1;
            for (int j = 0; j < itemCount; j++) {
                if (items[j].highlighted) {
                    selected = j + step;
                    if (selected >= itemCount)
                        selected = 0;
                    else if (selected < 0)
                        selected = itemCount - 1;
                }
            }
            if (selected == -1)
                selected = 0;
            return SetSelected(selected);
        }
        if (key == 0xd) {
            for (int k = 0; k < itemCount; k++)
                if (items[k].highlighted)
                    result = items[k].action();
        }
        return result;
    }
    case 0x200:
        return OnMouseMove(lParam & 0xffff, lParam >> 16 & 0xffff);
    case 0x201: {
        int x = lParam & 0xffff;
        int y = lParam >> 16 & 0xffff;
        for (int m = 0; m < itemCount; m++) {
            if (HitTest(m, x, y) && items[m].highlighted && items[m].action)
                result = items[m].action();
        }
    }
    }
    return result;
}

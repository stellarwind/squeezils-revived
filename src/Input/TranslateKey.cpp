// 0x43d7a0: normalize a keyboard message and find an eligible command binding.
// Best match score: 73; remaining compiler differences tracked in META/STUCK.md.
struct KeyMessageTable {
    virtual void slot0();
    virtual int Key(int command);
    char pad04[0x44];
    int firstCategory; // +0x48, subsequent categories are 0x44 bytes apart
    int Type(int command); // 0x43dbe0
    int Translate(int message, int key, int unused, int mode, int special);
};

int KeyMessageTable::Translate(int message, int wparam, int, int mode, int special)
{
    switch (message) {
    case 0x100: case 0x101: case 0x104: case 0x105: break;
    default: return -1;
    }
    int key = wparam;
    switch (key) {
    case 0x60: key = '0'; break;
    case 0x61: key = '1'; break;
    case 0x62: key = '2'; break;
    case 0x63: key = '3'; break;
    case 0x64: key = '4'; break;
    case 0x65: key = '5'; break;
    case 0x66: key = '6'; break;
    case 0x67: key = '7'; break;
    case 0x68: key = '8'; break;
    case 0x69: key = '9'; break;
    }
    if (Key(9) == key)
        return 9;
    int *category = &firstCategory;
    int i;
    for (i = 0; i < 51; i++, category += 17) {
        if (Key(i) != key)
            continue;
        if (special) {
            if (Type(i) == 1)
                return i;
            if (special)
                continue;
        }
        if (Type(i) != 1) {
            if (mode) {
                int value = *category;
                if (value == 1 || value == 0 || value == 3)
                    return i;
                if (mode)
                    continue;
            }
            int value = *category;
            if (value == 2 || value == 0 || value == 3)
                return i;
        }
    }
    return i;
}

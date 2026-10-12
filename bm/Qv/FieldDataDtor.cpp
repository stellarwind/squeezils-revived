// QvFieldData destructor (0x480630): free every field entry and every enum type, then the two lists.
struct QvName {
    void *p;
    QvName();                      // 0x482560
    ~QvName();                     // 0x47ec60
};
struct QvFieldEntry { QvName name; int offset; };
struct QvEnumType { QvName name; int count, cap; int *values; QvName *names; ~QvEnumType(); };   // 0x4805a0
struct QvPList {
    void **items; int count; int cap;
    QvPList();                     // 0x482a60
    ~QvPList();                    // 0x482a80
};
struct QvFieldData {
    QvPList fields;                // +0
    QvPList enums;                 // +0xc
    ~QvFieldData();
};
QvFieldData::~QvFieldData()
{
    int i = 0;
    for (;;) {
        if (i >= fields.count)
            break;
        QvFieldEntry *e = (QvFieldEntry *)fields.items[i];
        if (e)
            delete e;
        i++;
    }
    i = 0;
    for (;;) {
        if (i >= enums.count)
            break;
        QvEnumType *t = (QvEnumType *)enums.items[i];
        if (t)
            delete t;
        i++;
    }
}

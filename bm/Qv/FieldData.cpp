// QvFieldData::addField (0x480720): record a field's interned name and its offset inside the node.
struct QvName {                                       // interned-name handle, 4 bytes
    void *p;
    QvName();                                         // 0x482560
    QvName(void *x) { p = x; }
    ~QvName();                                        // 0x47ec60
};
void *QvIntern(const char *s);                       // 0x482390
struct QvFieldEntry {
    QvName name;
    int offset;
};
struct QvEntryList {
    QvFieldEntry **items; int count; int cap;
    void Grow(int n);                                 // 0x482b10
};
struct QvFieldData {
    QvEntryList fields;
    void addField(void *node, const char *name, void *field);
};
void QvFieldData::addField(void *node, const char *name, void *field)
{
    QvFieldEntry *e = new QvFieldEntry;
    e->name = QvName(QvIntern(name));
    e->offset = (int)field - (int)node;
    if (fields.cap < fields.count + 1)
        fields.Grow(fields.count + 1);
    fields.items[fields.count] = e;
    fields.count++;
}

// entry list accessors (0x480810 / 0x480820)
struct QvEntryList2 {
    QvFieldEntry **items; int count; int cap;
    QvFieldEntry *Get(int i);
    int OffsetOf(int base, int i);
};
QvFieldEntry *QvEntryList2::Get(int i) { return items[i]; }
int QvEntryList2::OffsetOf(int base, int i) { return items[i]->offset + base; }

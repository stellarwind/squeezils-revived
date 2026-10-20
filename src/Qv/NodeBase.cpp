// QvNode constructor (0x47e660) and destructor (0x47e710): the node owns a boxed interned name (+0x10), refcount at +0x14.
struct QvEntry { char *str; };
QvEntry *QvIntern(const char *s);                     // 0x482390
void QvRemoveName(void *node, char *name);            // 0x47ea90
struct QvNameBox {
    QvEntry *entry;
    QvNameBox(QvEntry *e) { entry = e; }
};
extern const char kUndefinedName[];                   // 0x4b9488
class QvNodeB {
public:
    QvNodeB();
    virtual ~QvNodeB();
    int pad4, pad8, padc;
    QvNameBox *name;                                  // +0x10
    int refCount;                                     // +0x14
};
QvNodeB::QvNodeB()
{
    refCount = 0;
    name = new QvNameBox(QvIntern(kUndefinedName));
}
QvNodeB::~QvNodeB()
{
    char *s = name->entry->str;
    if (*s != 0)
        QvRemoveName(this, s);
    if (name)
        delete name;
}

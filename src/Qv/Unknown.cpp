// QvLib generic ("unknown class") node: constructor 0x482c70 and SetClass 0x482df0 (the class name is strdup'ed into +0x18).
struct QvPListU { void **items; int count; int cap; QvPListU(); ~QvPListU(); };    // 0x482a60 / 0x482a80
struct QvFieldDataU { QvPListU fields; QvPListU enums; ~QvFieldDataU(); };         // 0x480630
char *QvStrdup(const char *s);                                                     // 0x49bd70
class QvNodeU { public: QvNodeU(); virtual ~QvNodeU(); int p4, p8, isBuiltIn, name, rc; };
class QvGroupBaseU : public QvNodeU { public: QvGroupBaseU(); virtual ~QvGroupBaseU(); };     // 0x481e00
class QvUnknownU : public QvGroupBaseU {
public:
    QvUnknownU();                                       // 0x482c70
    virtual ~QvUnknownU();
    char *className;                                    // +0x18
    QvFieldDataU *instFields;                           // +0x1c
    void SetClass(char *name);                          // 0x482df0
    static QvFieldDataU *fieldData;                     // 0x4c8874
    static int firstInstance;                           // 0x4c46e0
};
QvFieldDataU *QvUnknownU::fieldData;
int QvUnknownU::firstInstance = 1;
QvUnknownU::QvUnknownU()
{
    if (fieldData == 0)
        fieldData = new QvFieldDataU;
    else
        firstInstance = 0;
    isBuiltIn = 0;
    className = 0;
    QvFieldDataU *old;
    if (firstInstance == 1 && (old = fieldData) != 0)
        delete old;
    fieldData = instFields = new QvFieldDataU;
}
void QvUnknownU::SetClass(char *name) { className = QvStrdup(name); }

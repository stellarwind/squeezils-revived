// QvLib enum type record (0x14 bytes): ctor 0x4804d0, dtor 0x4805a0.
struct QvName {
    void *p;
    QvName();                      // 0x482560
    ~QvName();                     // 0x47ec60
};
extern int g_enumCap;              // 0x4c4538: initial capacity of the value tables
struct QvEnumType {
    QvName name;                   // +0
    int count;                     // +4
    int cap;                       // +8
    int *values;                   // +0xc
    QvName *names;                 // +0x10
    QvEnumType(QvName *n);
    ~QvEnumType();
};
QvEnumType::QvEnumType(QvName *n)
{
    name = *n;
    count = 0;
    cap = g_enumCap;
    values = new int[cap];
    names = new QvName[cap];
}
QvEnumType::~QvEnumType()
{
    delete values;
    delete [] names;
}

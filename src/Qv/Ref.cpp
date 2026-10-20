// QvNode reference counting (0x47e750 / 0x47e760) and the name accessor (0x47e780). Refcount at +0x14, name at +0x10.
class QvRc {
public:
    virtual ~QvRc();
    int pad4, pad8, padc;
    void *name;
    int refCount;
    void Ref();
    void Unref();
    void *GetName();
};
void QvRc::Ref() { refCount++; }
void QvRc::Unref()
{
    if (--refCount == 0 && this)
        delete this;
}
void *QvRc::GetName() { return name; }

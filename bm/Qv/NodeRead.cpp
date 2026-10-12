// QvLib QvNode::read (0x481fd0): the optional "fields [...]" extension header, then the field values and the children.
#include <string.h>
struct QvEntryN { char *str; };
struct QvNameN { QvEntryN *e; QvNameN(); ~QvNameN(); };            // 0x482560 / 0x47ec60
struct QvInputN {
    int ReadName(QvNameN *n, int ident);                          // 0x481300
    void SetFileName(char *s);                                    // 0x481650
};
struct QvFieldDataN {
    int HasExtension(QvInputN *in, void *node);                   // 0x480c00
    int ReadFields(QvInputN *in, void *node, int flag);           // 0x480df0
};
void QvError(QvInputN *in, const char *fmt, ...);                 // 0x482bd0
extern char kExtName[];                                           // 0x4c42a4
extern char kNoExt[];                                             // 0x4c4280
class QvNodeN {
public:
    virtual ~QvNodeN();
    virtual QvFieldDataN *GetFieldData();                         // slot 1
    virtual void v2(); virtual void v3(); virtual void v4();
    virtual int ReadChildren(QvInputN *in);                       // slot 5
    int p4, p8, isBuiltIn;                                        // +0xc
    int Read(QvInputN *in);
};
int QvNodeN::Read(QvInputN *in)
{
    QvNameN n;
    int result;
    QvFieldDataN *fd = GetFieldData();
    if (isBuiltIn == 0) {
        if (in->ReadName(&n, 1)) {
            char *s = n.e->str;
            int eq = (*s == kExtName[0] && strcmp(s, kExtName) == 0);
            if (eq) {
                if (!fd->HasExtension(in, this)) {
                    QvError(in, kNoExt);
                    return 0;
                }
            } else
                in->SetFileName(n.e->str);
        }
    }
    if (fd->ReadFields(in, this, 0)) {
        if (ReadChildren(in)) {
            result = 1;
            goto done;
        }
    }
    result = 0;
done:
    return result;
}

// QvNode::readFields (0x47ec70): like Read without the built-in check and without the children.
class QvNodeN2 {
public:
    virtual ~QvNodeN2();
    virtual QvFieldDataN *GetFieldData();                         // slot 1
    int ReadFieldsOnly(QvInputN *in);
};
int QvNodeN2::ReadFieldsOnly(QvInputN *in)
{
    QvNameN n;
    QvFieldDataN *fd = GetFieldData();
    if (in->ReadName(&n, 1)) {
        char *s = n.e->str;
        int eq = (*s == kExtName[0] && strcmp(s, kExtName) == 0);
        if (eq) {
            if (!fd->HasExtension(in, this)) {
                QvError(in, kNoExt);
                return 0;
            }
        } else
            in->SetFileName(n.e->str);
    }
    if (fd->ReadFields(in, this, 1))
        return 1;
    return 0;
}

// QvLib intermediate node class (vtbl 0x4a11f8): owns a child list at +8; complete destructor 0x481f30.
#include "Qv/Node.h"
struct QvChildList { ~QvChildList(); };      // 0x489900
class QvInter : public QvNode {
public:
    virtual ~QvInter();
};
QvInter::~QvInter()
{
    QvChildList *c = (QvChildList *)unk8;
    if (c)
        delete c;
}

// QvLib group-like node class (vtbl 0x4a1218), complete destructor 0x482e10: frees every child, the child list and the name.
#include <stdlib.h>
struct QvKids {
    int a, count;
    ~QvKids();                                   // 0x480630
    void *Get(QvNode *owner, int i);             // 0x480820
};
class QvGroupN : public QvInter {
public:
    char *nameStr;                               // +0x18
    QvKids *kids;                                // +0x1c
    virtual ~QvGroupN();
};
QvGroupN::~QvGroupN()
{
    int i;
    for (i = 0; i < kids->count; i++)
        delete (char *)kids->Get(this, i);
    if (kids)
        delete kids;
    if (nameStr)
        free(nameStr);
}

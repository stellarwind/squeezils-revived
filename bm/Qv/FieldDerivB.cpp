// QvLib string-valued field classes (derived from QvFieldB; 0x48a060 ...): the destructor frees the string at +0x14 with free() when
// the global "malloc'ed strings" flag (0x4c8904) is set, with delete otherwise.
#include <stdlib.h>
#include "Qv/Field.h"
extern int g_mallocStrings;
class QvFieldD : public QvFieldB {
public:
    char *str;                      // +0x14
    virtual ~QvFieldD();
};
QvFieldD::~QvFieldD()
{
    if (str) {
        if (g_mallocStrings != 0)
            free(str);
        else
            delete str;
    }
}

// constructor (0x489fc0 ...): base ctor, vtable, malloc flag, empty string, then the out-of-line initialiser (0x48a0e0) with 1
class QvFieldE : public QvFieldB {
public:
    char *str;
    QvFieldE();
    virtual ~QvFieldE();
    void Init(int v);               // 0x48a0e0
};
QvFieldE::QvFieldE()
{
    g_mallocStrings = 1;
    str = 0;
    Init(1);
}

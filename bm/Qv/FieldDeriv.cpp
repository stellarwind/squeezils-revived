// QvLib field classes derived from QvFieldA with no data of their own (e.g. 0x489ba0): the constructor only calls the base constructor
// (out of line) and stores its vtable. One source for all of them; the base-ctor call and the vtable address are masked.
#include "Qv/Field.h"
class QvFieldC : public QvFieldA {
public:
    QvFieldC();
    virtual ~QvFieldC();
};
QvFieldC::QvFieldC() {}

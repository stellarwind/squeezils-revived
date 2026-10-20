// QvLib node class (vtbl 0x4a1438) constructor 0x4864b0.
// Generator output repaired: SFMatrix has sixteen floats, initialized below.
#include "Qv/Node.h"
// SIZE 0x4c
struct QvF48acd0 : QvField {
    QvF48acd0();
    ~QvF48acd0();
    float value[4][4]; // 0x0c
};
// SIZE 0x64
class QvNodeC4864b0 : public QvNode {
public:
    QvNodeC4864b0();
    virtual ~QvNodeC4864b0();
    QvF48acd0 m0;   // +0x18
    static QvFieldData *fieldData;
    static int firstInstance;
};
QvFieldData *QvNodeC4864b0::fieldData;
int QvNodeC4864b0::firstInstance = 1;

QvNodeC4864b0::QvNodeC4864b0()
{
    if (fieldData == 0)
        fieldData = new QvFieldData;
    else
        firstInstance = 0;
    isBuiltIn = 1;
    if (firstInstance)
        fieldData->addField(this, "matrix", &m0);
    m0.SetOwner(this);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m0.value[i][j] = i == j ? 1.0f : 0.0f;
}

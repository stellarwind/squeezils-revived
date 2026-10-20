// QvLib group node base class constructor (0x481e00): owns a child list at +8.
#include "Qv/Node.h"
struct QvChildListG { char b[12]; QvChildListG(); ~QvChildListG(); };       // 0x4898b0
class QvGroupX : public QvNode {
public:
    QvGroupX();
    virtual ~QvGroupX();
    static QvFieldData *fieldData;
    static int firstInstance;
};
QvFieldData *QvGroupX::fieldData;
int QvGroupX::firstInstance = 1;
QvGroupX::QvGroupX()
{
    unk8 = (int)new QvChildListG;
    if (fieldData == 0)
        fieldData = new QvFieldData;
    else
        firstInstance = 0;
    isBuiltIn = 1;
}

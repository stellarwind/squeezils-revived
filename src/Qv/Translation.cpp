// QvTranslation constructor (0x483460) as the QvLib macros expand.
#include "Qv/Node.h"
class QvTranslation : public QvNode {
public:
    QvTranslation();
    virtual ~QvTranslation();
    QvSFVec3f translation;
    static QvFieldData *fieldData;
    static int firstInstance;
};
QvFieldData *QvTranslation::fieldData;
int QvTranslation::firstInstance = 1;

QvTranslation::QvTranslation()
{
    if (fieldData == 0)
        fieldData = new QvFieldData;
    else
        firstInstance = 0;
    isBuiltIn = 1;
    if (firstInstance)
        fieldData->addField(this, "translation", &translation);
    translation.SetOwner(this);
    translation.z = 0; translation.y = 0; translation.x = 0;
}
void F(QvTranslation *p) { new QvTranslation; }

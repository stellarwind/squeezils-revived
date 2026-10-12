// QvLib field factory (0x488bf0): the field class for a type name, built with new + constructor, tried in order.
#include <string.h>
struct QvEntryF2 { char *str; };
struct QvFieldF2 { int dummy; };
struct QvF48aed0 : QvFieldF2 { QvF48aed0(); char b[20]; };   // MFColor
extern char kMFColor[];
struct QvF48b130 : QvFieldF2 { QvF48b130(); char b[20]; };   // MFFloat
extern char kMFFloat[];
struct QvF48b360 : QvFieldF2 { QvF48b360(); char b[20]; };   // MFLong
extern char kMFLong[];
struct QvF48b7f0 : QvFieldF2 { QvF48b7f0(); char b[20]; };   // MFString
extern char kMFString[];
struct QvF489fc0 : QvFieldF2 { QvF489fc0(); char b[20]; };   // MFVec2f
extern char kMFVec2f[];
struct QvF48aa70 : QvFieldF2 { QvF48aa70(); char b[20]; };   // MFVec3f
extern char kMFVec3f[];
struct QvF48b590 : QvFieldF2 { QvF48b590(); char b[24]; };   // SFBitMask
extern char kSFBitMask[];
struct QvF48a6e0 : QvFieldF2 { QvF48a6e0(); char b[12]; };   // SFBool
extern char kSFBool[];
struct QvF48a950 : QvFieldF2 { QvF48a950(); char b[20]; };   // SFColor
extern char kSFColor[];
struct QvF489cc0 : QvFieldF2 { QvF489cc0(); char b[24]; };   // SFEnum
extern char kSFEnum[];
struct QvF48a320 : QvFieldF2 { QvF48a320(); char b[12]; };   // SFFloat
extern char kSFFloat[];
struct QvF48a400 : QvFieldF2 { QvF48a400(); char b[20]; };   // SFImage
extern char kSFImage[];
struct QvF48a600 : QvFieldF2 { QvF48a600(); char b[12]; };   // SFLong
extern char kSFLong[];
struct QvF48acd0 : QvFieldF2 { QvF48acd0(); char b[72]; };   // SFMatrix
extern char kSFMatrix[];
struct QvF489e90 : QvFieldF2 { QvF489e90(); char b[24]; };   // SFRotation
extern char kSFRotation[];
struct QvF489a80 : QvFieldF2 { QvF489a80(); char b[48]; };   // SFString
extern char kSFString[];
struct QvF48a210 : QvFieldF2 { QvF48a210(); char b[16]; };   // SFVec2f
extern char kSFVec2f[];
struct QvF489ba0 : QvFieldF2 { QvF489ba0(); char b[20]; };   // SFVec3f
extern char kSFVec3f[];
QvFieldF2 *QvFieldFactory(QvEntryF2 **name)
{
    QvFieldF2 *result = 0;
    int eq;
    eq = (*(*name)->str == kMFColor[0] && strcmp((*name)->str, kMFColor) == 0);
    if (eq) {
        result = new QvF48aed0;
        goto done;
    }
    eq = (*(*name)->str == kMFFloat[0] && strcmp((*name)->str, kMFFloat) == 0);
    if (eq) {
        result = new QvF48b130;
        goto done;
    }
    eq = (*(*name)->str == kMFLong[0] && strcmp((*name)->str, kMFLong) == 0);
    if (eq) {
        result = new QvF48b360;
        goto done;
    }
    eq = (*(*name)->str == kMFString[0] && strcmp((*name)->str, kMFString) == 0);
    if (eq) {
        result = new QvF48b7f0;
        goto done;
    }
    eq = (*(*name)->str == kMFVec2f[0] && strcmp((*name)->str, kMFVec2f) == 0);
    if (eq) {
        result = new QvF489fc0;
        goto done;
    }
    eq = (*(*name)->str == kMFVec3f[0] && strcmp((*name)->str, kMFVec3f) == 0);
    if (eq) {
        result = new QvF48aa70;
        goto done;
    }
    eq = (*(*name)->str == kSFBitMask[0] && strcmp((*name)->str, kSFBitMask) == 0);
    if (eq) {
        result = new QvF48b590;
        goto done;
    }
    eq = (*(*name)->str == kSFBool[0] && strcmp((*name)->str, kSFBool) == 0);
    if (eq) {
        result = new QvF48a6e0;
        goto done;
    }
    eq = (*(*name)->str == kSFColor[0] && strcmp((*name)->str, kSFColor) == 0);
    if (eq) {
        result = new QvF48a950;
        goto done;
    }
    eq = (*(*name)->str == kSFEnum[0] && strcmp((*name)->str, kSFEnum) == 0);
    if (eq) {
        result = new QvF489cc0;
        goto done;
    }
    eq = (*(*name)->str == kSFFloat[0] && strcmp((*name)->str, kSFFloat) == 0);
    if (eq) {
        result = new QvF48a320;
        goto done;
    }
    eq = (*(*name)->str == kSFImage[0] && strcmp((*name)->str, kSFImage) == 0);
    if (eq) {
        result = new QvF48a400;
        goto done;
    }
    eq = (*(*name)->str == kSFLong[0] && strcmp((*name)->str, kSFLong) == 0);
    if (eq) {
        result = new QvF48a600;
        goto done;
    }
    eq = (*(*name)->str == kSFMatrix[0] && strcmp((*name)->str, kSFMatrix) == 0);
    if (eq) {
        result = new QvF48acd0;
        goto done;
    }
    eq = (*(*name)->str == kSFRotation[0] && strcmp((*name)->str, kSFRotation) == 0);
    if (eq) {
        result = new QvF489e90;
        goto done;
    }
    eq = (*(*name)->str == kSFString[0] && strcmp((*name)->str, kSFString) == 0);
    if (eq) {
        result = new QvF489a80;
        goto done;
    }
    eq = (*(*name)->str == kSFVec2f[0] && strcmp((*name)->str, kSFVec2f) == 0);
    if (eq) {
        result = new QvF48a210;
        goto done;
    }
    eq = (*(*name)->str == kSFVec3f[0] && strcmp((*name)->str, kSFVec3f) == 0);
    if (eq) {
        result = new QvF489ba0;
        goto done;
    }
done:
    return result;
}

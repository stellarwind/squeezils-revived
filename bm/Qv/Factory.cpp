// QvLib node factory (0x47f1d0): the node class for a name, built with new + constructor, tried in alphabetical order.
#include <string.h>
struct QvEntryF { char *str; };
struct QvNodeF { int dummy; };
inline int Eq(const char *a, const char *b) { return *a == *b && strcmp(a, b) == 0; }
struct QvC488820 : QvNodeF { QvC488820(); char b[112]; };   // AsciiText ctor 0x488820
extern char kAsciiText[];
struct QvC488510 : QvNodeF { QvC488510(); char b[80]; };   // Cone ctor 0x488510
extern char kCone[];
struct QvC488330 : QvNodeF { QvC488330(); char b[44]; };   // Coordinate3 ctor 0x488330
extern char kCoordinate3[];
struct QvC4880c0 : QvNodeF { QvC4880c0(); char b[68]; };   // Cube ctor 0x4880c0
extern char kCube[];
struct QvC487d90 : QvNodeF { QvC487d90(); char b[80]; };   // Cylinder ctor 0x487d90
extern char kCylinder[];
struct QvC487ab0 : QvNodeF { QvC487ab0(); char b[100]; };   // DirectionalLight ctor 0x487ab0
extern char kDirectionalLight[];
struct QvC487720 : QvNodeF { QvC487720(); char b[92]; };   // FontStyle ctor 0x487720
extern char kFontStyle[];
struct QvC481e00 : QvNodeF { QvC481e00(); char b[20]; };   // Group ctor 0x481e00
extern char kGroup[];
struct QvC487440 : QvNodeF { QvC487440(); char b[116]; };   // IndexedFaceSet ctor 0x487440
extern char kIndexedFaceSet[];
struct QvC487160 : QvNodeF { QvC487160(); char b[116]; };   // IndexedLineSet ctor 0x487160
extern char kIndexedLineSet[];
struct QvC486f90 : QvNodeF { QvC486f90(); char b[72]; };   // Info ctor 0x486f90
extern char kInfo[];
struct QvC486d70 : QvNodeF { QvC486d70(); char b[68]; };   // LOD ctor 0x486d70
extern char kLOD[];
struct QvC486990 : QvNodeF { QvC486990(); char b[164]; };   // Material ctor 0x486990
extern char kMaterial[];
struct QvC4866a0 : QvNodeF { QvC4866a0(); char b[48]; };   // MaterialBinding ctor 0x4866a0
extern char kMaterialBinding[];
struct QvC4864b0 : QvNodeF { QvC4864b0(); char b[96]; };   // MatrixTransform ctor 0x4864b0
extern char kMatrixTransform[];
struct QvC4862d0 : QvNodeF { QvC4862d0(); char b[44]; };   // Normal ctor 0x4862d0
extern char kNormal[];
struct QvC485fe0 : QvNodeF { QvC485fe0(); char b[48]; };   // NormalBinding ctor 0x485fe0
extern char kNormalBinding[];
struct QvC485d00 : QvNodeF { QvC485d00(); char b[104]; };   // OrthographicCamera ctor 0x485d00
extern char kOrthographicCamera[];
struct QvC485a20 : QvNodeF { QvC485a20(); char b[104]; };   // PerspectiveCamera ctor 0x485a20
extern char kPerspectiveCamera[];
struct QvC485740 : QvNodeF { QvC485740(); char b[100]; };   // PointLight ctor 0x485740
extern char kPointLight[];
struct QvC485520 : QvNodeF { QvC485520(); char b[52]; };   // PointSet ctor 0x485520
extern char kPointSet[];
struct QvC485350 : QvNodeF { QvC485350(); char b[48]; };   // Rotation ctor 0x485350
extern char kRotation[];
struct QvC485180 : QvNodeF { QvC485180(); char b[44]; };   // Scale ctor 0x485180
extern char kScale[];
struct QvC484f30 : QvNodeF { QvC484f30(); char b[48]; };   // Separator ctor 0x484f30
extern char kSeparator[];
struct QvC484b00 : QvNodeF { QvC484b00(); char b[120]; };   // ShapeHints ctor 0x484b00
extern char kShapeHints[];
struct QvC484940 : QvNodeF { QvC484940(); char b[36]; };   // Sphere ctor 0x484940
extern char kSphere[];
struct QvC484530 : QvNodeF { QvC484530(); char b[156]; };   // SpotLight ctor 0x484530
extern char kSpotLight[];
struct QvC484370 : QvNodeF { QvC484370(); char b[36]; };   // Switch ctor 0x484370
extern char kSwitch[];
struct QvC483f90 : QvNodeF { QvC483f90(); char b[152]; };   // Texture2 ctor 0x483f90
extern char kTexture2[];
struct QvC483cc0 : QvNodeF { QvC483cc0(); char b[96]; };   // Texture2Transform ctor 0x483cc0
extern char kTexture2Transform[];
struct QvC483af0 : QvNodeF { QvC483af0(); char b[44]; };   // TextureCoordinate2 ctor 0x483af0
extern char kTextureCoordinate2[];
struct QvC483790 : QvNodeF { QvC483790(); char b[148]; };   // Transform ctor 0x483790
extern char kTransform[];
struct QvC483630 : QvNodeF { QvC483630(); char b[20]; };   // TransformSeparator ctor 0x483630
extern char kTransformSeparator[];
struct QvC483460 : QvNodeF { QvC483460(); char b[44]; };   // Translation ctor 0x483460
extern char kTranslation[];
struct QvC483150 : QvNodeF { QvC483150(); char b[152]; };   // WWWAnchor ctor 0x483150
extern char kWWWAnchor[];
struct QvC482ed0 : QvNodeF { QvC482ed0(); char b[120]; };   // WWWInline ctor 0x482ed0
extern char kWWWInline[];
QvNodeF *QvFactory(QvEntryF **name)
{
    QvNodeF *result = 0;
    int eq;
    eq = (*(*name)->str == kAsciiText[0] && strcmp((*name)->str, kAsciiText) == 0);
    if (eq) {
        result = new QvC488820;
        goto done;
    }
    eq = (*(*name)->str == kCone[0] && strcmp((*name)->str, kCone) == 0);
    if (eq) {
        result = new QvC488510;
        goto done;
    }
    eq = (*(*name)->str == kCoordinate3[0] && strcmp((*name)->str, kCoordinate3) == 0);
    if (eq) {
        result = new QvC488330;
        goto done;
    }
    eq = (*(*name)->str == kCube[0] && strcmp((*name)->str, kCube) == 0);
    if (eq) {
        result = new QvC4880c0;
        goto done;
    }
    eq = (*(*name)->str == kCylinder[0] && strcmp((*name)->str, kCylinder) == 0);
    if (eq) {
        result = new QvC487d90;
        goto done;
    }
    eq = (*(*name)->str == kDirectionalLight[0] && strcmp((*name)->str, kDirectionalLight) == 0);
    if (eq) {
        result = new QvC487ab0;
        goto done;
    }
    eq = (*(*name)->str == kFontStyle[0] && strcmp((*name)->str, kFontStyle) == 0);
    if (eq) {
        result = new QvC487720;
        goto done;
    }
    eq = (*(*name)->str == kGroup[0] && strcmp((*name)->str, kGroup) == 0);
    if (eq) {
        result = new QvC481e00;
        goto done;
    }
    eq = (*(*name)->str == kIndexedFaceSet[0] && strcmp((*name)->str, kIndexedFaceSet) == 0);
    if (eq) {
        result = new QvC487440;
        goto done;
    }
    eq = (*(*name)->str == kIndexedLineSet[0] && strcmp((*name)->str, kIndexedLineSet) == 0);
    if (eq) {
        result = new QvC487160;
        goto done;
    }
    eq = (*(*name)->str == kInfo[0] && strcmp((*name)->str, kInfo) == 0);
    if (eq) {
        result = new QvC486f90;
        goto done;
    }
    eq = (*(*name)->str == kLOD[0] && strcmp((*name)->str, kLOD) == 0);
    if (eq) {
        result = new QvC486d70;
        goto done;
    }
    eq = (*(*name)->str == kMaterial[0] && strcmp((*name)->str, kMaterial) == 0);
    if (eq) {
        result = new QvC486990;
        goto done;
    }
    eq = (*(*name)->str == kMaterialBinding[0] && strcmp((*name)->str, kMaterialBinding) == 0);
    if (eq) {
        result = new QvC4866a0;
        goto done;
    }
    eq = (*(*name)->str == kMatrixTransform[0] && strcmp((*name)->str, kMatrixTransform) == 0);
    if (eq) {
        result = new QvC4864b0;
        goto done;
    }
    eq = (*(*name)->str == kNormal[0] && strcmp((*name)->str, kNormal) == 0);
    if (eq) {
        result = new QvC4862d0;
        goto done;
    }
    eq = (*(*name)->str == kNormalBinding[0] && strcmp((*name)->str, kNormalBinding) == 0);
    if (eq) {
        result = new QvC485fe0;
        goto done;
    }
    eq = (*(*name)->str == kOrthographicCamera[0] && strcmp((*name)->str, kOrthographicCamera) == 0);
    if (eq) {
        result = new QvC485d00;
        goto done;
    }
    eq = (*(*name)->str == kPerspectiveCamera[0] && strcmp((*name)->str, kPerspectiveCamera) == 0);
    if (eq) {
        result = new QvC485a20;
        goto done;
    }
    eq = (*(*name)->str == kPointLight[0] && strcmp((*name)->str, kPointLight) == 0);
    if (eq) {
        result = new QvC485740;
        goto done;
    }
    eq = (*(*name)->str == kPointSet[0] && strcmp((*name)->str, kPointSet) == 0);
    if (eq) {
        result = new QvC485520;
        goto done;
    }
    eq = (*(*name)->str == kRotation[0] && strcmp((*name)->str, kRotation) == 0);
    if (eq) {
        result = new QvC485350;
        goto done;
    }
    eq = (*(*name)->str == kScale[0] && strcmp((*name)->str, kScale) == 0);
    if (eq) {
        result = new QvC485180;
        goto done;
    }
    eq = (*(*name)->str == kSeparator[0] && strcmp((*name)->str, kSeparator) == 0);
    if (eq) {
        result = new QvC484f30;
        goto done;
    }
    eq = (*(*name)->str == kShapeHints[0] && strcmp((*name)->str, kShapeHints) == 0);
    if (eq) {
        result = new QvC484b00;
        goto done;
    }
    eq = (*(*name)->str == kSphere[0] && strcmp((*name)->str, kSphere) == 0);
    if (eq) {
        result = new QvC484940;
        goto done;
    }
    eq = (*(*name)->str == kSpotLight[0] && strcmp((*name)->str, kSpotLight) == 0);
    if (eq) {
        result = new QvC484530;
        goto done;
    }
    eq = (*(*name)->str == kSwitch[0] && strcmp((*name)->str, kSwitch) == 0);
    if (eq) {
        result = new QvC484370;
        goto done;
    }
    eq = (*(*name)->str == kTexture2[0] && strcmp((*name)->str, kTexture2) == 0);
    if (eq) {
        result = new QvC483f90;
        goto done;
    }
    eq = (*(*name)->str == kTexture2Transform[0] && strcmp((*name)->str, kTexture2Transform) == 0);
    if (eq) {
        result = new QvC483cc0;
        goto done;
    }
    eq = (*(*name)->str == kTextureCoordinate2[0] && strcmp((*name)->str, kTextureCoordinate2) == 0);
    if (eq) {
        result = new QvC483af0;
        goto done;
    }
    eq = (*(*name)->str == kTransform[0] && strcmp((*name)->str, kTransform) == 0);
    if (eq) {
        result = new QvC483790;
        goto done;
    }
    eq = (*(*name)->str == kTransformSeparator[0] && strcmp((*name)->str, kTransformSeparator) == 0);
    if (eq) {
        result = new QvC483630;
        goto done;
    }
    eq = (*(*name)->str == kTranslation[0] && strcmp((*name)->str, kTranslation) == 0);
    if (eq) {
        result = new QvC483460;
        goto done;
    }
    eq = (*(*name)->str == kWWWAnchor[0] && strcmp((*name)->str, kWWWAnchor) == 0);
    if (eq) {
        result = new QvC483150;
        goto done;
    }
    eq = (*(*name)->str == kWWWInline[0] && strcmp((*name)->str, kWWWInline) == 0);
    if (eq) {
        result = new QvC482ed0;
        goto done;
    }
done:
    return result;
}

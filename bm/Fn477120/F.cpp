// SQZ.EXE 0x477120 (stdcall, 3 args): ask a scene object for the axes and position of one node and write its 4x4 transform to out.
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

struct Raw3 { float x, y, z; };
struct SceneObj {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void __stdcall GetPosition(int node, Raw3 *out);            // slot 24 (+0x60)
    virtual void s25(); virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29(); virtual void s30();
    virtual void __stdcall GetAxes(int node, Raw3 *a, Raw3 *b);         // slot 31 (+0x7c)
};
struct NodeMat { float v[16]; };

void __stdcall GetNodeMatrix(SceneObj *obj, int node, NodeMat *out)
{
    Raw3 ra, rb, rc;
    obj->GetAxes(node, &ra, &rb);
    obj->GetPosition(node, &rc);
    Vector3f a(ra.x, ra.y, ra.z);
    Vector3f b(rb.x, rb.y, rb.z);
    Vector3f r = b.Cross(a);
    Vector3f c(rc.x, rc.y, rc.z);
    out->v[0] = r.x;
    out->v[1] = r.y;
    out->v[2] = r.z;
    out->v[3] = 0;
    out->v[4] = b.x;
    out->v[5] = b.y;
    out->v[6] = b.z;
    out->v[7] = 0;
    out->v[8] = a.x;
    out->v[9] = a.y;
    out->v[10] = a.z;
    out->v[11] = 0;
    out->v[12] = c.x;
    out->v[13] = c.y;
    out->v[14] = c.z;
    out->v[15] = 1.0f;
}

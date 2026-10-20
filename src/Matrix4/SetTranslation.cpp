// Matrix4::SetTranslation (0x47e330): writes the translation column m[12..14], returns *this.
#include "Matrix4.h"
Matrix4 &Matrix4::SetTranslation(const float *t)
{
    m[12] = t[0];
    m[13] = t[1];
    m[14] = t[2];
    return *this;
}

// Body getters (0x40bb70-0x40bba0, 0x41fcb0).
#include "Body/Body.h"

float *Body::GetMatrix()      { return matrix; }
Float3 *Body::GetFacing()     { return &facing; }
float Body::GetFacingValue()  { return facingValue; }
float Body::GetExtraValue()   { return extraValue; }
Contact *Body::GetContact()   { return &contact; }
int Body::IsOnSurface()       { return 0; }

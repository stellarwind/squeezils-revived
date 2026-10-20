// SQZ.EXE 0x457bc0: blend ground-orientation changes and advance animation.
#include "SqueezilBody/BodyUpdate.h"

void UpdatingSqueezilBody::Update()
{
    BodyUpdateMatrix shown;
    if (IsOnGround()) {
        BodyUpdateMatrix *current;
        if (!noBlend && ((current = GetMatrix())->m[4] != lastMatrix.m[4] ||
                        current->m[5] != lastMatrix.m[5] ||
                        current->m[6] != lastMatrix.m[6])) {
            blendTime = 0.0f;
            blendFrom.CopyInline(lastMatrix);
            blending = 1;
        } else
            unknown20c = 0;
    }
    if (blending) {
        blendTime += TheClock()->SimRate() * kBodyBlendTimeScale;
        if (blendTime >= gBlendSeconds) {
            blending = 0;
            shownMatrix.CopyInline(blendTo);
        } else {
            blendTo.CopyInline(*GetMatrix());
            shown = BlendedMatrix();
            shownMatrix = shown;
        }
    } else {
        shownMatrix = *GetMatrix();
        if (noBlend == 1)
            noBlend = 0;
    }
    lastMatrix = *GetMatrix();
    if (mode == 0x31)
        return;
    float seconds = TheClock()->SimRate() * kBodyClipTimeScale;
    if (IsOnGround())
        AnimateOnGround(seconds);
    else
        AnimateInAir(seconds);
}

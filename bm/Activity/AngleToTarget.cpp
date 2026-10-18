// TurnTargetActivity::AngleToTarget, SQZ.EXE 0x4016e0.
#include "Activity/Activity.h"
#include <math.h>

float TurnTargetActivity::AngleToTarget() const
{
    return (float)acos((double)cosAngle) * 57.29578f;
}

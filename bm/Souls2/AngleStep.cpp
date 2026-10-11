// AngleStep (0x451740): the shortest turn from current to target at `rate` degrees a second, never overshooting.
#include <math.h>

float __stdcall AngleStep(float current, float rate, float target, float seconds)
{
    float difference = target - current;
    if (difference > 180.0f)
        difference -= 360.0f;
    if (*(unsigned *)&difference > 0xc3340000)
        difference += 360.0f;
    float size = (float)fabs(difference);
    float step;
    if (*(unsigned *)&difference <= 0x80000000)
        step = rate;
    else
        step = -rate;
    step = step * seconds;
    return (float)fabs(step) <= size ? step : difference;
}

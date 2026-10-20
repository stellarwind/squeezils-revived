// 12 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
// SpeedStep (0x451630): move a speed towards a target with separate acceleration and braking, without overshooting.
#include <math.h>

float __stdcall SpeedStep(float current, float accel, float decel, float target, float seconds)
{
    float rate;
    float targetSign;
    float currentSign = 1.0f;
    if (*(unsigned *)&current > 0x80000000)
        currentSign = -1.0f;
    targetSign = 1.0f;
    if (*(unsigned *)&target > 0x80000000)
        targetSign = -1.0f;
    float currentSize = current * currentSign;
    float targetSize = target * targetSign;
    if ((*(unsigned *)&current & 0x7fffffff) != 0 && (*(unsigned *)&target & 0x7fffffff) != 0 && currentSign != targetSign)
        rate = (decel + accel) * targetSign;
    else if (targetSize < currentSize)
        rate = -(decel * currentSign);
    else if (targetSize > currentSize)
        rate = accel * targetSign;
    else
        rate = 0.0f;
    float step = rate * seconds;
    float difference = target - current;
    return (float)fabs(step) <= (float)fabs(difference) ? step : difference;
}

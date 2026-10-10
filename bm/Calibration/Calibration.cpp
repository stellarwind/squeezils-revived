// CalibrationCheck (0x4479c0): return the field at +0x2c.
#include "Calibration/Calibration.h"

int CalibrationCheck::Check()
{
    return field2c;
}

void CalibrationCheck::Start()
{
    field2c = 0;
}

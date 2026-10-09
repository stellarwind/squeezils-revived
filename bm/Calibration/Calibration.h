// Calibration: the input calibration object. Only the trivial check is ported here.
#pragma once
#include "compat41.h"

struct CalibrationCheck {
    char pad[0x2c];
    int  field2c;   // 0x2c

    int Check();    // 0x4479c0 (thiscall)
};

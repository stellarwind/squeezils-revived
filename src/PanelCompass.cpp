// PanelCompass: the panel's compass. Each frame the player's heading is read from the player's matrix; when it has turned (or looked up
// or down) the region screen's compass is turned to match, with a small roll animation while the view moves up and down.
// From SQZ.EXE 0x42e2b0. The state kept between frames is in three globals (heading, height, roll).
#include <string.h>
#include <math.h>
#include "Panel.h"
#include "Original.h"

static unsigned BitsOf(float f)
{
    unsigned u;
    memcpy(&u, &f, sizeof u);
    return u;
}

// the compass state shared by every frame (initial values: 100.0, -99.0 meaning none, 0.0)
static float *const kCompassAngle  = (float *)0x4bbe94;
static float *const kCompassHeight = (float *)0x4bbe98;
static float *const kCompassRoll   = (float *)0x4bbe9c;

void Panel::UpdateCompass(int force)
{
    float *m = PlayerMatrix();                       // the player's matrix: row 0, row 2 and the position row are used
    if (!m)
        return;
    vec5c8.x = m[0];  vec5c8.y = m[1];  vec5c8.z = m[2];
    vec5e0.x = m[8];  vec5e0.y = m[9];  vec5e0.z = m[10];
    float angle = (float)atan2((double)vec5e0.z, (double)vec5c8.z);
    vec5d4.x = m[12]; vec5d4.y = m[13]; vec5d4.z = m[14];
    if (angle < 0.0f)
        angle = field610 + angle;
    float dAngle = angle - *kCompassAngle;
    float dHeight = BitsOf(*kCompassHeight) == 0xc2c60000 ? 0.0f : vec5d4.y - *kCompassHeight;
    if (fabsf(dAngle) < field5bc && !force && !(fabsf(dHeight) > verticalChange) && (BitsOf(*kCompassRoll) & 0x7fffffff) == 0)
        return;

    float roll = *kCompassRoll;
    if (fabsf(dHeight) > verticalChange) {
        if (BitsOf(dHeight) > 0x80000000u)
            roll = verticalThreshold + roll;
        else
            roll = roll - verticalThreshold;
    } else {
        unsigned b = BitsOf(roll);
        if (b > 0x80000000u)
            roll = roll - verticalThreshold;
        else if ((int)b > 0)
            roll = verticalThreshold + roll;
    }
    if ((int)BitsOf(roll) >= 0x3f800000 || BitsOf(roll) >= 0xbf800000u)
        roll = 0.0f;
    *kCompassRoll = roll;
    RegionScreenSetCompass(regionScreen, angle, roll);
    memcpy(&vec5ec, &vec5c8, 9 * sizeof(float));     // remember this frame's three vectors
    *kCompassAngle = angle;
    *kCompassHeight = vec5d4.y;
}


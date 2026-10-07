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
    row0.x = m[0];  row0.y = m[1];  row0.z = m[2];
    row2.x = m[8];  row2.y = m[9];  row2.z = m[10];
    float angle = (float)atan2((double)row2.z, (double)row0.z);
    position.x = m[12]; position.y = m[13]; position.z = m[14];
    if (angle < 0.0f)
        angle = fullTurn + angle;
    float dAngle = angle - *kCompassAngle;
    float dHeight = BitsOf(*kCompassHeight) == 0xc2c60000 ? 0.0f : position.y - *kCompassHeight;
    if (fabsf(dAngle) < turnThreshold && !force && !(fabsf(dHeight) > verticalChange) && (BitsOf(*kCompassRoll) & 0x7fffffff) == 0)
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
    compass.SetCompass(angle, roll);
    memcpy(&prevRow0, &row0, 9 * sizeof(float));     // remember this frame's three vectors
    *kCompassAngle = angle;
    *kCompassHeight = position.y;
}


// SqueezilSoul: ground movement states (0x451630-0x4569c0).
#include <math.h>
#include "Soul.h"

// @004565e0 Touched down: going forward it keeps the forward part of its speed (at most MaxLandingSpeed, unless it tumbled in,
// and MaxRunSpeed when running) and picks walk, run or stumble from it; going sideways it slips; else it stops.
void SqueezilSoul::Land(GroundControl *c, int next)
{
    float speed = Body()->Speed();
    Vector3f direction(Body()->Direction());
    Vector3f right(Body()->GetMatrix(), 0);
    Vector3f forward(Body()->GetMatrix(), 2);
    if (speed > 0.01f) {
        float along = forward.y * direction.y + forward.z * direction.z + forward.x * direction.x;
        float sideways = right.y * direction.y + right.z * direction.z + right.x * direction.x;
        int to;
        if (along > 0.3) {
            speed = along * speed;
            to = next;
            if (next != 5)
                to = GroundStateForSpeed(speed);
        } else if (sideways > 0.3) {
            speed = along * speed;
            to = 6;
        } else {
            to = 0;
            speed = 0.0f;
        }
        SetState(to, 0);
        if (physics->maxLandingSpeed < speed && limitLanding)
            speed = physics->maxLandingSpeed;
        limitLanding = -1;
        if (to == 4 && physics->maxRunSpeed < speed)
            speed = physics->maxRunSpeed;
        Body()->SetVelocity(&forward, speed);
    } else {
        SetState(0, 1);
        Body()->SetVelocity(&direction, 0.0f);
        speed = 0.0f;
    }
    Body()->SetClipSpeed(speed);
}

// @00456800 Braking: it moves with no forward wish and without slipping; when the brake clip is near standstill the skid sound
// plays (skid piece 2), then it stops. A backward wish keeps it braking.
void SqueezilSoul::Skid(GroundControl *c, int next)
{
    int reversing = c->forward < 0.0f;
    c->forward = 0.0f;
    canSlip = 0;
    GroundMove(c, 24);
    canSlip = 1;
    if (GetState() == 6)
        return;
    switch (Body()->GetMode()) {
    case 0:
        if (!reversing) {
            if (next != 24 && CanEnter(next))
                SetState(next, 0);
            else
                SetState(0, 0);
        }
        break;
    case 47:
        if (!(fabs(Body()->Speed()) > 0.01)) {
            PlayEffect("skidEffect", *Body()->GetMatrix()->Position());
            Body()->SetMode(48, 0, 0);
        }
        break;
    case 48:
        if (Body()->ClipEnded())
            Body()->SetMode(0, 0, 0);
        break;
    }
}

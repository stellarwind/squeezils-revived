// SQZ.EXE 0x446fb0: transfer an attacker's motion to the body it strikes.
#define V3_DTOR_OUTOFLINE
#include "CollisionBody.h"
#include <math.h>

void AttackImpulse(CollisionBody *attacker, CollisionBody *defender)
{
    Vector3f result;
    Vector3f axis;
    Vector3f *d = defender->Direction();
    Vector3f *a = attacker->Direction();
    float dot = a->x * d->x + a->z * d->z + a->y * d->y;
    if (fabs(defender->Speed()) <= (double)0.01f || fabs(dot + 1.0f) <= (double)0.0001f) {
        result = *attacker->Direction();
    } else if (*(unsigned *)&dot <= 0xbd4ccccdU && *(int *)&dot <= 0x3d4ccccd) {
        result = *attacker->Direction() + *defender->Direction();
        result.Normalize();
    } else if (fabs(1.0f - dot) <= (double)0.0001f) {
        result = *defender->Direction();
    } else {
        if (*(unsigned *)&dot > 0x80000000U) {
            axis = *attacker->Direction();
        } else if (*(int *)&dot > 0) {
            Vector3f *position = (Vector3f *)attacker->GetMatrix();
            d = defender->Direction();
            dot = position->x * d->x + d->y * position->y + d->z * position->z;
            axis = *(Vector3f *)attacker->GetMatrix();
            if (*(unsigned *)&dot > 0x80000000U)
                axis = axis * -1.0f;
        }
        Vector3f direction = *defender->Direction();
        result = direction - axis * ((axis.z * direction.z + axis.y * direction.y +
                                      axis.x * direction.x) * 2.0f);
        result.Normalize();
    }
    float speed = attacker->Speed();
    defender->SetMotion(&result, (defender->Speed() + speed) * defender->bounciness);
    attacker->SetMotion(attacker->Direction(), 0.0f);
}
